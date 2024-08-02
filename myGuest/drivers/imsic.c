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

#include "device.h"
#include "vmap.h"
#include "asm/type.h"
#include "asm/csr.h"
#include "string.h"
#include "print.h"
#include "irq.h"
#include "mm.h"

#define MAX_HARTS 16
#define MAX_GUEST_PER_CPU 64	//1Smode + 63VSmode
#define MAX_IDS 2048

#define TOPEI_ID_SHIFT          16

#define U64_MAX (~(0ULL))
#define BITS_PER_LONG 64

#define INT_MAX (~(0U))

#define IMSIC_MMIO_PAGE_SHIFT           12
#define IMSIC_MMIO_PAGE_SZ              (1UL << IMSIC_MMIO_PAGE_SHIFT)
#define IMSIC_MMIO_PAGE_LE              0x00
#define IMSIC_MMIO_PAGE_BE              0x04

#define IMSIC_MIN_ID                    63
#define IMSIC_MAX_ID                    2048

#define IMSIC_EIDELIVERY                0x70

#define IMSIC_EITHRESHOLD               0x72

#define IMSIC_EIP0                      0x80
#define IMSIC_EIP63                     0xbf
#define IMSIC_EIPx_BITS                 32

#define IMSIC_EIE0                      0xc0
#define IMSIC_EIE63                     0xff
#define IMSIC_EIEx_BITS                 32

#define IMSIC_FIRST                     IMSIC_EIDELIVERY
#define IMSIC_LAST                      IMSIC_EIE63

#define IMSIC_MMIO_SETIPNUM_LE          0x00
#define IMSIC_MMIO_SETIPNUM_BE          0x04

#define IMSIC_DISABLE_EIDELIVERY		0
#define IMSIC_ENABLE_EIDELIVERY			1
#define IMSIC_DISABLE_EITHRESHOLD		1
#define IMSIC_ENABLE_EITHRESHOLD		0

#define IMSIC_MMIO_PAGE_SHIFT           12
#define IMSIC_MMIO_PAGE_SZ              (1UL << IMSIC_MMIO_PAGE_SHIFT)
#define IMSIC_MMIO_PAGE_LE              0x00
#define IMSIC_MMIO_PAGE_BE              0x04

#define IMSIC_MIN_ID                    63
#define IMSIC_MAX_ID                    2048

#define IMSIC_EIDELIVERY                0x70

#define IMSIC_EITHRESHOLD               0x72

#define IMSIC_EIP0                      0x80
#define IMSIC_EIP63                     0xbf
#define IMSIC_EIPx_BITS                 32

#define IMSIC_EIE0                      0xc0
#define IMSIC_EIE63                     0xff
#define IMSIC_EIEx_BITS                 32

#define IMSIC_FIRST                     IMSIC_EIDELIVERY
#define IMSIC_LAST                      IMSIC_EIE63

#define IMSIC_MMIO_SETIPNUM_LE          0x00
#define IMSIC_MMIO_SETIPNUM_BE          0x04

#define imsic_csr_write(__c, __v)		\
do {						\
	write_csr(CSR_ISELECT, __c);		\
	write_csr(CSR_IREG, __v);		\
} while (0)

#define imsic_csr_read(__c)			\
({						\
	unsigned long __v;			\
	write_csr(CSR_ISELECT, __c);		\
	__v = read_csr(CSR_IREG);		\
	__v;					\
})

#define imsic_csr_set(__c, __v)			\
do {						\
	write_csr(CSR_ISELECT, __c);		\
	csr_set(CSR_IREG, __v);			\
} while (0)

#define imsic_csr_clear(__c, __v)		\
do {						\
	write_csr(CSR_ISELECT, __c);		\
	csr_clear(CSR_IREG, __v);		\
} while (0)

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

static inline void imsic_id_enable(int id)
{
	imsic_eix_update(id, 1, 0, 1);
}

static inline void imsic_id_disable(int id)
{
	imsic_eix_update(id, 1, 0, 0);
}

