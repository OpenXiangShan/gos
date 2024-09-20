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

#include "vcpu_aia.h"
#include "virt.h"
#include "percpu.h"
#include "asm/csr.h"
#include "mm.h"
#include "../drivers/irqchip/aia/imsic/imsic.h"
#include "asm/type.h"
#include "vmap.h"
#include "imsic_emulator.h"
#include "machine.h"
#include "asm/bitops.h"
#include "irq.h"
#include "asm/trap.h"

static DEFINE_PER_CPU(unsigned long, hgei_bitmap);

static int find_free_hgei(unsigned long *hgei)
{
	unsigned long bitmap = *hgei;
	int pos = 0;

	while (bitmap & 0x01) {
		if (pos == 64)
			return -1;
		bitmap = bitmap >> 1;
		pos++;
	}

	*hgei |= (1UL) << pos;

	return pos;
}

static int vcpu_alloc_hgei(int cpu)
{
	unsigned long *hgei;

	hgei = &per_cpu(hgei_bitmap, cpu);
	if (!hgei)
		return -1;

	return find_free_hgei(hgei);
}

static void hgei_irq_handler(void *data)
{
	print("%s %d\n", __FUNCTION__, __LINE__);
	print("hgeie: 0x%lx\n", read_csr(CSR_HGEIE));
	print("hgeip: 0x%lx\n", read_csr(CSR_HGEIP));
}

void vcpu_enable_hgei(int hgei)
{
	csr_set(CSR_HGEIE, 1UL << hgei);
}

void vcpu_disable_hgei(int hgei)
{
	csr_clear(CSR_HGEIE, 1UL << hgei);
}

int vcpu_interrupt_file_update(struct vcpu *vcpu)
{
	int hgei;
	int cpu = vcpu->cpu;
	unsigned long interrupt_file_base;
	struct virt_cpu_context *guest_ctx = &vcpu->cpu_ctx.guest_context;
	struct irq_domain *intc;

	if (cpu == vcpu->last_cpu)
		return 0;

	hgei = vcpu_alloc_hgei(cpu);
	if (-1 == hgei)
		return -1;

	print("%s %d hgei:%d\n", __FUNCTION__, __LINE__, hgei);
	if (vcpu->hgei != -1)
		vcpu_disable_hgei(vcpu->hgei);
	vcpu_enable_hgei(vcpu->hgei);

	intc = get_intc_domain();
	if (!intc) {
		print("get intc domain failed...\n");
		return -1;
	}
	register_device_irq(NULL, intc, INTERRUPT_CAUSE_GEXTERNAL, hgei_irq_handler, NULL);

	interrupt_file_base = imsic_get_interrupt_file_base(cpu, hgei);
	if (!interrupt_file_base)
		return -1;

	vcpu->vs_interrupt_file_va =
	    (unsigned long)ioremap((void *)interrupt_file_base, PAGE_SIZE,
				   NULL);
	vcpu->vs_interrupt_file_pa = interrupt_file_base;
	vcpu->hgei = hgei;

	guest_ctx->hstatus &= ~HSTATUS_VGEIN;
	guest_ctx->hstatus |= hgei << HSTATUS_VGEIN_SHIFT;

	vcpu->vs_interrupt_file_gpa = get_machine_memmap_base(VIRT_IMSIC);
	vcpu->vs_interrupt_file_size = get_machine_memmap_size(VIRT_IMSIC);

	imsic_gstage_ioremap((unsigned long *)vcpu->machine.gstage_pgdp,
			     vcpu->vs_interrupt_file_pa,
			     get_machine_memmap_base(VIRT_IMSIC),
			     get_machine_memmap_size(VIRT_IMSIC));

	return 0;
}

int vcpu_aia_init(void)
{
	int cpu, nr_hgei;
	unsigned long *hgei;
	unsigned long available_hgei;

	write_csr(CSR_HGEIE, -1UL);
	available_hgei = read_csr(CSR_HGEIE);
	write_csr(CSR_HGEIE, 0);
	nr_hgei = fls64(available_hgei);

	print("%s -- nr_hgei:%d available_hgei:0x%lx\n",
	      __FUNCTION__, nr_hgei, available_hgei);

	for_each_online_cpu(cpu) {
		hgei = &per_cpu(hgei_bitmap, cpu);
		*hgei = ~available_hgei;
	}

	return 0;
}
