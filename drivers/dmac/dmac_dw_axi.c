#include <asm/mmio.h>
#include <asm/sbi.h>
#include <device.h>
#include <print.h>
#include <asm/type.h>
#include <dmac.h>
#include <timer.h>
#include <string.h>
#include "dmac_dw_axi.h"
#include "event.h"

static int done = 0;
static unsigned long base;

static void dw_dmac_irq_handler(void *data)
{
	done = 1;

	writel(base + DMAC_AXI0_CH1_INTR_CLEAR, 0x3);
}

static int dw_dmac_mem_to_mem(unsigned int src_addr,
			      unsigned int des_addr,
			      unsigned int blockTS,
			      unsigned int src_addr_inc,
			      unsigned int des_addr_inc,
			      unsigned int src_width,
			      unsigned int des_width,
			      unsigned int src_burstsize,
			      unsigned int des_burstsize,
			      unsigned int burst_len)
{
	unsigned long ch1_cfg;
	unsigned long ch1_cfg_high;
	unsigned long ch1_ctl;
	unsigned long ch1_ctl_hi;
	unsigned int type = 0;
	unsigned int src_hs = 0;
	unsigned int des_hs = 0;

	print
	    ("src_addr: 0x%x des_addr: 0x%x blockTS: %d src_addr_inc: %d des_addr_inc: %d src_width: %d des_width: %d src_burstsize: %d des_burstsize: %d burst_len: %d\n",
	     src_addr, des_addr, blockTS, src_addr_inc, des_addr_inc, src_width,
	     des_width, src_burstsize, des_burstsize, burst_len);

	/* check dmac reset complete */
	while ((readl(base + DMAC_AXI0_COMMON_RST_REG) & 0x01) != 0) ;

	/* check CH1 is idle */
	while ((readl(base + DMAC_AXI0_COMMON_CH_EN) & 0x01) != 0) ;

	/* CHANNEL_CFG */
	ch1_cfg = readl(base + DMAC_AXI0_CH1_CFG);
	ch1_cfg_high = readl(base + DMAC_AXI0_CH1_CFG + 0x4);

	/* multi-block type */
	ch1_cfg = (ch1_cfg & 0xfffffffc) | (0x0 << 0);	// src multi-block type -> contiguous
	ch1_cfg = (ch1_cfg & 0xfffffff3) | (0x0 << 2);	// des multi-block type -> contiguous
	writel(base + DMAC_AXI0_CH1_CFG, ch1_cfg);

	/* transfer type and flow control */
	ch1_cfg_high = (ch1_cfg_high & 0xfffffff8) | (type << 0);	// peri to memory, dmac is flow control
	/* hardware handshake select */
	ch1_cfg_high = (ch1_cfg_high & 0xffffffe7) | (0x0 << 3);	// src hw handshake
	ch1_cfg_high = (ch1_cfg_high & 0xffffffef) | (0x0 << 4);	// des sw handshake
	/* handshake polarity */
	ch1_cfg_high = (ch1_cfg_high & 0xffffffdf) | (0x0 << 5);	// src hw handshake high active
	ch1_cfg_high = (ch1_cfg_high & 0xffffffbf) | (0x0 << 6);	// des hw handshake high active
	/* hw handshake interface */
	ch1_cfg_high = (ch1_cfg_high & 0xffffff7f) | (src_hs << 7);	// src hw interface 1
	ch1_cfg_high = (ch1_cfg_high & 0xffffefff) | (des_hs << 12);	// des hw interface 1
	/* channel priority */
	ch1_cfg_high = (ch1_cfg_high & 0xfff1ffff) | (0x7 << 17);	// the highest priority
	writel(base + DMAC_AXI0_CH1_CFG + 0x4, ch1_cfg_high);

	/* CHANNEL_SRC_ADDR */
	writel(base + DMAC_AXI0_CH1_SAR, src_addr);	// src address -> 0x0
	/* CHANNEL_DES_ADDR */
	writel(base + DMAC_AXI0_CH1_DAR, des_addr);	// des address -> 0x1f0000

	/* TRANSFER_BLOCK_SIZE */
	writel(base + DMAC_AXI0_CH1_BLOCK_TS, blockTS);	// block size 14*32bit = 56 bytes

	/* CHANNEL_CTRL */
	ch1_ctl = readl(base + DMAC_AXI0_CH1_CTL);
	//ch1_ctl_high = REG_READ(DMAC_AXI0_CH1_CTL + 0x4);

	/* address increment */
	ch1_ctl = (ch1_ctl & 0xffffffef) | (src_addr_inc << 4);	// src address increment
	ch1_ctl = (ch1_ctl & 0xffffffbf) | (des_addr_inc << 6);	// des address increment

	/* transfer width */
	ch1_ctl = (ch1_ctl & 0xfffff8ff) | (src_width << 8);	// src transfer width 32bit
	ch1_ctl = (ch1_ctl & 0xffffc7ff) | (des_width << 11);	// des transfer width 32bit

	/* burst transanction size */
	ch1_ctl = (ch1_ctl & 0xfffc3fff) | (src_burstsize << 14);	// src 1 data item
	ch1_ctl = (ch1_ctl & 0xffc3ffff) | (des_burstsize << 18);	// des 1 data item

	writel(base + DMAC_AXI0_CH1_CTL, ch1_ctl);

	ch1_ctl_hi = readl(base + DMAC_AXI0_CH1_CTL_HI);
	ch1_ctl_hi = (ch1_ctl_hi & 0xff00ffff) | (burst_len << 16);	//AWLEN
	ch1_ctl_hi = (ch1_ctl_hi & 0xffff7fff) | (1 << 15);	//AWLEN
	ch1_ctl_hi = (ch1_ctl_hi & 0xffff807f) | (burst_len << 7);	//AWLEN
	ch1_ctl_hi = (ch1_ctl_hi & 0xffffffbf) | (1 << 6);	//AWLEN
	writel(base + DMAC_AXI0_CH1_CTL_HI, ch1_ctl_hi);

	/* CHANNEL_ENABLE */
	writel(base + DMAC_AXI0_COMMON_CH_EN,
	       readl(base + DMAC_AXI0_COMMON_CH_EN) | 0x101);

	return 0;
}

