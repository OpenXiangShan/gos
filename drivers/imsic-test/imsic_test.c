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

#include "irq.h"
#include "device.h"
#include "string.h"
#include "print.h"
#include "asm/type.h"
#include "asm/mmio.h"
#include "asm/sbi.h"
#include "vmap.h"
#include "asm/pgtable.h"

extern int mmu_is_on;

struct msi_msg {
	unsigned long msi_addr_pa;
	unsigned long msi_addr;
	unsigned long msi_data;
};

static struct msi_msg msi_msg[3];
static int base_hwirq = 0;

static void imsic_test_irq_handler0(void *data)
{
	print("imsic test driver: ###### enter %s (cpu%d)\n", __FUNCTION__, sbi_get_cpu_id());
}

static void imsic_test_irq_handler1(void *data)
{
	print("imsic test driver: ###### enter %s (cpu%d)\n", __FUNCTION__, sbi_get_cpu_id());
}

static void imsic_test_irq_handler2(void *data)
{
	print("imsic test driver: ###### enter %s (cpu%d)\n", __FUNCTION__, sbi_get_cpu_id());
}

static void imsic_test_write_msi_msg(unsigned long msi_addr,
				     unsigned long msi_data, int hwirq,
				     void *priv)
{
	static int ii = 0;
	unsigned long msi_addr_va;

	print
	    ("imsic test driver: ##### %s %d base_hwirq:%d hwirq:%d msi_addr:0x%x msi_data:0x%x\n",
	     __FUNCTION__, __LINE__, base_hwirq, hwirq, msi_addr, msi_data);

	base_hwirq = 2;
	if (ii++ == 0)
		base_hwirq = hwirq;

	if (mmu_is_on)
		msi_addr_va = (unsigned long)ioremap((void *)msi_addr, PAGE_SIZE, 0);
	else
		msi_addr_va = msi_addr;

	print("imsic test driver: hwirq:%d base_hwirq:%d\n", hwirq, base_hwirq);
	msi_msg[hwirq - base_hwirq].msi_addr_pa = msi_addr;
	msi_msg[hwirq - base_hwirq].msi_addr = msi_addr_va;
	msi_msg[hwirq - base_hwirq].msi_data = msi_data;
}

static int imsic_test_ioctl(struct device *dev, unsigned int cmd, void *arg)
{
	unsigned long msi_addr, msi_data, msi_addr_pa;

	switch (cmd) {
	case 0:
		msi_addr_pa = msi_msg[0].msi_addr_pa;
		msi_addr = msi_msg[0].msi_addr;
		msi_data = msi_msg[0].msi_data;
		break;
	case 1:
		msi_addr_pa = msi_msg[1].msi_addr_pa;
		msi_addr = msi_msg[1].msi_addr;
		msi_data = msi_msg[1].msi_data;
		break;
	case 2:
		msi_addr_pa = msi_msg[1].msi_addr_pa;
		msi_addr = msi_msg[2].msi_addr;
		msi_data = msi_msg[2].msi_data;
		break;
	default:
		return -1;
	}

	print("imsic test driver: imsic test driver: write 0x%lx to 0x%lx\n", msi_data, msi_addr_pa);
	writel(msi_addr, msi_data);

	return 0;
}

static const struct driver_ops imsic_test_ops = {
	.ioctl = imsic_test_ioctl,
};

int imsic_test_init(struct device *dev, void *data)
{
	int hwirq;
	struct driver *drv;

	hwirq = msi_get_hwirq_affinity(dev, 3, imsic_test_write_msi_msg, 0);
	if (hwirq == -1) {
		print("imsic test driver: imsic test driver: msi_get_hwirq failed\n", __FUNCTION__);
		return -1;
	}
	register_device_irq(dev->irq_domain, hwirq, imsic_test_irq_handler0,
			    NULL);
	register_device_irq(dev->irq_domain, hwirq + 1, imsic_test_irq_handler1,
			    NULL);
	register_device_irq(dev->irq_domain, hwirq + 2, imsic_test_irq_handler2,
			    NULL);

	drv = dev->drv;
	strcpy(dev->name, "IMSIC_TEST");
	strcpy(drv->name, "IMSIC_TEST");
	drv->ops = &imsic_test_ops;

	return 0;
}

DRIVER_REGISTER(imsic_test, imsic_test_init, "imsic,test");
