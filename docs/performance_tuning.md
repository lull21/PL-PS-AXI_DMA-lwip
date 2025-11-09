# 性能优化建议（AXI DMA + LWIP TCP）

本文档针对PL→PS→AXI DMA→LWIP TCP链路的典型瓶颈与优化策略，目标是在稳定前提下提升总吞吐。

## 硬件侧
- 使用 HP0/HP2 接口直连 DDR，避免共享拥塞；
- AXI DMA 配置：
  - 使能 S2MM 简单模式或SG模式（SG在大批量长数据更有优势）；
  - 对齐数据到缓存行与突发边界；
  - 合理设置最大突发长度与突发对齐（由硬件设计决定）。
- PL 端输出：
  - 连续写入大块数据，减少小包分割；
  - 与 `DMA_RING_SEGMENT_SIZE` 保持一致。

## 软件侧（PS）
- 环形缓冲：
  - 提升 `DMA_RING_SEGMENT_SIZE`（如 16KB/64KB）与 `DMA_RING_SEGMENT_COUNT`（如 16/32），增大流水深度；
  - 避免单段过小导致 `pbuf` 创建与 `tcp_write` 调用频繁。
- 缓存维护：
  - 仅对已完成段做 `InvalidateRange`，避免过度维护；
  - 段大小对齐缓存行，减少无效行带来的额外开销。
- TCP发送：
  - 合并多个 `pbuf` 后再调用 `tcp_output()`，减少报文层开销；
  - 调整 `tcp_sndbuf`、`TCP_WND` 与 `MEMP_NUM_PBUF`、`PBUF_POOL_SIZE`；
  - 若网络可靠且要求高吞吐，关闭 `TCP_NODELAY` 合并小包（或使用MSS对齐分片）。
- 轮询与中断：
  - 高吞吐场景下轮询更可控；低频率且节能场景可改用中断。

## 网络侧（PC）
- 使用千兆/万兆以太网，确保链路无瓶颈；
- PC端客户端应使用大接收缓冲与高效读取方式（如 `recv` 大块读取）。

## 指标与目标
- CPU占用（PS核）稳定在可接受范围；
- 无丢包、无环溢出；
- 吞吐达到硬件设计预期（示例：≥100MB/s，GigE上限约120MB/s）。