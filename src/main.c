/**
 * @file    main.c
 * @brief   64x64 HUB75 点阵显示铺满全屏的数字 "3"（上红下蓝）
 *
 * 板卡：STM32F407VET6（立创天猛星 / 天空星），168MHz，128KB SRAM
 * 屏幕：RGB-Matrix-P2-64x64，HUB75 接口，1/32 扫描
 * 接线：见 inc/board_pins.h 与 README.md
 *
 * 流程：
 *   board_init()      配 PLL 跑到 72MHz
 *   hub75_init()      14 个 GPIO 配成输出，预计算 BSRR 查找表
 *   boot_selftest()   红/绿/蓝 + 白边框各闪一遍（第一次接线排错用，可关）
 *   app_draw_digit3() 查字模表 -> 写位平面
 *   死循环扫描        约 110Hz 刷新
 */
#include "board.h"
#include "board_pins.h"
#include "hub75.h"
#include "glyph3.h"
#include "app.h"

/* ================= 可调参数 ================= */

/* 1 = 开机先闪红/绿/蓝三色 + 白色边框；0 = 直接出字 */
#define BOOT_SELFTEST   1

/* 1 = 板载 LED（PC13）每 50 帧翻转一次，用来证明 CPU 活着。
   屏不亮时先看这颗灯：
     LED 在闪  -> 程序在跑，问题在屏的接线/供电
     LED 不亮  -> 程序没跑起来（下载失败 / 卡在启动） */
#define HEARTBEAT_LED   1

/* 接线排查模式：
   0 = 正常显示数字 "3"
   1 = 四色方块测试图：
         左上 32x32 = 白      右上 32x32 = 红
         左下 32x32 = 绿      右下 32x32 = 蓝
   2 = 单通道轮播：红(1.5s) -> 绿(1.5s) -> 蓝(1.5s) -> 白(1.5s) 循环
       用来把"我的哪个通道"对上"屏上什么颜色"，一次问清 6 根数据线的顺序 */
#define WIRING_TEST     2

/* 颜色分界：y < 32 的上半部分红色，下半部分蓝色（每通道 0~15） */
#define SPLIT_Y         32
#define TOP_R 15
#define TOP_G 0
#define TOP_B 0
#define BOT_R 0
#define BOT_G 0
#define BOT_B 15
//111111
/* ================= 心跳灯 ================= */

#if HEARTBEAT_LED
/**
 * @brief 板载 LED：天猛星/天空星是 PB2，**高电平点亮**
 * @note  纯粹是调试用的"我还活着"指示，跟 HUB75 无关
 */
static void heartbeat_init(void)
{
    uint32_t sh = HEARTBEAT_PIN * 2UL;

    RCC->AHB1ENR |= (1UL << 1);                 /* GPIOB 时钟（心跳灯 PB2） */
    (void)RCC->AHB1ENR;

    /* F4 的写法：MODER 每脚 2 位，01 = 通用输出 */
    HEARTBEAT_GPIO->MODER &= ~(3UL << sh);
    HEARTBEAT_GPIO->MODER |=  (1UL << sh);
    HEARTBEAT_GPIO->OSPEEDR &= ~(3UL << sh);
    HEARTBEAT_GPIO->OSPEEDR |=  (1UL << sh);    /* 2MHz 够用 */
    HEARTBEAT_GPIO->BSRR = (1UL << (HEARTBEAT_PIN + 16));   /* 先熄灭（低电平） */
}

static void heartbeat_tick(void)
{
    static uint32_t n = 0;

    if (++n >= 50) {                            /* 约 50 帧 = 0.5 秒 */
        n = 0;
        HEARTBEAT_GPIO->ODR ^= (1UL << HEARTBEAT_PIN);   /* 翻转 PB2 */
    }
}
#else
static void heartbeat_init(void) { }
static void heartbeat_tick(void) { }
#endif

/* ================= 单通道轮播（接线排查 mode 2） ================= */

#if WIRING_TEST == 2
/**
 * @brief 每 1.5 秒换一次画面：只红 -> 只绿 -> 只蓝 -> 三个一起（白）
 *
 * 判读方法：盯着屏，记下这四次你**实际看到的颜色**。
 *   第 1 次（我送红）   屏上显示？____
 *   第 2 次（我送绿）   屏上显示？____
 *   第 3 次（我送蓝）   屏上显示？____
 *   第 4 次（我送白）   屏上显示？____
 * 哪一次是黑的，说明对应的 2 根数据线（R1/R2 或 G1/G2 或 B1/B2）没接通；
 * 哪两次颜色对调了，说明那两组线接反了。
 */
static void channel_cycle(void)
{
    static uint32_t frames = 0;
    static uint32_t state  = 0;

    if (++frames < 150) {           /* 150 帧 ≈ 1.5 秒 */
        return;
    }
    frames = 0;
    state  = (state + 1) & 3;

    switch (state) {
    case 0:  hub75_fill(15,  0,  0); break;   /* 只送红 */
    case 1:  hub75_fill( 0, 15,  0); break;   /* 只送绿 */
    case 2:  hub75_fill( 0,  0, 15); break;   /* 只送蓝 */
    default: hub75_fill(15, 15, 15); break;   /* 三个都送 = 白 */
    }
}
#endif

