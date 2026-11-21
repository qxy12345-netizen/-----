/**
 * @file    hub75.h
 * @brief   HUB75 (64x64 / 1-32 扫描) 纯 GPIO 位操作驱动 —— STM32F407VET6 版
 *
 * 内存账（虽然 F407 有 128KB SRAM，还是保持最省的写法）：
 *   位平面 32 地址 x 4 位平面 x 64 列 = 8192 字节
 *   BSRR 查表 2 x 64 x 4                   =  512 字节
 * 合计约 8.7KB，剩下的 11KB 留给栈和变量。
 *
 * 所以这一版**不做帧缓冲**：画图直接写进位平面，省掉 8KB。
 * 代价是以后想做动画/叠加图层会麻烦一点，但 20KB 里这是必要的取舍。
 */
#ifndef HUB75_H_INCLUDED
#define HUB75_H_INCLUDED

#include <stdint.h>

#define HUB75_WIDTH     64
#define HUB75_HEIGHT    64

/* 位平面（BCM）最小时的显示时间，单位 us。
   调大 = 更亮、刷新率下降；调小 = 更暗、刷新率更高。
   168MHz 下 16us 时整屏刷新约 116Hz。 */
#ifndef HUB75_BCM_BASE_US
#define HUB75_BCM_BASE_US   16u
#endif

/* 数据线在 CLK 边沿前后的稳定时间（NOP 个数）。
   屏出现噪点/串色/竖条纹就往上加。
   168MHz 下 1 个 NOP 只有 6ns，先用 3（约 18ns）；
   飞线用长杜邦线时建议加到 6~8。 */
#ifndef HUB75_CLK_NOP
#define HUB75_CLK_NOP   3
#endif

/* 画面镜像/倒置？改这里，不用动接线。
   例：数字 3 显示成 "Ɛ"（左右镜像）-> HUB75_FLIP_X 改 1
       数字上下颠倒                    -> HUB75_FLIP_Y 改 1 */
#ifndef HUB75_FLIP_X
#define HUB75_FLIP_X    0
#endif
#ifndef HUB75_FLIP_Y
#define HUB75_FLIP_Y    0
#endif

/** @brief 初始化 14 个 GPIO 并预计算 BSRR 查找表 */
void hub75_init(void);

/** @brief 清屏（所有像素熄灭） */
void hub75_clear(void);

/** @brief 整屏填同一个颜色（每通道 0~15） */
void hub75_fill(uint8_t r, uint8_t g, uint8_t b);

/** @brief 调试用：数据线常高 + OE 常低，不扫描。量屏引脚电平时用 */
void hub75_static_white(void);

/** @brief 画一个点（越界自动忽略），每通道 0~15 */
void hub75_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

/** @brief 扫描一整帧（阻塞）。要持续显示就死循环调用它 */
void hub75_scan_frame(void);

/** @brief 连续扫描 n 帧（开机自检用） */
void hub75_hold_frames(uint32_t frames);

#endif /* HUB75_H_INCLUDED */
