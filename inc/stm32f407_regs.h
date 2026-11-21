/**
 * @file    stm32f407_regs.h
 * @brief   STM32F407 最小寄存器定义（不依赖 CMSIS / HAL / 标准库）
 *
 * 只定义本工程用到的外设，寄存器偏移按 RM0090。
 *
 * ⚠️ F4 和 F1 的 GPIO 配置完全不同：
 *      F4：MODER / OTYPER / OSPEEDR / PUPDR，每个引脚占 2 位
 *      F1：CRL / CRH，每个引脚占 4 位（MODE + CNF）
 */
#ifndef STM32F407_REGS_H
#define STM32F407_REGS_H

#include <stdint.h>

#define __IO volatile

/* ---------------- GPIO（F4 结构：每脚 2 位） ---------------- */
typedef struct {
    __IO uint32_t MODER;    /* 0x00 模式  00输入 01输出 10复用 11模拟 */
    __IO uint32_t OTYPER;   /* 0x04 类型  0推挽 1开漏 */
    __IO uint32_t OSPEEDR;  /* 0x08 速度  00 2M / 01 25M / 10 50M / 11 100M */
    __IO uint32_t PUPDR;    /* 0x0C 上下拉 00无 01上拉 10下拉 */
    __IO uint32_t IDR;      /* 0x10 */
    __IO uint32_t ODR;      /* 0x14 */
    __IO uint32_t BSRR;     /* 0x18 [15:0]置1 [31:16]清0 */
    __IO uint32_t LCKR;     /* 0x1C */
    __IO uint32_t AFR[2];   /* 0x20 复用功能 */
} GPIO_TypeDef;

/* ---------------- RCC ---------------- */
typedef struct {
    __IO uint32_t CR;         /* 0x00 */
    __IO uint32_t PLLCFGR;    /* 0x04 */
    __IO uint32_t CFGR;       /* 0x08 */
    __IO uint32_t CIR;        /* 0x0C */
    __IO uint32_t AHB1RSTR;   /* 0x10 */
    __IO uint32_t AHB2RSTR;   /* 0x14 */
    __IO uint32_t AHB3RSTR;   /* 0x18 */
    __IO uint32_t RESERVED0;
    __IO uint32_t APB1RSTR;   /* 0x20 */
    __IO uint32_t APB2RSTR;   /* 0x24 */
    __IO uint32_t RESERVED1[2];
    __IO uint32_t AHB1ENR;    /* 0x30 */
    __IO uint32_t AHB2ENR;    /* 0x34 */
    __IO uint32_t AHB3ENR;    /* 0x38 */
    __IO uint32_t RESERVED2;
    __IO uint32_t APB1ENR;    /* 0x40 */
    __IO uint32_t APB2ENR;    /* 0x44 */
} RCC_TypeDef;

/* ---------------- FLASH / PWR ---------------- */
typedef struct {
    __IO uint32_t ACR;      /* 0x00 */
    __IO uint32_t KEYR;
    __IO uint32_t OPTKEYR;
    __IO uint32_t SR;
    __IO uint32_t CR;
    __IO uint32_t OPTCR;
} FLASH_TypeDef;

typedef struct {
    __IO uint32_t CR;       /* 0x00 */
    __IO uint32_t CSR;      /* 0x04 */
} PWR_TypeDef;

/* ---------------- SysTick（做微秒延时） ---------------- */
typedef struct {
    __IO uint32_t CTRL;     /* 0x00 bit0 使能 bit2 时钟源 bit16 到零标志 */
    __IO uint32_t LOAD;     /* 0x04 24 位重装值 */
    __IO uint32_t VAL;      /* 0x08 当前值 */
    __IO uint32_t CALIB;    /* 0x0C */
} SysTick_TypeDef;

/* ---------------- 地址映射 ---------------- */
#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   (PERIPH_BASE)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define GPIOA_BASE        (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE        (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE        (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE        (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASE        (AHB1PERIPH_BASE + 0x1000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define FLASH_R_BASE      (AHB1PERIPH_BASE + 0x3C00UL)
#define PWR_BASE          (APB1PERIPH_BASE + 0x7000UL)

#define GPIOA   ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE   ((GPIO_TypeDef *) GPIOE_BASE)

#define RCC     ((RCC_TypeDef  *) RCC_BASE)
#define FLASH   ((FLASH_TypeDef*) FLASH_R_BASE)
#define PWR     ((PWR_TypeDef  *) PWR_BASE)

#define SysTick ((SysTick_TypeDef *) 0xE000E010UL)

/* 协处理器访问控制寄存器：开 FPU 用 */
#define SCB_CPACR   (*(__IO uint32_t *) 0xE000ED88UL)

#endif /* STM32F407_REGS_H */
