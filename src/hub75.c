/**
 * @file    hub75.c
 * @brief   HUB75 位操作驱动（STM32F407VET6 @168MHz）
 *
 * ------------------------------------------------------------------
 *  HUB75 是怎么扫出一帧的
 * ------------------------------------------------------------------
 *  64x64 的屏用 1/32 扫描：行选 A~E 选中一对行，R1/G1/B1 送到上半屏那一行，
 *  R2/G2/B2 同时送到下半屏（地址+32）那一行。32 个地址刷完 64 行。
 *
 *  一个地址的时序（本文件 hub75_scan_frame 干的事）：
 *
 *    OE  ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔╲____________╱▔▔▔▔▔▔      (低=点亮)
 *    CLK  ↕↕↕↕↕↕↕↕↕↕↕↕↕↕↕↕↕↕↕↕↕  (64 个上升沿，移入 64 列)
 *    LAT  ______________________╱▔╲___________
 *    行选 <----------- 本地址保持不变 ----------->
 *
 *  颜色用 BCM（二进制编码调制）实现 16 级亮度：每个地址分 4 个位平面，
 *  第 b 个平面点亮 2^b 个基准时间，四段加起来 = 1..15 级灰度。
 *
 * ------------------------------------------------------------------
 *  位平面怎么存的
 * ------------------------------------------------------------------
 *  s_plane[地址 0~31][位平面 0~3][列 0~63]，每个字节就是屏幕某一列
 *  在这一位平面上的 6 个数据位：
 *
 *      bit0 R1   bit1 G1   bit2 B1   bit3 R2   bit4 G2   bit5 B2
 *
 *  一次移位把 64 个这样的字节依次送进屏，就完成了一个位平面。
 *  画点的时候直接改这 8KB，不再另外存帧缓冲 —— 20KB SRAM 经不起两份 8KB。
 */
#include "hub75.h"
#include "board_pins.h"
#include "board.h"
#include "stm32f407_regs.h"

/* 本工程用到的 9 个脚（PA0~PA8），BSRR 只碰这 9 位 */
#define HUB_DATA_MASK   ((1UL << HUB_PIN_R1) | (1UL << HUB_PIN_G1) | \
                         (1UL << HUB_PIN_B1) | (1UL << HUB_PIN_R2) | \
                         (1UL << HUB_PIN_G2) | (1UL << HUB_PIN_B2))
#define HUB_ALL_MASK    (HUB_DATA_MASK | (1UL << HUB_PIN_CLK) | \
                         (1UL << HUB_PIN_LAT) | (1UL << HUB_PIN_OE))

/* 6 条数据线对应的引脚号。PB2 被板载 LED 占着，所以数据脚不连号，
   必须用这张表映射，"位序 -> 引脚"别搞反。 */
static const uint8_t s_data_pin[6] = {
    HUB_PIN_R1, HUB_PIN_G1, HUB_PIN_B1,
    HUB_PIN_R2, HUB_PIN_G2, HUB_PIN_B2
};

static uint32_t s_lut_lo[64];   /* 数据 + CLK=0 + OE=1(关) 的 BSRR 字 */
static uint32_t s_lut_hi[64];   /* 数据 + CLK=1 + OE=1(关) 的 BSRR 字 */
static uint8_t  s_plane[32][4][HUB75_WIDTH];

/* ------------------------------------------------------------------ */
/**
 * @brief 把一个引脚配成高速推挽输出（F4 的写法：每脚 2 位）
 * @note  F4 和 F1 完全不同：F1 是 CRL/CRH 每脚 4 位（MODE+CNF），
 *        F4 是 MODER/OSPEEDR/OTYPER/PUPDR 每脚 2 位。
 */
static void gpio_config_output(GPIO_TypeDef *port, uint32_t pin)
{
    uint32_t sh = pin * 2UL;
    uint32_t v;

    v = port->MODER;
    v &= ~(3UL << sh);
    v |=  (1UL << sh);              /* 01 = 通用输出 */
    port->MODER = v;

    v = port->OSPEEDR;
    v &= ~(3UL << sh);
    v |=  (3UL << sh);              /* 11 = 100MHz，HUB75 要陡边沿 */
    port->OSPEEDR = v;

    v = port->PUPDR;
    v &= ~(3UL << sh);              /* 00 = 无上下拉 */
    port->PUPDR = v;

    port->OTYPER &= ~(1UL << pin);  /* 0 = 推挽 */
}

