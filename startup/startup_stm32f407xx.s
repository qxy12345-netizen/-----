/**
 * @file    startup_stm32f407xx.s
 * @brief   启动文件（GNU as 语法，给 arm-none-eabi-gcc / VSCode 用）
 *
 * Keil(ARMCC) 用的是另一份：MDK-ARM/startup_stm32f407xx_keil.s
 *
 * 做三件事：放中断向量表 -> 把 .data 从 Flash 搬到 RAM、清零 .bss -> 跳 main
 * 本工程不开任何中断，所以外部中断全部指向 Default_Handler。
 */
    .syntax unified
    .cpu    cortex-m4
    .fpu    fpv4-sp-d16
    .thumb

    .global  g_pfnVectors
    .global  Reset_Handler
    .global  Default_Handler

/* ---------------- 中断向量表 ---------------- */
    .section .isr_vector, "a", %progbits
    .type    g_pfnVectors, %object
g_pfnVectors:
    .word    _estack                /* 初始堆栈指针 */
    .word    Reset_Handler          /* 复位 */
    .word    Default_Handler        /* NMI */
    .word    Default_Handler        /* HardFault */
    .word    Default_Handler        /* MemManage */
    .word    Default_Handler        /* BusFault */
    .word    Default_Handler        /* UsageFault */
    .word    0
    .word    0
    .word    0
    .word    0
    .word    Default_Handler        /* SVCall */
    .word    Default_Handler        /* DebugMon */
    .word    0
    .word    Default_Handler        /* PendSV */
    .word    Default_Handler        /* SysTick */
    .rept    82                     /* EXTI0..IRQ81，F407 共 82 个外部中断 */
    .word    Default_Handler
    .endr
    .size    g_pfnVectors, . - g_pfnVectors

/* ---------------- 复位入口 ---------------- */
    .section .text.Reset_Handler, "ax", %progbits
    .weak    Reset_Handler
    .type    Reset_Handler, %function
    .thumb_func
Reset_Handler:
    ldr      sp, =_estack

    /* 1) .data 段：从 Flash 拷贝到 RAM */
    ldr      r0, =_sdata
    ldr      r1, =_edata
    ldr      r2, =_sidata
copy_data:
    cmp      r0, r1
    bcs      copy_data_done
    ldr      r3, [r2], #4
    str      r3, [r0], #4
    b        copy_data
copy_data_done:

    /* 2) .bss 段：清零 */
    ldr      r0, =_sbss
    ldr      r1, =_ebss
    movs     r3, #0
zero_bss:
    cmp      r0, r1
    bcs      zero_bss_done
    str      r3, [r0], #4
    b        zero_bss
zero_bss_done:

    /* 3) 进 C 世界 */
    bl       main
    b        .
    .size    Reset_Handler, . - Reset_Handler

/* ---------------- 兜底中断 ---------------- */
    .section .text.Default_Handler, "ax", %progbits
    .type    Default_Handler, %function
    .thumb_func
Default_Handler:
    b        .
    .size    Default_Handler, . - Default_Handler