static int wake_expr(void *data)
{
	int *wake = (int *)data;

	return *wake == 1;
}

static int dw_dmac_ioctl(unsigned int cmd, void *arg)
{
	int ret = 0;
	struct dmac_ioctl_data *data = (struct dmac_ioctl_data *)arg;
	unsigned long src_addr;
	unsigned long des_addr;
	unsigned int blockTS;
	unsigned int src_addr_inc;
	unsigned int des_addr_inc;
	unsigned int src_width;
	unsigned int des_width;
	unsigned int src_burstsize;
	unsigned int des_burstsize;
	unsigned int burst_len;
	unsigned long start_cycles = 0, end_cycles = 0;

	switch (cmd) {
	case MEM_TO_MEM:
		src_addr = (unsigned long)data->src;
		des_addr = (unsigned long)data->dst;
		blockTS = data->blockTS;
		src_addr_inc = data->src_addr_inc;
		des_addr_inc = data->des_addr_inc;
		src_width = data->src_width;
		des_width = data->des_width;
		src_burstsize = data->src_burstsize;
		des_burstsize = data->des_burstsize;
		burst_len = data->burst_len;

		dw_dmac_mem_to_mem((unsigned int)src_addr,
				   (unsigned int)des_addr,
				   blockTS,
				   src_addr_inc,
				   des_addr_inc,
				   src_width,
				   des_width,
				   src_burstsize, des_burstsize, burst_len);

		start_cycles = sbi_get_cpu_cycles();
		print("start dma transfer m2m, get system cycles: %d\n",
		      start_cycles);

		wait_for_event_timeout(&done, wake_expr, 5 * 1000 /* 5s */ );
		if (done == 0)
			ret = -1;
		else {
			done = 0;
			ret = 0;
			end_cycles = sbi_get_cpu_cycles();
			print("end dma transfer m2m, get system cycles: %d\n",
			      end_cycles);
			print("cycles diff: %d\n", end_cycles - start_cycles);
		}

		break;

	default:
		print("%s unsupported cmd: %d\n", __FUNCTION__, cmd);

	}

	return ret;
}

static const struct driver_ops dw_dmac_ops = {
	.ioctl = dw_dmac_ioctl,
};

int dw_dmac_init(struct device *dev, void *data)
{
	struct driver *drv;
	int ret = 0;

	print("%s %d base: 0x%x, len: %d, irq: %d\n", __FUNCTION__, __LINE__,
	      dev->start, dev->len, dev->irq);

	base = dev->start;

	ret = register_device_irq(dev->irq, dw_dmac_irq_handler, NULL);
	if (ret == -1)
		print("%s register device irq failed. irq=%d\n", __FUNCTION__,
		      dev->irq);

	/* enable interrupt */
	writel(base + DMAC_AXI0_CH1_INTR_STATUS_ENABLE, 0x3);
	writel(base + DMAC_AXI0_CH2_INTR_STATUS_ENABLE, 0x3);

	/* enable dmac_axi0 and interrupt */
	print("DMAC: check dma reset.\n");
	writel(base + DMAC_AXI0_COMMON_CFG, 0x3);

	drv = dev->drv;
	strcpy(dev->name, "DMAC0");
	strcpy(drv->name, "DMAC0");
	drv->ops = &dw_dmac_ops;

	return 0;
}

DRIVER_REGISTER(dw_dmac, dw_dmac_init, "dw,dmac");
