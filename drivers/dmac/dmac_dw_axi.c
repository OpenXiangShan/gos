/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <asm/mmio.h>
#include <asm/sbi.h>
#include <device.h>
#include <print.h>
#include <asm/type.h>
#include <dmac.h>
#include <string.h>
#include "dmac_dw_axi.h"
#include "event.h"
#include "vmap.h"
#include "irq.h"
#include "mm.h"

static struct dmac_dw_axi *dw_axi_dmac;

static void wait_for_dmac_complete(void)
{
	unsigned int tmp;
	tmp = readl(dw_axi_dmac->base + DMAC_AXI0_CH1_INTR_STATUS);
	while((tmp & 0x2) == 0x0)
	{
		tmp = readl(dw_axi_dmac->base + DMAC_AXI0_CH1_INTR_STATUS);
	}
}

static void dw_dmac_irq_handler(void *data)
{
	dw_axi_dmac->done = 1;
	wait_for_dmac_complete();
	writel(dw_axi_dmac->base + DMAC_AXI0_CH1_INTR_CLEAR, 0x3);
}

static int dw_dmac_mem_to_mem(unsigned long src_addr,
			      unsigned long des_addr,
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
	unsigned long src_hs = 0;
	unsigned long des_hs = 0;
	unsigned long base = dw_axi_dmac->base;

	print
	    ("src_addr: 0x%lx des_addr: 0x%lx blockTS: %d src_addr_inc: %d des_addr_inc: %d src_width: %d des_width: %d src_burstsize: %d des_burstsize: %d burst_len: %d\n",
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
	writeq(base + DMAC_AXI0_CH1_SAR, src_addr);	// src address -> 0x0
	/* CHANNEL_DES_ADDR */
	writeq(base + DMAC_AXI0_CH1_DAR, des_addr);	// des address -> 0x1f0000

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

static int dw_axi_dmac_transfer_m2m(unsigned long src, unsigned long dst, int size, void *priv)
{
	int ret = 0;
	struct dw_dmac_priv_info *info = (struct dw_dmac_priv_info *)priv;
	unsigned long src_addr = src;
	unsigned long des_addr = dst;
	unsigned int blockTS = (size >> info->dma_width) - 1;
	unsigned int src_addr_inc = info->src_addr_inc;
	unsigned int des_addr_inc = info->des_addr_inc;
	unsigned int src_width = info->src_width;
	unsigned int des_width = info->des_width;
	unsigned int src_burstsize = info->src_burstsize;
	unsigned int des_burstsize = info->des_burstsize;
	unsigned int burst_len = info->burst_len;

	dw_dmac_mem_to_mem(src_addr,
			   des_addr,
			   blockTS,
			   src_addr_inc,
			   des_addr_inc,
			   src_width,
			   des_width,
			   src_burstsize, des_burstsize, burst_len);

	wait_for_event_timeout(&dw_axi_dmac->done, wake_expr, 5 * 1000 /* 5s */ );
	if (dw_axi_dmac->done == 0)
		ret = -1;
	else {
		dw_axi_dmac->done = 0;
		ret = 0;
	}
	return ret;
}

static struct dmac_ops dw_axi_dmac_ops = {
	.transfer_m2m = dw_axi_dmac_transfer_m2m,
};

int dw_dmac_init(struct device *dev, void *data)
{
	struct dmac_device *dmac;
	int irqs[16], nr_irqs, i;
	struct dw_dmac_priv_info *info;

	print("dw-dmac: base: 0x%x, len: %d, irq: %d\n",
	      dev->base, dev->len, dev->irqs[0]);

	dw_axi_dmac = (struct dmac_dw_axi *)mm_alloc(sizeof(struct dmac_dw_axi));
	if (!dw_axi_dmac) {
		print("dw-dmac: alloc dw_axi_dmac failed\n");
		return -1;
	}

	dw_axi_dmac->base = (unsigned long)ioremap((void *)dev->base, dev->len, 0);

	nr_irqs = get_hwirq(dev, irqs);
	for (i = 0; i < nr_irqs; i++)
		register_device_irq(dev, dev->irq_domain, irqs[i],
				    dw_dmac_irq_handler, NULL);
	/* enable interrupt */
	writel(dw_axi_dmac->base + DMAC_AXI0_CH1_INTR_STATUS_ENABLE, 0x3);
	writel(dw_axi_dmac->base + DMAC_AXI0_CH2_INTR_STATUS_ENABLE, 0x3);

	/* enable dmac_axi0 and interrupt */
	print("DMAC: check dma reset.\n");
	writel(dw_axi_dmac->base + DMAC_AXI0_COMMON_CFG, 0x3);

	info = (struct dw_dmac_priv_info *)mm_alloc(sizeof(struct dw_dmac_priv_info));
	info->dma_width = 0;
	info->src_addr_inc = 0;
	info->des_addr_inc = 0;
	info->src_width = info->dma_width;
	info->des_width = info->dma_width;
	info->src_burstsize = 0;
	info->des_burstsize = 0;
	info->burst_len = 7;

	dmac = &dw_axi_dmac->dmac;
	dmac->dev = dev;
	dmac->ops = &dw_axi_dmac_ops;
	dmac->priv = (void *)info;

	register_dmac_device(dmac);

	return 0;
}

DRIVER_REGISTER(dw_dmac, dw_dmac_init, "dw,dmac");