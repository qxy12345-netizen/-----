;******************************************************************************
;* @file    startup_stm32f407xx_keil.s
;* @brief   Keil MDK (ARMCC/armasm) 启动文件 —— STM32F407VET6
;*
;* 配套：MDK-ARM/matrix3.sct （分散加载文件）
;* 本工程不用任何中断，82 个中断向量全部指向 Default_Handler。
;* GCC 用的是另一份：startup/startup_stm32f407xx.s
;******************************************************************************

Stack_Size      EQU     0x00002000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

Heap_Size       EQU     0x00000200

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

; ------------------------- 中断向量表 -------------------------
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp            ; 栈顶
                DCD     Reset_Handler           ; 复位
                DCD     NMI_Handler
                DCD     HardFault_Handler
                DCD     MemManage_Handler
                DCD     BusFault_Handler
                DCD     UsageFault_Handler
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     SVC_Handler
                DCD     DebugMon_Handler
                DCD     0
                DCD     PendSV_Handler
                DCD     SysTick_Handler
                DCD     Default_Handler     ; IRQ0
                DCD     Default_Handler     ; IRQ1
                DCD     Default_Handler     ; IRQ2
                DCD     Default_Handler     ; IRQ3
                DCD     Default_Handler     ; IRQ4
                DCD     Default_Handler     ; IRQ5
                DCD     Default_Handler     ; IRQ6
                DCD     Default_Handler     ; IRQ7
                DCD     Default_Handler     ; IRQ8
                DCD     Default_Handler     ; IRQ9
                DCD     Default_Handler     ; IRQ10
                DCD     Default_Handler     ; IRQ11
                DCD     Default_Handler     ; IRQ12
                DCD     Default_Handler     ; IRQ13
                DCD     Default_Handler     ; IRQ14
                DCD     Default_Handler     ; IRQ15
                DCD     Default_Handler     ; IRQ16
                DCD     Default_Handler     ; IRQ17
                DCD     Default_Handler     ; IRQ18
                DCD     Default_Handler     ; IRQ19
                DCD     Default_Handler     ; IRQ20
                DCD     Default_Handler     ; IRQ21
                DCD     Default_Handler     ; IRQ22
                DCD     Default_Handler     ; IRQ23
                DCD     Default_Handler     ; IRQ24
                DCD     Default_Handler     ; IRQ25
                DCD     Default_Handler     ; IRQ26
                DCD     Default_Handler     ; IRQ27
                DCD     Default_Handler     ; IRQ28
                DCD     Default_Handler     ; IRQ29
                DCD     Default_Handler     ; IRQ30
                DCD     Default_Handler     ; IRQ31
                DCD     Default_Handler     ; IRQ32
                DCD     Default_Handler     ; IRQ33
                DCD     Default_Handler     ; IRQ34
                DCD     Default_Handler     ; IRQ35
                DCD     Default_Handler     ; IRQ36
                DCD     Default_Handler     ; IRQ37
                DCD     Default_Handler     ; IRQ38
                DCD     Default_Handler     ; IRQ39
                DCD     Default_Handler     ; IRQ40
                DCD     Default_Handler     ; IRQ41
                DCD     Default_Handler     ; IRQ42
                DCD     Default_Handler     ; IRQ43
                DCD     Default_Handler     ; IRQ44
                DCD     Default_Handler     ; IRQ45
                DCD     Default_Handler     ; IRQ46
                DCD     Default_Handler     ; IRQ47
                DCD     Default_Handler     ; IRQ48
                DCD     Default_Handler     ; IRQ49
                DCD     Default_Handler     ; IRQ50
                DCD     Default_Handler     ; IRQ51
                DCD     Default_Handler     ; IRQ52
                DCD     Default_Handler     ; IRQ53
                DCD     Default_Handler     ; IRQ54
                DCD     Default_Handler     ; IRQ55
                DCD     Default_Handler     ; IRQ56
                DCD     Default_Handler     ; IRQ57
                DCD     Default_Handler     ; IRQ58
                DCD     Default_Handler     ; IRQ59
                DCD     Default_Handler     ; IRQ60
                DCD     Default_Handler     ; IRQ61
                DCD     Default_Handler     ; IRQ62
                DCD     Default_Handler     ; IRQ63
                DCD     Default_Handler     ; IRQ64
                DCD     Default_Handler     ; IRQ65
                DCD     Default_Handler     ; IRQ66
                DCD     Default_Handler     ; IRQ67
                DCD     Default_Handler     ; IRQ68
                DCD     Default_Handler     ; IRQ69
                DCD     Default_Handler     ; IRQ70
                DCD     Default_Handler     ; IRQ71
                DCD     Default_Handler     ; IRQ72
                DCD     Default_Handler     ; IRQ73
                DCD     Default_Handler     ; IRQ74
                DCD     Default_Handler     ; IRQ75
                DCD     Default_Handler     ; IRQ76
                DCD     Default_Handler     ; IRQ77
                DCD     Default_Handler     ; IRQ78
                DCD     Default_Handler     ; IRQ79
                DCD     Default_Handler     ; IRQ80
                DCD     Default_Handler     ; IRQ81

__Vectors_End
__Vectors_Size  EQU     __Vectors_End - __Vectors

; ------------------------- 复位入口 -------------------------
                AREA    |.text|, CODE, READONLY

Reset_Handler   PROC
                EXPORT  Reset_Handler               [WEAK]
                IMPORT  __main
                LDR     R0, =__main
                BX      R0
                ENDP

; ------------------------- 默认中断 -------------------------
Default_Handler PROC
                EXPORT  NMI_Handler                 [WEAK]
                EXPORT  HardFault_Handler           [WEAK]
                EXPORT  MemManage_Handler           [WEAK]
                EXPORT  BusFault_Handler            [WEAK]
                EXPORT  UsageFault_Handler          [WEAK]
                EXPORT  SVC_Handler                 [WEAK]
                EXPORT  DebugMon_Handler            [WEAK]
                EXPORT  PendSV_Handler              [WEAK]
                EXPORT  SysTick_Handler             [WEAK]

NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
SVC_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
                B       .
                ENDP

                ALIGN
                END
