# AXI DMA 控制器与LWIP集成接口说明

本文档描述 `axi_dma_controller.c/h` 暴露的API、平台层封装与在 `tcp_perf_server` 的集成方式，帮助在ZynqMP平台上实现 PL→PS（S2MM）→TCP（LWIP）的数据通路。

## 设计概述
- 通道：使用 AXI DMA S2MM 通道从 PL 写入到 PS DDR。
- 缓冲：采用环形缓冲区，多个段反复周转，避免单缓冲阻塞。
- 调度：默认轮询模式（可后续接入中断），从环中取已完成段，切片拷贝到 TCP 的 `pbuf` 并发送。
- 内存一致性：在段读取前执行 `Xil_DCacheInvalidateRange` 和 `dsb`，保证PS看到PL更新的数据。

## 关键宏配置
- `DMA_DEVICE_ID`：AXI DMA设备ID（来自 `xparameters.h`）。
- `PS_DDR_BASE` / `PS_DDR_HIGH`：DDR有效地址范围（来自 `xparameters_ps.h`）。
- `DMA_RING_BASE_ADDR`：环形缓冲区首地址，必须落在DDR范围内，且与PL协商。
- `DMA_RING_SEGMENT_SIZE`：单段大小，建议与PL输出对齐（如 4096/8192/16384）。
- `DMA_RING_SEGMENT_COUNT`：段个数，常见为 8/16/32。

## 结构体
```c
typedef struct {
    XAxiDma AxiDma;
    volatile u8 started;
    volatile u8 use_irq;
    volatile u32 completed_count;

    UINTPTR ring_base;
    u32 segment_size;
    u32 segment_count;
    volatile u32 head; // 已完成段的读取索引
    volatile u32 tail; // 已发起段的写入索引
} AxiDmaRing;
```

## API
- `int axi_dma_init(AxiDmaRing *ring, u16 dev_id, UINTPTR base, u32 sz, u32 cnt, u8 use_irq)`
  - 初始化驱动、校验参数并设置环形缓冲区。
- `int axi_dma_start(AxiDmaRing *ring)` / `void axi_dma_stop(AxiDmaRing *ring)`
  - 启动/停止捕获；启动阶段为每个段发起一次S2MM（简单模式）。
- `int axi_dma_poll(AxiDmaRing *ring)`
  - 轻量轮询更新完成计数与 `head`。
- `int axi_dma_get_filled(AxiDmaRing *ring, UINTPTR *addr, u32 *len)`
  - 获取一个已完成段地址与长度（`len==segment_size`）；内部做缓存失效与数据屏障。
- `void axi_dma_release(AxiDmaRing *ring)`
  - 释放刚取出的段（移动 `head`）。
- `void axi_dma_isr(void *cb)`
  - 预留中断处理（当前工程默认轮询）。
- `void axi_dma_query_status(AxiDmaRing *ring)`
  - 打印环状态，用于调试。

## 平台层封装
- `void platform_dma_barrier(UINTPTR addr, u32 len)`
  - DDR缓存失效与数据屏障，确保内存可见性。
- `int platform_dma_register_isr(Xil_ExceptionHandler Handler, void *Ref)`
  - 预留DMA中断注册，当前打印提示并返回成功。

## LWIP 集成
在 `tcp_perf_server.c` 中新增：
- `static void tcp_server_dma_poll(struct tcp_pcb *tpcb)`
  - 从 `AxiDmaRing` 拉取完成段，切片构造 `pbuf`，调用 `tcp_write()`/`tcp_output()` 发送。
- `void tcp_server_service(void)`
  - 在主循环中调用，统一调度LWIP定时与DMA→TCP发送。

## 使用范式
```c
// main.c
static AxiDmaRing g_dma_ring;
axi_dma_init(&g_dma_ring, DMA_DEVICE_ID, DMA_RING_BASE_ADDR,
             DMA_RING_SEGMENT_SIZE, DMA_RING_SEGMENT_COUNT, 0);
// 启动后在 tcp_server_dma_poll 内第一次自动调用 axi_dma_start()

while (1) {
    // 维持LWIP协议栈定时与TCP发送
    tcp_server_service();
}
```

## 注意事项
- HP0/HP2接口与PS-DDR映射需在硬件设计中正确连接；软件仅使用有效DDR地址范围。
- PL与PS对环形布局（基地址、段大小、段数）需一致。
- `segment_size` 建议缓存行对齐（64B/128B）以减少缓存维护开销。