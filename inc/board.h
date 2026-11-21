/**
 * @file    board.h
 * @brief   板级底层：FPU、168MHz 主频、微秒级延时（Cortex-M4F）
 */
#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include <stdint.h>
#include "stm32f407_regs.h"

/* 系统主频（HSE 8MHz × PLL 到 168MHz，见 board.c） */
#define BOARD_SYSCLK_HZ     168000000UL

static inline void board_nop(void) { __asm volatile ("nop"); }

/**
 * @brief 初始化：开 FPU、配 PLL 到 168MHz
 * @note  必须在任何 GPIO 操作之前调用
 */
void board_init(void);

/** @brief 实际运行的主频（HSE 起振失败会退回 64MHz，这里会跟着变） */
uint32_t board_sysclk(void);

/** @brief 忙等微秒（SysTick 单次计时） */
void board_delay_us(uint32_t us);

/** @brief 忙等毫秒 */
void board_delay_ms(uint32_t ms);

#endif /* BOARD_H_INCLUDED */