/* ------------------------------------------------------------------ */
void hub75_init(void)
{
    /* 1) 开 GPIOE（数据/时序）和 GPIOB（行选）时钟
          F4 的 GPIO 挂在 AHB1，不是 F1 的 APB2 */
    RCC->AHB1ENR |= (1UL << 4) | (1UL << 1);    /* GPIOEEN | GPIOBEN */
    (void)RCC->AHB1ENR;

    /* 2) 配置成输出之前先把输出寄存器摆到安全状态：
          OE=1 不点亮，数据/CLK/LAT 全 0 */
    HUB_GPIO->BSRR = (1UL << HUB_PIN_OE);
    HUB_ADDR_GPIO->BSRR = (0x1FUL << (16 + HUB_ADDR_SHIFT));

    /* 3) 6 根数据 + CLK + LAT + OE（PE7~PE15，位号连续） */
    for (uint32_t i = 0; i < 6; i++) {
        gpio_config_output(HUB_GPIO, s_data_pin[i]);
    }
    gpio_config_output(HUB_GPIO, HUB_PIN_CLK);
    gpio_config_output(HUB_GPIO, HUB_PIN_LAT);
    gpio_config_output(HUB_GPIO, HUB_PIN_OE);

    /* 4) 行选 A~E：PB10~PB14（位号连续，写地址一条指令） */
    for (uint32_t i = 0; i < 5; i++) {
        gpio_config_output(HUB_ADDR_GPIO, HUB_ADDR_SHIFT + i);
    }

    /* 5) 预计算 BSRR 查找表：一次 32 位写同时更新 9 个脚。
          这是刷新率的关键，否则每列要写 9 次。 */
    for (uint32_t v = 0; v < 64; v++) {
        uint32_t hi = (1UL << HUB_PIN_OE);      /* OE 拉高 = 关显示 */
        for (uint32_t i = 0; i < 6; i++) {
            if (v & (1UL << i)) {
                hi |= (1UL << s_data_pin[i]);   /* 位序 -> 实际引脚 */
            }
        }
        s_lut_lo[v] = hi | ((HUB_ALL_MASK & ~hi) << 16);
        {
            uint32_t hi2 = hi | (1UL << HUB_PIN_CLK);
            s_lut_hi[v] = hi2 | ((HUB_ALL_MASK & ~hi2) << 16);
        }
    }

    hub75_clear();
}

/* ------------------------------------------------------------------ */
void hub75_clear(void)
{
    uint8_t *p = &s_plane[0][0][0];
    for (uint32_t i = 0; i < sizeof(s_plane); i++) {
        p[i] = 0;
    }
}