/* ================= 四色方块测试图 ================= */

#if WIRING_TEST
/**
 * @brief 画 2x2 的四色方块，用来验证扫描、地址线、数据线
 *
 *      白(左上)  |  红(右上)
 *     -----------+-----------
 *      绿(左下)  |  蓝(右下)
 *
 * 判读方法：
 *   四块都在正确位置         -> 接线全对
 *   只有左上白块             -> R2/G2/B2 三根没接（下半屏+右半屏没数据）
 *   白块在别的角             -> 屏装反了，改 HUB75_FLIP_X / FLIP_Y
 *   红绿蓝位置串了           -> 6 根数据线的顺序接错
 *   只有上/下 8 行有内容     -> 行选 D、E 两根没接（A~C 只能选中 8 个地址）
 */
static void draw_quadrants(void)
{
    hub75_clear();

    for (int y = 0; y < HUB75_HEIGHT; y++) {
        for (int x = 0; x < HUB75_WIDTH; x++) {
            uint8_t r = 0, g = 0, b = 0;

            if (y < 32) {
                if (x < 32) { r = 15; g = 15; b = 15; }   /* 左上：白 */
                else        { r = 15;                   }   /* 右上：红 */
            } else {
                if (x < 32) {          g = 15;         }   /* 左下：绿 */
                else        {                   b = 15; }   /* 右下：蓝 */
            }

            hub75_set_pixel(x, y, r, g, b);
        }
    }
}
#endif

/* ================= 开机自检 ================= */

static void draw_border(uint8_t v)
{
    for (int x = 0; x < HUB75_WIDTH; x++) {
        hub75_set_pixel(x, 0, v, v, v);
        hub75_set_pixel(x, HUB75_HEIGHT - 1, v, v, v);
    }
    for (int y = 0; y < HUB75_HEIGHT; y++) {
        hub75_set_pixel(0, y, v, v, v);
        hub75_set_pixel(HUB75_WIDTH - 1, y, v, v, v);
    }
}

/**
 * @brief 三段纯色 + 一个 1 像素白边框
 *
 * 排错对照：
 *   红/绿/蓝都正常      -> 6 根数据线全通
 *   缺某一色            -> 对应数据线（R1/R2、G1/G2、B1/B2）没接好
 *   边框变成虚线/斜纹   -> CLK 或 LAT 接触不良，或 HUB75_CLK_NOP 太小
 *   整屏偏移、缺行      -> 行选 A~E 接线顺序错了
 *   只有一半亮          -> R2/G2/B2（下半屏那组）没接
 */
static void boot_selftest(void)
{
    /* 先来 1 秒全白：最亮、最容易用眼睛确认，
       也方便拿万用表量屏上 OE / CLK 的平均电压（扫描时是稳定的平均值） */
    hub75_fill(15, 15, 15);  hub75_hold_frames(100);

    hub75_fill(15, 0, 0);    hub75_hold_frames(25);   /* 红 */
    hub75_fill(0, 15, 0);    hub75_hold_frames(25);   /* 绿 */
    hub75_fill(0, 0, 15);    hub75_hold_frames(25);   /* 蓝 */

    hub75_clear();
    draw_border(15);
    hub75_hold_frames(40);
}

/* ================= 主函数 ================= */

/* ================= 运行计数器（调试用，不打印） ================= */
/* 主循环每转一圈 +1。用 ST-Link 读这个变量就能知道程序到底有没有在跑，
   完全不依赖板上的 LED 接在哪个脚。 */
volatile uint32_t g_boot_stage;   /* 走到哪一步了：1=时钟 2=GPIO 3=心跳 4=主循环 */
volatile uint32_t g_frame_count;  /* 主循环计数 */

int main(void)
{
    board_init();
    g_boot_stage = 1;

    hub75_init();
    g_boot_stage = 2;

    heartbeat_init();
    g_boot_stage = 3;

#if BOOT_SELFTEST && !WIRING_TEST
    boot_selftest();
#endif

#if WIRING_TEST == 2
    hub75_fill(15, 0, 0);           /* 从"只红"开始 */
#elif WIRING_TEST == 1
    draw_quadrants();
#else
    app_draw_digit3(TOP_R, TOP_G, TOP_B,
                    BOT_R, BOT_G, BOT_B,
                    SPLIT_Y);
#endif

    g_boot_stage = 4;

    for (;;) {
        hub75_scan_frame();     /* 屏幕显示的就是位平面里的内容 */
        heartbeat_tick();       /* 板载 LED 心跳，证明 CPU 还活着 */
        g_frame_count++;
#if WIRING_TEST == 2
        channel_cycle();        /* 每 1.5 秒换一个颜色 */
#endif
    }
}
