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

#include <print.h>
#include <device.h>
#include <asm/mmio.h>
#include <asm/type.h>
#include <event.h>
#include "../command.h"
#include "virt.h"
#include "cpu.h"
#include "task.h"
#include "string.h"
#include "mm.h"

#define MSI_DEVICE_MMIO_BASE 0x30000
#define MSI_DEVICE_MMIO_LEN  0x1000

#define REG_MSI_ADDR 0x0
#define REG_MSI_DATA 0x4
#define REG_TRIGGER  0x8

struct msi_device {
	struct vcpu_machine_device dev;
	unsigned long msi_addr;
	unsigned int msi_data;
	unsigned long interrupt_file_mmio_gpa;
	unsigned long interrupt_file_mmio_hva;
	unsigned long interrupt_file_mmio_hpa;
};

static struct vcpu *vcpu1, *vcpu2;
static struct msi_device device[2];

static void msi_device_init(struct msi_device *dev,
			    unsigned long base,
			    unsigned int size, struct memory_region_ops *ops)
{
	dev->dev.base = base;
	dev->dev.size = size;
	dev->dev.ops = ops;
	strcpy(dev->dev.compatible, "msi_device");
	dev->dev.data = (void *)-1;
	dev->msi_addr = 0;
	dev->msi_data = 0;
}

static int vcpu_start1(void *data)
{
	struct virt_run_params *params =
	    (struct virt_run_params *)mm_alloc(sizeof(struct virt_run_params));

	strcpy(params->command, "hello");

	vcpu1 = vcpu_create_force();
	vcpu_add_machine_device(vcpu1, &device[0].dev);
	vcpu_run(vcpu1, params);

	mm_free((void *)params, sizeof(struct virt_run_params));

	return 0;
}

static int vcpu_start2(void *data)
{
	struct virt_run_params *params =
	    (struct virt_run_params *)mm_alloc(sizeof(struct virt_run_params));

	strcpy(params->command, "hello");

	vcpu2 = vcpu_create_force();
	vcpu_add_machine_device(vcpu2, &device[1].dev);
	vcpu_run(vcpu2, params);

	mm_free((void *)params, sizeof(struct virt_run_params));

	return 0;
}

static void msi_dev_send(struct msi_device *msi)
{
	unsigned long offset = msi->msi_addr - msi->interrupt_file_mmio_gpa;

	print("%s -- msi_addr:0x%lx msi_data:%d\n",
	      __FUNCTION__,
	      msi->interrupt_file_mmio_hpa + offset, msi->msi_data);
	writel(msi->interrupt_file_mmio_hva + offset, msi->msi_data);
}

static unsigned long msi_dev_mmio_read(struct memory_region *region,
				       unsigned long addr, unsigned int len)
{
	struct vcpu *vcpu;
	unsigned long reg = addr - region->start;
	unsigned long ret = 0;
	struct msi_device *msi;

	vcpu = container_of(region->machine, struct vcpu, machine);
	if (vcpu == vcpu1)
		msi = &device[0];
	else if (vcpu == vcpu2)
		msi = &device[1];
	else
		return NULL;

	if (reg == REG_MSI_ADDR) {
		ret = msi->msi_addr;
	} else if (reg == REG_MSI_DATA) {
		ret = msi->msi_data;
	}

	return ret;
}

static void msi_dev_mmio_write(struct memory_region *region,
			       unsigned long addr, unsigned long val,
			       unsigned int len)
{
	struct vcpu *vcpu;
	unsigned long reg = addr - region->start;
	struct msi_device *msi;

	vcpu = container_of(region->machine, struct vcpu, machine);

	if (vcpu == vcpu1)
		msi = &device[0];
	else if (vcpu == vcpu2)
		msi = &device[1];
	else
		return;

	print("%s -- reg:0x%x val:0x%lx\n", __FUNCTION__, reg, val);
	if (reg == REG_MSI_ADDR) {
		msi->msi_addr = val;
	} else if (reg == REG_MSI_DATA) {
		msi->msi_data = val;
	} else if (reg == REG_TRIGGER) {
		msi_dev_send(msi);
	}
}

struct memory_region_ops msi_dev_ops = {
	.write = msi_dev_mmio_write,
	.read = msi_dev_mmio_read,
};

static int cmd_vs_imsic_multi_test_handler(int argc, char *argv[], void *priv)
{
	msi_device_init(&device[0], MSI_DEVICE_MMIO_BASE, MSI_DEVICE_MMIO_LEN,
			&msi_dev_ops);
	msi_device_init(&device[1], MSI_DEVICE_MMIO_BASE, MSI_DEVICE_MMIO_LEN,
			&msi_dev_ops);

	if (create_task
	    ("vs imsic test vcpu1", vcpu_start1, NULL, 0, NULL, 0, NULL)) {
		int n;
		print("create task on cpu%d failed...\n", 1);
		print("online cpu info:\n");
		for_each_online_cpu(n) {
			print("  cpu%d\n", n);
		}

		return -1;
	}

	if (create_task
	    ("vs imsic test vcpu2", vcpu_start2, NULL, 0, NULL, 0, NULL)) {
		int n;
		print("create task on cpu%d failed...\n", 1);
		print("online cpu info:\n");
		for_each_online_cpu(n) {
			print("  cpu%d\n", n);
		}

		return -1;
	}

	wait_for_ms(1000);

	device[0].interrupt_file_mmio_hva = vcpu1->vs_interrupt_file_va;
	device[0].interrupt_file_mmio_hpa = vcpu1->vs_interrupt_file_pa;
	device[0].interrupt_file_mmio_gpa = vcpu1->vs_interrupt_file_gpa;
	device[1].interrupt_file_mmio_hva = vcpu2->vs_interrupt_file_va;
	device[1].interrupt_file_mmio_hpa = vcpu2->vs_interrupt_file_pa;
	device[1].interrupt_file_mmio_gpa = vcpu2->vs_interrupt_file_gpa;

	msi_dev_send(&device[0]);
	msi_dev_send(&device[1]);

	wait_for_ms(100);

	return 0;
}

static const struct command cmd_vs_imsic_multi_test = {
	.cmd = "vs_imsic_multi_test",
	.handler = cmd_vs_imsic_multi_test_handler,
	.priv = NULL,
};

int command_vs_imsic_multi_test_init()
{
	register_command(&cmd_vs_imsic_multi_test);

	return 0;
}

APP_COMMAND_REGISTER(vs_imsic_multi_test_command,
		     command_vs_imsic_multi_test_init);
