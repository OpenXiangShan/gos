#include "machine.h"
#include "virt.h"
#include "print.h"
#include "mm.h"

#ifdef USE_AIA
static void imsic_mmio_write(struct memory_region *region,
			     unsigned long addr, unsigned long val,
			     unsigned int len)
{

}

static unsigned long imsic_mmio_read(struct memory_region *region,
				     unsigned long addr, unsigned int len)
{
	return 0;
}

int imsic_gstage_ioremap(unsigned long *pgdp, unsigned long hpa,
			 unsigned long gpa, unsigned int size)
{
	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY |
		     _PAGE_USER);

	print
	    ("gstage page mapping[imsic] -- gpa:0x%lx -> hpa:0x%lx size:0x%x\n",
	     gpa, hpa, size);

	return mmu_gstage_page_mapping(pgdp, hpa, gpa, size, pgprot);
}

static const struct memory_region_ops imsic_mmio_ops = {
	.write = imsic_mmio_write,
	.read = imsic_mmio_read,
};

int create_imsic_device(struct virt_machine *machine, int id,
			unsigned long base, unsigned int size)
{
	return add_memory_region(machine, id, base, size, &imsic_mmio_ops);
}

int imsic_device_finialize(struct virt_machine *machine, unsigned long gpa,
			   unsigned int size, int id, int pt)
{
	struct vcpu *vcpu;
	unsigned long hpa;

	vcpu = container_of(machine, struct vcpu, machine);
	if (!vcpu)
		return -1;

	hpa = vcpu->vs_interrupt_file_pa;

	if (pt) {
		if (!machine->gstage_pgdp)
			return -1;

		imsic_gstage_ioremap((unsigned long *)machine->gstage_pgdp, hpa,
				     gpa, size);
	}

	return 0;
}
#endif