# 集成测试方案：PL→PS→AXI DMA→LWIP TCP→PC

目标是验证在ZynqMP平台上，PL产生数据经AXI DMA(S2MM)写入PS DDR，并由LWIP TCP服务器稳定发送到PC端客户端的完整链路。

## 前置条件
- 设备：Zynq UltraScale+ MPSoC，PL已实现AXI DMA S2MM写入逻辑。
- Vivado硬件：AXI DMA 使能 S2MM 简单模式，连接到 S_AXI_HP0_FPD（或其他HP端口）与 DDR。
- 软件：本工程已集成 `axi_dma_controller` 与 `tcp_perf_server_service`。
- PC端：使用任意TCP客户端（建议 `python`, `netcat`, 或 `iperf` 自定义客户端）。

## 配置检查
- 确认 `xparameters.h` 中 `XPAR_AXIDMA_0_BASEADDR` 存在。
- 确认 DDR 范围（`XPAR_DDR_MEM_BASEADDR` ~ `XPAR_DDR_MEM_HIGHADDR`），`DMA_RING_BASE_ADDR` 落在此范围。
- PL 与 PS 对齐：`DMA_RING_SEGMENT_SIZE` 与 PL输出块大小一致。

## 测试步骤
1. 启动板卡，串口观察启动日志：
   - 平台初始化、RFdc与时钟配置完成；
   - LWIP 初始化与TCP服务器监听成功；
   - DMA初始化成功（打印设备ID与环参数）。
2. PC端连接TCP：
   - 记录板卡IP与端口（默认 `TCP_SERVER_PORT`）。
   - 在PC运行客户端连接板卡，例如：
     - `python` 简单客户端或 `nc <ip> <port>`。
3. 触发PL产生数据：
   - 让PL开始写入AXI DMA S2MM（硬件侧）；
   - 观察串口：`axi_dma_start`自动触发，`completed_count`逐步增加。
4. 接收数据验证：
   - PC端持续接收数据；
   - 验证数据总量、内容模式（可在PL端发送特定序列以校验完整性）。
5. 压力/稳定性测试：
   - 长时间（≥30分钟）运行，观察是否出现LWIP发送阻塞或DDR环溢出；
   - 统计吞吐与CPU占用。

## 观测与日志
- 串口：DMA环状态（`head/tail/completed_count`）、TCP发送长度、错误回调。
- PC端：记录接收速率与数据校验结果。

## 通过准则
- 无崩溃/异常终止（`tcp_server_err` 未触发）。
- 无环形缓冲溢出（`ring_free_slots` 充足）。
- TCP发送稳定，吞吐达到目标（例如 ≥100MB/s，视硬件设计与网络配置）。

## 故障排查
- 若无数据：
  - 检查PL是否触发S2MM写入；
  - 检查 `DMA_RING_BASE_ADDR` 是否正确映射到HP端口；
  - 检查缓存一致性（确认已调用 `Xil_DCacheInvalidateRange`）。
- 若吞吐低：
  - 增大 `DMA_RING_SEGMENT_SIZE` 与 `DMA_RING_SEGMENT_COUNT`；
  - 调整TCP窗口与 `tcp_sndbuf`；
  - 采用批量 `pbuf` 发送与减少 `tcp_output` 调用频次。
- 若报错 `ERR_MEM`：
  - 增大 `MEMP_NUM_PBUF`, `PBUF_POOL_SIZE`，或改为零拷贝方式（需更复杂结构）。