void hub75_fill(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint32_t k = 0; k < 4; k++) {
        uint8_t v = 0;
        if (r & (1UL << k)) { v |= 0x09; }      /* R1(bit0) + R2(bit3) */
        if (g & (1UL << k)) { v |= 0x12; }      /* G1(bit1) + G2(bit4) */
        if (b & (1UL << k)) { v |= 0x24; }      /* B1(bit2) + B2(bit5) */

        /* 注意：不能写成 &s_plane[0][k][0] 然后连续写 2048 字节 ——
           s_plane 的三维数组在内存里是 [地址][位平面][列] 顺序排列的，
           连续写会把位平面维度踩乱。必须按 [地址][列] 两层循环写。 */
        for (uint32_t a = 0; a < 32; a++) {
            for (uint32_t x = 0; x < HUB75_WIDTH; x++) {
                s_plane[a][k][x] = v;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* 调试用：全屏常亮，不做扫描，只把数据线拉高、OE 拉低。
   拿万用表量屏的 OE / R1 引脚时用这个模式，读数才是稳定的直流值。 */
void hub75_static_white(void)
{
    /* 6 条数据线全高 -> 所有颜色满亮 */
    HUB_GPIO->BSRR = HUB_DATA_MASK;
    /* CLK 低、LAT 低、OE 低（点亮） */
    HUB_GPIO->BSRR = ((1UL << HUB_PIN_CLK) | (1UL << HUB_PIN_LAT) |
                      (1UL << HUB_PIN_OE)) << 16;
    /* 行选拉到 0，至少第一对行是选中的 */
    HUB_ADDR_GPIO->BSRR = (0x1FUL << (16 + HUB_ADDR_SHIFT));
}

void hub75_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    /* HUB75 的移位寄存器是"先入后出"：第一个移进去的位要走完 64 级才停下，
       最后落在最右边。所以第 63 列必须先送、第 0 列最后送。
       这里把像素直接写到"移位次序"对应的槽位，扫描时按下标 0..63 顺序发出去就对了。 */
    int col = HUB75_WIDTH - 1 - x;
#if HUB75_FLIP_X
    col = HUB75_WIDTH - 1 - col;
#endif

    /* 上半屏（y<32）走 R1/G1/B1 = bit0~2；下半屏（y>=32）走 R2/G2/B2 = bit3~5 */
    uint8_t  base  = (y < 32) ? 0u : 3u;
    uint8_t  bitr  = (uint8_t)(1u << base);
    uint8_t  mask  = (uint8_t)(bitr | (bitr << 1) | (bitr << 2));
    uint8_t  row   = (uint8_t)(y & 31);

    if ((unsigned)x >= HUB75_WIDTH || (unsigned)y >= HUB75_HEIGHT) {
        return;
    }

    for (uint32_t k = 0; k < 4; k++) {
        uint8_t v = 0;
        if (r & (1UL << k)) { v |= bitr; }
        if (g & (1UL << k)) { v |= (uint8_t)(bitr << 1); }
        if (b & (1UL << k)) { v |= (uint8_t)(bitr << 2); }

        uint8_t *q = &s_plane[row][k][col];
        *q = (uint8_t)((*q & ~mask) | v);
    }
}

/* ------------------------------------------------------------------ */
/* 移出一行 64 列：每列两次 BSRR 写（先数据带 CLK 低，再 CLK 高）        */
static inline void shift_line(const uint8_t *px)
{
    for (int i = 0; i < HUB75_WIDTH; i++) {
        uint32_t v = px[i];

        HUB_GPIO->BSRR = s_lut_lo[v];
        for (int k = 0; k < HUB75_CLK_NOP; k++) { board_nop(); }
        HUB_GPIO->BSRR = s_lut_hi[v];           /* ↑ 上升沿，屏里移入一位 */
        for (int k = 0; k < HUB75_CLK_NOP; k++) { board_nop(); }
    }
    HUB_GPIO->BSRR = (1UL << (HUB_PIN_CLK + 16));   /* CLK 回低电平 */
}

static inline void latch(void)
{
    /* 先用 8 个空指令让 CLK 的振铃衰减。CLK(13脚) 和 LAT(14脚) 在排线上
       紧挨着，用散杜邦线时 4MHz 的 CLK 很容易耦合到 LAT 上，
       造成"这次锁存进的是上一个位平面的数据" -> 每列亮度不一样（竖条纹）。 */
    for (int k = 0; k < 8; k++) { board_nop(); }

    /* LAT 高脉冲宽度手册要求 > 20ns，这里 4 个空指令 + 两次写约 100ns */
    HUB_GPIO->BSRR = (1UL << HUB_PIN_LAT);
    board_nop(); board_nop(); board_nop(); board_nop();
    HUB_GPIO->BSRR = (1UL << (HUB_PIN_LAT + 16));
}

static inline void set_addr(uint32_t a)
{
    HUB_ADDR_GPIO->BSRR = ((a & 0x1FUL) << HUB_ADDR_SHIFT) |
                          (((~a) & 0x1FUL) << (16 + HUB_ADDR_SHIFT));
}

/* ------------------------------------------------------------------ */
void hub75_scan_frame(void)
{
    for (uint32_t a = 0; a < 32; a++) {
        set_addr(a);

        for (uint32_t b = 0; b < 4; b++) {
            HUB_GPIO->BSRR = (1UL << HUB_PIN_OE);        /* 移位期间关显示 */
            shift_line(s_plane[a][b]);
            latch();
            HUB_GPIO->BSRR = (1UL << (HUB_PIN_OE + 16)); /* 拉低 OE -> 点亮 */
            board_delay_us((1UL << b) * HUB75_BCM_BASE_US);
            HUB_GPIO->BSRR = (1UL << HUB_PIN_OE);        /* 熄灭，换下一位平面 */
        }
    }
}

void hub75_hold_frames(uint32_t frames)
{
    while (frames--) {
        hub75_scan_frame();
    }
}
