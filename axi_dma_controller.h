/*
 * AXI DMA Controller (S2MM) ring-buffer helper
 *
 * 提供：
 * - DMA初始化（基于xparameters.h）
 * - S2MM环形缓冲区管理（简单双缓冲/多段轮转）
 * - 中断服务例程与轮询模式
 * - 传输状态查询接口
 */

#ifndef AXI_DMA_CONTROLLER_H
#define AXI_DMA_CONTROLLER_H

#include "xaxidma.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xil_cache.h"
#include "xil_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DDR缓冲区配置（位于PS DDR地址空间） */
#ifndef DMA_BUFFER_BASE
#define DMA_BUFFER_BASE     0x10000000U /* 落在 XPAR_PSU_DDR_0_S_AXI_BASEADDR(0x0) ~ 0x7FFFFFFF 范围内 */
#endif

#ifndef DMA_SEGMENT_SIZE
#define DMA_SEGMENT_SIZE    (256 * 1024U) /* 每段大小：256KB，可根据吞吐需求调整 */
#endif

#ifndef DMA_RING_SEGMENTS
#define DMA_RING_SEGMENTS   16U           /* 段数量：16段，总缓冲大小 4MB */
#endif

typedef struct {
    UINTPTR base_addr;        /* DDR起始地址（物理） */
    u32     segment_size;     /* 每段长度 */
    u32     segment_count;    /* 段数量 */
    volatile u32 write_index; /* 下一个用于DMA写入的段索引 */
    volatile u32 read_index;  /* 下一个供PS读取/发送的段索引 */
    volatile u8  ready[DMA_RING_SEGMENTS]; /* 段完成标记 */
    volatile u32 overflow_cnt;/* 环满导致未启动下一次DMA的计数 */
    volatile u32 error_cnt;   /* DMA错误计数 */
    volatile int dma_busy;    /* 当前是否有S2MM正在进行 */
    volatile int running;     /* DMA环是否处于运行状态 */
    u32 active_slot;          /* 当前正在写入的段索引 */
} AxiDmaRing;

/* 初始化AXI DMA控制器与环形缓冲 */
int axi_dma_init(void);

/* 启动S2MM环形捕获（轮转段，自动续传） */
int axi_dma_start(void);

/* 停止S2MM捕获 */
void axi_dma_stop(void);

/* 轮询DMA中断状态（无中断时可用），内部会标记段完成并续传 */
void axi_dma_poll(void);

/* 查询环中已完成但未读取的段数量 */
u32 axi_dma_ring_filled_slots(void);

/* 查询环中可用空闲段数量 */
u32 axi_dma_ring_free_slots(void);

/* 取出一个已完成段的地址与长度（NO_COPY发送前使用），返回0成功，非0失败 */
int axi_dma_get_filled(void **addr, u32 *len);

/* 读取完成后释放该段，推进read_index（NO_COPY发送ACK后调用） */
void axi_dma_release(void);

/* DMA完成中断服务例程（需要在平台层注册到GIC/INTC） */
void axi_dma_isr(void *CallbackRef);

/* 简单状态查询：busy/错误/溢出等（位图或计数） */
u32 axi_dma_query_status(void);

#ifdef __cplusplus
}
#endif

#endif /* AXI_DMA_CONTROLLER_H */