struct imsic {
	unsigned long base;
	/*
	 * MSI Target Address Scheme
	 *
	 * XLEN-1                                                12     0
	 * |                                                     |     |
	 * -------------------------------------------------------------
	 * |xxxxxx|Group Index|xxxxxxxxxxx|HART Index|Guest Index|  0  |
	 * -------------------------------------------------------------
	 */
	unsigned long guest_index_bits;
	unsigned long hart_index_bits;
	unsigned long group_index_bits;
	int nr_ids;
	int nr_harts;
	int nr_guests;
	unsigned long interrupt_file_base[MAX_HARTS][MAX_GUEST_PER_CPU];
	unsigned long ids_enable_bits[MAX_IDS / (sizeof(unsigned long) * 8)];
	unsigned long ids_used_bits[MAX_IDS / (sizeof(unsigned long) * 8)];
	unsigned int ids_target_cpu[MAX_IDS];
};

static struct imsic imsic;

static int imsic_do_irq_handler(void)
{
	int hwirq;

	while ((hwirq = csr_swap(CSR_TOPEI, 0))) {
		hwirq = hwirq >> TOPEI_ID_SHIFT;
		myGuest_print("%s -- hwirq:%d\n", __FUNCTION__, hwirq);
	}

	return 0;
}

static int imsic_id_get_target(struct imsic *p_imsic, int id)
{
	return p_imsic->ids_target_cpu[id];
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

static int imsic_alloc_ids_fix(struct imsic *p_imsic, int id)
{
	unsigned long ids;
	int per_ids = sizeof(unsigned long) * 8;

	ids = p_imsic->ids_used_bits[id / per_ids];
	if (((ids >> (id % per_ids)) & 0x1UL) == 0) {
		ids |= (1 << (id % per_ids));
		p_imsic->ids_used_bits[id / per_ids] = ids;
		return id;
	}

	return -1;
}

static int imsic_alloc_ids(int nr_irqs, struct imsic *p_imsic)
{
	int index = 0, nr = 0;
	unsigned long ids;
	int per_ids = sizeof(unsigned long) * 8;

	while (index < p_imsic->nr_ids) {
		ids = p_imsic->ids_used_bits[index / per_ids];
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
	p_imsic->ids_used_bits[index / per_ids] = ids;

	return (index - nr_irqs + 1);
}

static int imsic_id_set_target(struct imsic *p_imsic, int id, int cpu)
{
	if (id > MAX_IDS)
		return -1;

	p_imsic->ids_target_cpu[id] = cpu;

	return 0;
}

static int imsic_alloc_irqs(int nr_irqs)
{
	struct imsic *p_imsic = (struct imsic *)&imsic;
	int id = -1, i;

	id = imsic_alloc_ids(nr_irqs, p_imsic);
	if (id == -1)
		return -1;

	for (i = 0; i < nr_irqs; i++) {
		// set target cpus of all cpus to cpu0 default
		imsic_id_set_target(p_imsic, id + i, 0);
	}

	return id;
}

static int imsic_compose_msi_msg(int hwirq, unsigned long *msi_addr,
				 unsigned long *msi_data)
{
	struct imsic *p_imsic = (struct imsic *)&imsic;
	unsigned int cpu;

	cpu = imsic_id_get_target(p_imsic, hwirq);
	if (cpu == ~0U)
		return -1;

	*msi_addr = p_imsic->interrupt_file_base[cpu][0];
	*msi_data = hwirq;

	return 0;
}

static int imsic_disable_id(struct imsic *p_imsic, int id)
{
	unsigned long ids;
	unsigned long *ids_enable_bits = p_imsic->ids_enable_bits;
	int bits_per_ids = sizeof(unsigned long) * 8;

	ids = ids_enable_bits[id / bits_per_ids];
	ids &= ~(1 << (id % bits_per_ids));
	ids_enable_bits[id / bits_per_ids] = ids;

	imsic_id_disable(id);

	return 0;
}

static int imsic_enable_id(struct imsic *p_imsic, int id)
{
	unsigned long ids;
	unsigned long *ids_enable_bits = p_imsic->ids_enable_bits;
	int bits_per_ids = sizeof(unsigned long) * 8;

	ids = ids_enable_bits[id / bits_per_ids];
	ids |= (1 << (id % bits_per_ids));
	ids_enable_bits[id / bits_per_ids] = ids;

	imsic_id_enable(id);

	return 0;
}

static int imsic_mask_irq(int hwirq)
{
	struct imsic *p_imsic = (struct imsic *)&imsic;

	return imsic_disable_id(p_imsic, hwirq);
}

static int imsic_unmask_irq(int hwirq)
{
	struct imsic *p_imsic = (struct imsic *)&imsic;

	return imsic_enable_id(p_imsic, hwirq);
}

struct msi_irq_ops imsic_ops = {
	.alloc_irqs = imsic_alloc_irqs,
	.compose_msi_msg = imsic_compose_msi_msg,
	.mask_irq = imsic_mask_irq,
	.unmask_irq = imsic_unmask_irq,
};

int imsic_init(unsigned long base, unsigned int len, void *data)
{
	int hart_id, guest_id;
	int i;
	struct imsic *p_imsic = &imsic;

	memset((char *)p_imsic, 0, sizeof(struct imsic));
	p_imsic->base = (unsigned long)ioremap((void *)base, len, NULL);

	p_imsic->guest_index_bits = 0;
	p_imsic->hart_index_bits = 3;
	p_imsic->group_index_bits = 0;
	p_imsic->nr_ids = 255;
	p_imsic->nr_harts = 1;
	p_imsic->nr_guests = 1;

	if (1ULL << (p_imsic->guest_index_bits - 12) < p_imsic->nr_guests) {
		print("%s -- invalid guest param\n", __FUNCTION__);
		return -1;
	}

	if (1ULL << (p_imsic->hart_index_bits - p_imsic->guest_index_bits - 12)
	    < p_imsic->nr_harts) {
		print("%s -- invalid hart param\n", __FUNCTION__);
		return -1;
	}

	for (hart_id = 0; hart_id < p_imsic->nr_harts; hart_id++) {
		for (guest_id = 0; guest_id < p_imsic->nr_guests; guest_id++) {
			unsigned long per_hart_base;
			unsigned long interrupt_file_base;

			per_hart_base =
			    (unsigned long)walk_pt_va_to_pa(p_imsic->base) +
			    (1ULL << (p_imsic->guest_index_bits + 12)) *
			    hart_id;
			interrupt_file_base =
			    per_hart_base + (1ULL << 12) * guest_id;
			p_imsic->interrupt_file_base[hart_id][guest_id] =
			    interrupt_file_base;
			print("%s -- interrupt file(hart_%d, guest_%d): 0x%x\n",
			      __FUNCTION__, hart_id, guest_id,
			      interrupt_file_base);
		}
	}

	memset((char *)p_imsic->ids_enable_bits, 0,
	       sizeof(p_imsic->ids_enable_bits) /
	       sizeof(p_imsic->ids_enable_bits[0]) * sizeof(unsigned long));
	memset((char *)p_imsic->ids_used_bits, 0,
	       sizeof(p_imsic->ids_used_bits) /
	       sizeof(p_imsic->ids_used_bits[0]) * sizeof(unsigned long));

	for (i = 0;
	     i <
	     sizeof(p_imsic->ids_target_cpu) /
	     sizeof(p_imsic->ids_target_cpu[0]); i++) {
		p_imsic->ids_target_cpu[i] = ~0U;
	}

	imsic_alloc_ids_fix(p_imsic, 0);	//interrupt identity 0 is unused

	imsic_ids_local_delivery(1);

	set_msi_irq_ops(&imsic_ops);

	set_irq_handler(imsic_do_irq_handler);

	return 0;
}

DRIVER_REGISTER(riscv_imsic, imsic_init, "imsic");
