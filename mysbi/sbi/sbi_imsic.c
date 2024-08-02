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

#include <string.h>
#include <print.h>
#include <device.h>
#include <asm/mmio.h>
#include <asm/csr.h>
#include "sbi_trap.h"
#include "sbi_imsic.h"
#include "sbi.h"

#define U64_MAX (~(0ULL))
#define BITS_PER_LONG 64

static void imsic_eix_update(unsigned long base_id,
			     unsigned long num_id, int pend, int val)
{
	unsigned long i, isel, ireg;
	unsigned long id = base_id, last_id = base_id + num_id;

	while (id < last_id) {
		isel = id / BITS_PER_LONG;
		isel *= BITS_PER_LONG / IMSIC_EIPx_BITS;
		isel += (pend) ? IMSIC_EIP0 : IMSIC_EIE0;

		ireg = 0;
		for (i = id & (__riscv_xlen - 1);
		     (id < last_id) && (i < __riscv_xlen); i++) {
			ireg |= 1 << i;
			id++;
		}

		/*
		 * The IMSIC EIEx and EIPx registers are indirectly
		 * accessed via using ISELECT and IREG CSRs so we
		 * need to access these CSRs without getting preempted.
		 *
		 * All existing users of this function call this
		 * function with local IRQs disabled so we don't
		 * need to do anything special here.
		 */
		if (val)
			imsic_csr_set(isel, ireg);
		else
			imsic_csr_clear(isel, ireg);
	}
}

static void imsic_id_enable(int id)
{
	imsic_eix_update(id, 1, 0, 1);
}

static void imsic_id_disable(int id)
{
	imsic_eix_update(id, 1, 0, 0);
}

static void imsic_ids_local_delivery(int enable)
{
	if (enable) {
		imsic_csr_write(IMSIC_EITHRESHOLD, IMSIC_ENABLE_EITHRESHOLD);
		imsic_csr_write(IMSIC_EIDELIVERY, IMSIC_ENABLE_EIDELIVERY);
	} else {
		imsic_csr_write(IMSIC_EIDELIVERY, IMSIC_DISABLE_EIDELIVERY);
		imsic_csr_write(IMSIC_EITHRESHOLD, IMSIC_DISABLE_EITHRESHOLD);
	}
}

static unsigned long get_imsic_base(void)
{
	extern unsigned long DEVICE_INIT_TABLE, DEVICE_INIT_TABLE_END;
	int nr = &DEVICE_INIT_TABLE_END - &DEVICE_INIT_TABLE;
	struct device_init_entry *device_entry;

	for (device_entry = (struct device_init_entry *)&DEVICE_INIT_TABLE;
	     nr; device_entry++, nr--) {
		if (!strncmp(device_entry->compatible, "IMSIC_M", 128)) {
			return device_entry->start;
		}
	}

	return 0;
}

static int sbi_imsic_alloc_ids(int nr_irqs, struct sbi_trap_hw_context *ctx)
{
	int index = 0, nr = 0;
	unsigned long ids;
	int per_ids = sizeof(unsigned long) * 8;

	while (index < ctx->imsic_nr_ids) {
		ids = ctx->imsic_ids_used_bits[index / per_ids];
		if (((ids >> (index % per_ids)) & 0x1UL) == 0) {
			if (++nr == nr_irqs)
				goto alloc_success;
		} else {
			nr = 0;
		}

		index++;
	}

	return -1;

alloc_success:
	ids |= (1 << (index % per_ids));
	ctx->imsic_ids_used_bits[index / per_ids] = ids;

	return (index - nr_irqs + 1);
}

static int sbi_imsic_irq_handler()
{
	int hwirq;

	while ((hwirq = csr_swap(CSR_MTOPEI, 0))) {
		hwirq = hwirq >> TOPEI_ID_SHIFT;
		sbi_print("%s -- hwirq: %d\n", __FUNCTION__, hwirq);
	}

	return 0;
}

int sbi_imsic_enable(int id, struct sbi_trap_hw_context *ctx)
{
	imsic_id_enable(id);

	return 0;
}

int sbi_imsic_disable(int id, struct sbi_trap_hw_context *ctx)
{
	imsic_id_disable(id);

	return 0;
}

int sbi_imsic_alloc_irqs(int nr_irqs, struct sbi_trap_hw_context *ctx)
{
	int id = -1, i;

	id = sbi_imsic_alloc_ids(nr_irqs, ctx);
	if (id == -1) {
		sbi_print("sbi imsic alloc irqs failed...\n");
		return -1;
	}

	for (i = 0; i < nr_irqs; i++) {
		sbi_imsic_enable(id + i, ctx);
	}

	return id;
}

unsigned long sbi_imsic_get_mmio(struct sbi_trap_hw_context *ctx)
{
	return ctx->imsic_base;
}

int sbi_imsic_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx)
{
	unsigned long base;

	base = get_imsic_base();

	ctx->imsic_base = base + (1UL << 12) * hart_id;
	ctx->imsic_nr_ids = 255;
	ctx->imsic_ids_used_bits[0] = 1;

	sbi_print("imsic base: 0x%lx\n", ctx->imsic_base);

	imsic_ids_local_delivery(1);

	sbi_register_ext_irq_handler(sbi_imsic_irq_handler);

	return 0;
}
