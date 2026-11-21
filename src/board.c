/**
 * @file    board.c
 * @brief   FPU + 时钟树 8MHz HSE -> PLL -> 168MHz，SysTick 做微秒延时
 *
 * 时钟链：HSE 8MHz / M=8 = 1MHz -> ×N=336 = 336MHz(VCO) -> /P=2 = 168MHz
 *         168MHz 下 APB1 = 42MHz(÷4)、APB2 = 84MHz(÷2)，都在手册上限内。
 */
#include "board.h"
#include "stm32f407_regs.h"

static uint32_t s_sysclk = BOARD_SYSCLK_HZ;

uint32_t board_sysclk(void)
{
    return s_sysclk;
}

/* ------------------------------------------------------------------ */
void board_init(void)
{
    uint32_t timeout;

    /* 0) 打开 FPU（CP10/CP11 全访问）。本工程现在不用浮点，
          但开着以后加浮点代码不用再改。注意：开了之后必须同步开 -mfpu 编译选项。 */
    SCB_CPACR |= (0xFUL << 20);
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");

    /* 1) 打开 PWR 时钟并把内核电压调到 Scale 1（168MHz 必须） */
    RCC->APB1ENR |= (1UL << 28);
    (void)RCC->APB1ENR;
    PWR->CR |= (1UL << 14);

    /* 2) Flash：5 个等待周期 + 预取 + 指令/数据 Cache */
    FLASH->ACR = (5UL << 0) | (1UL << 8) | (1UL << 9) | (1UL << 10);

    /* 3) 启动 HSE（板载 8MHz 晶振） */
    RCC->CR |= (1UL << 16);
    timeout = 0x40000;
    while (!(RCC->CR & (1UL << 17)) && --timeout) { }

    if (timeout) {
        /* HSE 正常：168 = 8MHz / 8 × 336 / 2 */
        RCC->PLLCFGR = (8UL << 0) | (336UL << 6) | (0UL << 16) |
                       (1UL << 22) | (7UL << 24);
        s_sysclk = 168000000UL;
    } else {
        /* HSE 起振失败：退回内部 HSI 16MHz，16/16×336/2 = 168MHz */
        RCC->CR &= ~(1UL << 16);
        RCC->PLLCFGR = (16UL << 0) | (336UL << 6) | (0UL << 16) |
                       (0UL << 22) | (7UL << 24);
        s_sysclk = 168000000UL;
    }

    /* 4) 总线分频：AHB ÷1，APB1 ÷4，APB2 ÷2 */
    RCC->CFGR = (0UL << 4) | (5UL << 10) | (4UL << 13);

    /* 5) 开 PLL 并等锁定 */
    RCC->CR |= (1UL << 24);
    while (!(RCC->CR & (1UL << 25))) { }

    /* 6) 切换系统时钟到 PLL */
    RCC->CFGR |= 2UL;
    while (((RCC->CFGR >> 2) & 3UL) != 2UL) { }
}

/* ------------------------------------------------------------------ */
void board_delay_us(uint32_t us)
{
    uint32_t ticks = us * (s_sysclk / 1000000UL);

    if (ticks == 0) {
        return;
    }
    if (ticks > 0x00FFFFFFUL) {          /* SysTick 只有 24 位 */
        ticks = 0x00FFFFFFUL;
    }

    SysTick->CTRL = 0;
    SysTick->LOAD = ticks - 1UL;
    SysTick->VAL  = 0;
    SysTick->CTRL = 5UL;                 /* 时钟源 HCLK，开计数器，不产生中断 */
    while (!(SysTick->CTRL & (1UL << 16))) { }
    SysTick->CTRL = 0;
}

void board_delay_ms(uint32_t ms)
{
    while (ms--) {
        board_delay_us(1000);
    }
}
