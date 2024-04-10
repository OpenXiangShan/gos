#include "machine.h"
#include "print.h"
#include "mm.h"
#include "../drivers/uart/qemu-8250.h"

extern int mmu_is_on;

static void uart_mmio_write(unsigned long addr, unsigned long val,
			    unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);
}

static unsigned long uart_mmio_read(unsigned long addr, unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);

	return 0;
}

int uart_gstage_ioremap(unsigned long *pgdp, unsigned long gpa,
			unsigned int size)
{
	unsigned long addr = qemu_8250_get_base();
	unsigned long hpa;

	if (!mmu_is_on)
		hpa = addr;
	else
		hpa = (unsigned long)walk_pt_va_to_pa((unsigned long)addr);

	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY |
		     _PAGE_USER);

	print("gstage page mapping[uart] -- gpa:0x%lx -> hpa:0x%lx size:0x%x\n",
	      gpa, hpa, size);
	return mmu_gstage_page_mapping(pgdp, hpa, gpa, size, pgprot);
}

static const struct memory_region_ops uart_memory_ops = {
	.write = uart_mmio_write,
	.read = uart_mmio_read,
	.ioremap = uart_gstage_ioremap,
};

int create_uart_device(struct virt_machine *machine, int id, unsigned long base,
		       unsigned int size)
{
	add_memory_region(machine, id, base, size, &uart_memory_ops);

	return 0;
}
