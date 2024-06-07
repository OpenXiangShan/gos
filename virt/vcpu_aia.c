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

int vcpu_interrupt_file_upadte(struct vcpu *vcpu)
{
	int hgei;
	int cpu = vcpu->cpu;
	unsigned long interrupt_file_base;
	struct virt_cpu_context *guest_ctx = &vcpu->cpu_ctx.guest_context;

	if (cpu == vcpu->last_cpu)
		return 0;

	hgei = vcpu_alloc_hgei(cpu);
	if (-1 == hgei)
		return -1;

	print("%s %d hgei:%d\n", __FUNCTION__, __LINE__, hgei);
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

	imsic_gstage_ioremap((unsigned long *)vcpu->machine.gstage_pgdp,
			     vcpu->vs_interrupt_file_pa,
			     get_machine_memmap_base(VIRT_IMSIC),
			     get_machine_memmap_size(VIRT_IMSIC));

	return 0;
}

int vcpu_aia_init(struct vcpu *vcpu)
{
	int cpu;
	unsigned long *hgei;

	for_each_online_cpu(cpu) {
		hgei = &per_cpu(hgei_bitmap, cpu);
		*hgei = 1UL;
	}

	return 0;
}
