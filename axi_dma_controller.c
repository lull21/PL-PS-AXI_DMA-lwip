/*
 * AXI DMA (S2MM) 控制器实现（简单双缓冲/多段环形捕获）
 */

#include "axi_dma_controller.h"
#include "xaxidma_hw.h"

static XAxiDma gAxiDma;
static AxiDmaRing gRing;

static inline void pre_invalidate(UINTPTR addr, u32 len)
{
    /* 在PL通过S2MM写DDR之前，先失效对应缓存，避免旧缓存污染后续读取 */
    Xil_DCacheInvalidateRange(addr, len);
}

static int start_transfer_for_slot(u32 slot)
{

    UINTPTR dst = gRing.base_addr + ((UINTPTR)slot * gRing.segment_size);
    pre_invalidate(dst, gRing.segment_size);

    int rc = XAxiDma_SimpleTransfer(&gAxiDma, dst, gRing.segment_size,
                                    XAXIDMA_DEVICE_TO_DMA);
    if (rc != XST_SUCCESS) {
        gRing.error_cnt++;
        return rc;
    }
    gRing.active_slot = slot;
    gRing.dma_busy = 1;
    return 0;
}

int axi_dma_init(void)
{
    XAxiDma_Config *cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    if (cfg == NULL) {
        xil_printf("AXI DMA config lookup failed\r\n");
        return XST_FAILURE;
    }

    int rc = XAxiDma_CfgInitialize(&gAxiDma, cfg);
    if (rc != XST_SUCCESS) {
        xil_printf("AXI DMA init failed %d\r\n", rc);
        return rc;
    }

    if (XAxiDma_HasSg(&gAxiDma)) {
        xil_printf("AXI DMA is configured with SG, but this module expects simple mode.\r\n");
    }

    /* 复位并等待完成 */
    XAxiDma_Reset(&gAxiDma);
    while (!XAxiDma_ResetIsDone(&gAxiDma)) {}

    /* 使能S2MM完成与错误中断（如未注册ISR，可用axi_dma_poll轮询） */
    XAxiDma_IntrDisable(&gAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    XAxiDma_IntrEnable(&gAxiDma,
                       (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK),
                       XAXIDMA_DEVICE_TO_DMA);

    /* 环形缓冲区初始化 */
    gRing.base_addr     = DMA_BUFFER_BASE;
    gRing.segment_size  = DMA_SEGMENT_SIZE;
    gRing.segment_count = DMA_RING_SEGMENTS;
    gRing.write_index   = 0;
    gRing.read_index    = 0;
    gRing.overflow_cnt  = 0;
    gRing.error_cnt     = 0;
    gRing.dma_busy      = 0;
    gRing.running       = 0;
    gRing.active_slot   = 0;
    for (u32 i = 0; i < DMA_RING_SEGMENTS; i++) gRing.ready[i] = 0U;

    xil_printf("AXI DMA initialized. S2MM only, base=0x%08lx, seg=%lu, cnt=%lu\r\n",
               (unsigned long)gRing.base_addr,
               (unsigned long)gRing.segment_size,
               (unsigned long)gRing.segment_count);
    return XST_SUCCESS;
}

int axi_dma_start(void)
{
    if (gRing.running) return XST_SUCCESS;

    /* 从write_index对应段启动首次传输 */
    int rc = start_transfer_for_slot(gRing.write_index);
    if (rc != 0) return XST_FAILURE;

    gRing.running = 1;
    return XST_SUCCESS;
}

void axi_dma_stop(void)
{
    gRing.running = 0;
    /* 停止S2MM通道 */
    /* 设置RUNSTOP为0：使用寄存器写实现停止 */
    u32 cr = XAxiDma_ReadReg(gAxiDma.RegBase + (XAXIDMA_RX_OFFSET * XAXIDMA_DEVICE_TO_DMA),
                             XAXIDMA_CR_OFFSET);
    cr &= ~XAXIDMA_CR_RUNSTOP_MASK;
    XAxiDma_WriteReg(gAxiDma.RegBase + (XAXIDMA_RX_OFFSET * XAXIDMA_DEVICE_TO_DMA),
                     XAXIDMA_CR_OFFSET, cr);
    gRing.dma_busy = 0;
}

u32 axi_dma_ring_filled_slots(void)
{
    u32 count = 0;
    for (u32 i = 0; i < gRing.segment_count; i++) {
        if (gRing.ready[i]) count++;
    }
    return count;
}

u32 axi_dma_ring_free_slots(void)
{
    return gRing.segment_count - axi_dma_ring_filled_slots();
}

int axi_dma_get_filled(void **addr, u32 *len)
{
    if (!gRing.ready[gRing.read_index]) return -1;
    UINTPTR p = gRing.base_addr + ((UINTPTR)gRing.read_index * gRing.segment_size);
    *addr = (void *)p;
    *len  = gRing.segment_size;
    /* 再次失效该段缓存，保证读取的数据一致 */
    Xil_DCacheInvalidateRange(p, gRing.segment_size);
    return 0;
}

void axi_dma_release(void)
{
    gRing.ready[gRing.read_index] = 0U;
    gRing.read_index = (gRing.read_index + 1) % gRing.segment_count;
}

void axi_dma_isr(void *CallbackRef)
{
    (void)CallbackRef;
    u32 irq = XAxiDma_IntrGetIrq(&gAxiDma, XAXIDMA_DEVICE_TO_DMA);
    if (!(irq & XAXIDMA_IRQ_ALL_MASK)) return;

    /* 清中断 */
    XAxiDma_IntrAckIrq(&gAxiDma, irq, XAXIDMA_DEVICE_TO_DMA);

    if (irq & XAXIDMA_IRQ_ERROR_MASK) {
        gRing.error_cnt++;
        /* 出错后复位DMA */
        XAxiDma_Reset(&gAxiDma);
        while (!XAxiDma_ResetIsDone(&gAxiDma)) {}
        gRing.dma_busy = 0;
        /* 尝试恢复：如果仍运行则重启当前段 */
        if (gRing.running) {
            (void)start_transfer_for_slot(gRing.write_index);
        }
        return;
    }

    if (irq & XAXIDMA_IRQ_IOC_MASK) {
        /* 标记当前活动段完成 */
        gRing.ready[gRing.active_slot] = 1U;
        gRing.dma_busy = 0;
        /* 推进write_index */
        u32 next = (gRing.active_slot + 1) % gRing.segment_count;
        gRing.write_index = next;
        /* 如环满则不继续启动（对PL背压，避免溢出） */
        if (gRing.ready[next]) {
            gRing.overflow_cnt++;
            return;
        }
        /* 启动下一段 */
        (void)start_transfer_for_slot(next);
    }
}

void axi_dma_poll(void)
{
    /* 轮询SR寄存器的中断位，仿照ISR流程 */
    u32 irq = XAxiDma_IntrGetIrq(&gAxiDma, XAXIDMA_DEVICE_TO_DMA);
    if (!(irq & XAXIDMA_IRQ_ALL_MASK)) return;
    XAxiDma_IntrAckIrq(&gAxiDma, irq, XAXIDMA_DEVICE_TO_DMA);

    if (irq & XAXIDMA_IRQ_ERROR_MASK) {
        gRing.error_cnt++;
        XAxiDma_Reset(&gAxiDma);
        while (!XAxiDma_ResetIsDone(&gAxiDma)) {}
        gRing.dma_busy = 0;
        if (gRing.running) (void)start_transfer_for_slot(gRing.write_index);
        return;
    }
    if (irq & XAXIDMA_IRQ_IOC_MASK) {
        gRing.ready[gRing.active_slot] = 1U;
        gRing.dma_busy = 0;
        u32 next = (gRing.active_slot + 1) % gRing.segment_count;
        gRing.write_index = next;
        if (gRing.ready[next]) {
            gRing.overflow_cnt++;
            return;
        }
        (void)start_transfer_for_slot(next);
    }
}

u32 axi_dma_query_status(void)
{
    /* 返回一个简单的状态位图/计数：
     * bit0: dma_busy
     * bit1: running
     * bit2: 有错误（error_cnt>0）
     * bit3: 溢出（overflow_cnt>0）
     * 高16位：error_cnt，低16位：overflow_cnt
     */
    u32 status = 0U;
    if (gRing.dma_busy) status |= (1U << 0);
    if (gRing.running)  status |= (1U << 1);
    if (gRing.error_cnt)    status |= (1U << 2);
    if (gRing.overflow_cnt) status |= (1U << 3);
    status |= ((gRing.error_cnt & 0xFFFFU) << 16);
    status |= (gRing.overflow_cnt & 0xFFFFU);
    return status;
}