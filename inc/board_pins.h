/**
 * @file    board_pins.h
 * @brief   【接线唯一真源】HUB75 屏幕信号 <-> STM32F407VET6（天猛星 / 天空星）
 *
 * ==========================================================================
 *  选脚思路：14 根全部落在左边排针 H1 的第 13~28 脚这一小段
 * --------------------------------------------------------------------------
 *  按板子原理图，H1（左边 2×20 排针）的引脚是按端口整组排的：
 *
 *      脚号 13~22：PE7 PE8 PE9 PE10 PE11 PE12 PE13 PE14 PE15   （9 根，2×5）
 *      脚号 23~28：PB10 PB11 PB12 PB13 PB14 PB15               （6 根，2×3）
 *
 *  正好够用：
 *      PE7 ~ PE12 = 6 根颜色数据
 *      PE13       = CLK
 *      PE14       = LAT/STB
 *      PE15       = OE
 *      PB10~PB14  = 行选 A B C D E
 *
 *  也就是说 14 根线全在 H1 的 13~28 脚，物理上连成一片（约 4cm），
 *  飞线时一根 16P 排线直接压上去就行，不用在板子两头找脚。
 *
 *  顺带的好处：9 根数据/时序脚在 PE7~PE15 上**位号连续**，
 *  一次 32 位写 BSRR 就能全更新，扫描速度做满。
 *
 * ==========================================================================
 *  避开的引脚
 * --------------------------------------------------------------------------
 *    PB2       板载 LED —— 拿来当心跳灯，看程序在不在跑
 *    PA13/PA14 SWD 下载调试口，占了就没法烧程序
 *    PA11/PA12 USB-FS
 *    PA0       用户按键（板上有下拉）
 *    PA9/PA10  USART1（留给串口调试）
 *    PE2/PE3   板载 8MHz 晶振
 *    PD3       TF 卡检测
 */
#ifndef BOARD_PINS_H
#define BOARD_PINS_H

/* ---------- 数据 + 时序：PE7 ~ PE15（H1 的 13~22 脚） ---------- */
#define HUB_GPIO            GPIOE

#define HUB_PIN_R1          7         /* PE7  (脚13) -> 排线 R1  上半屏红 */
#define HUB_PIN_G1          8         /* PE8  (脚14) -> 排线 G1  上半屏绿 */
#define HUB_PIN_B1          9         /* PE9  (脚15) -> 排线 B1  上半屏蓝 */
#define HUB_PIN_R2          10        /* PE10 (脚16) -> 排线 R2  下半屏红 */
#define HUB_PIN_G2          11        /* PE11 (脚17) -> 排线 G2  下半屏绿 */
#define HUB_PIN_B2          12        /* PE12 (脚18) -> 排线 B2  下半屏蓝 */
#define HUB_PIN_CLK         13        /* PE13 (脚19) -> 排线 CLK */
#define HUB_PIN_LAT         14        /* PE14 (脚20) -> 排线 LAT/STB */
#define HUB_PIN_OE          15        /* PE15 (脚21) -> 排线 OE */

/* ---------- 行选：PB10 ~ PB14（H1 的 23~26 脚附近） ---------- */
#define HUB_ADDR_GPIO       GPIOB
#define HUB_ADDR_SHIFT      10        /* A=PB10 B=PB11 C=PB12 D=PB13 E=PB14 */

/* ---------- 板载 LED：心跳指示（不用接线） ---------- */
#define HEARTBEAT_GPIO      GPIOB
#define HEARTBEAT_PIN       2         /* PB2，高电平点亮 */

#endif /* BOARD_PINS_H */
