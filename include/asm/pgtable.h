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

#ifndef PGTABLE_H
#define PGTABLE_H

extern int pgtable_l5_enabled;
extern int pgtable_l4_enabled;

extern unsigned long va_pa_offset;

#define PAGE_OFFSET 0xffffffd800000000

#define VMAP_START 0xffffffc800000000
#define VMAP_END 0xffffffd7ffffffff

#define FIXMAP_START 0xffffffc6fea00000
#define FIXMAP_LEN (6 * 1024 * 1024)

#define FIXMAP_DTB_START FIXMAP_START

#define FIXMAP_HW_START FIXMAP_START
#define FIXMAP_HW_LEN   0x10000

#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_ALIGN_2M(addr) (((addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK)
#define PAGE_ALIGN_1G(addr) (((addr) + PAGE_1G_SIZE - 1) & PAGE_1G_MASK)

#define N_PAGE(size)     size % PAGE_SIZE == 0 ? size/PAGE_SIZE : (size/PAGE_SIZE + 1)
#define N_PAGE_EXT(size, page_size)  size % page_size == 0 ? size/page_size : (size/page_size + 1)

#define phy_to_virt(phy) ((unsigned long)(phy) + va_pa_offset)
#define virt_to_phy(virt) ((unsigned long)(virt) - va_pa_offset)

#define PGDIR_SHIFT_L3  30
#define PGDIR_SHIFT_L4  39
#define PGDIR_SHIFT_L5  48

#define PAGE_SHIFT       12
#define PAGE_SIZE        (1UL << PAGE_SHIFT)
#define PAGE_MASK        (~(PAGE_SIZE - 1))

#define PAGE_2M_SHIFT       21
#define PAGE_2M_SIZE        (1UL << PAGE_2M_SHIFT)
#define PAGE_2M_MASK        (~(PAGE_2M_SIZE - 1))

#define PAGE_1G_SHIFT       30
#define PAGE_1G_SIZE        (1UL << PAGE_1G_SHIFT)
#define PAGE_1G_MASK        (~(PAGE_1G_SIZE - 1))

#define PAGE_8K_SHIFT       13
#define PAGE_8K_SIZE        (1UL << PAGE_8K_SHIFT)
#define PAGE_8K_MASK        (~(PAGE_8K_SIZE - 1))

#define PAGE_16K_SHIFT      14
#define PAGE_16K_SIZE       (1UL << PAGE_16K_SHIFT)
#define PAGE_16K_MASK       (~(PAGE_16K_SIZE - 1))

#define PAGE_32K_SHIFT      15
#define PAGE_32K_SIZE       (1UL << PAGE_32K_SHIFT)
#define PAGE_32K_MASK       (~(PAGE_32K_SIZE - 1))

#define PAGE_64K_SHIFT      16
#define PAGE_64K_SIZE       (1UL << PAGE_64K_SHIFT)
#define PAGE_64K_MASK       (~(PAGE_64K_SIZE - 1))

#define _PAGE_PFN_SHIFT 10
#define _PAGE_PFN_MASK  (0x3FFFFFFFFFFC00ULL)

#define PGDIR_SHIFT     (pgtable_l5_enabled ? PGDIR_SHIFT_L5 : \
                (pgtable_l4_enabled ? PGDIR_SHIFT_L4 : PGDIR_SHIFT_L3))

#define SATP_MODE_39    0x8000000000000000UL
#define SATP_MODE_48    0x9000000000000000UL
#define SATP_MODE_57    0xa000000000000000UL
#define SATP_MODE_BARE  (0xfUL << 60)

#define SATP_MODE (pgtable_l5_enabled ? SATP_MODE_57 : \
		(pgtable_l4_enabled ? SATP_MODE_48 : SATP_MODE_39))

#define HGATP_MODE_39    0x8000000000000000UL
#define HGATP_MODE_48    0x9000000000000000UL
#define HGATP_MODE_57    0xa000000000000000UL

#define HGATP_MODE (pgtable_l5_enabled ? HGATP_MODE_57 : \
		(pgtable_l4_enabled ? HGATP_MODE_48 : HGATP_MODE_39))

#define HGATP_VMID_SHIFT 44
#define HGATP_VMID_MASK 0x3FFF
#define HGATP_VMID(vmid) ((((unsigned long)vmid) & HGATP_VMID_MASK) << 44)

typedef struct {
	unsigned long pgprot;
} pgprot_t;

#define pgprot_val(x)   ((x).pgprot)

#define __pgprot(x)     ((pgprot_t) { (x) } )

#define _PAGE_PRESENT   (1UL << 0)
#define _PAGE_READ      (1UL << 1)	/* Readable */
#define _PAGE_WRITE     (1UL << 2)	/* Writable */
#define _PAGE_EXEC      (1UL << 3)	/* Executable */
#define _PAGE_USER      (1UL << 4)	/* User */
#define _PAGE_GLOBAL    (1UL << 5)	/* Global */
#define _PAGE_ACCESSED  (1UL << 6)	/* Set by hardware on any access */
#define _PAGE_DIRTY     (1UL << 7)	/* Set by hardware on any write */
#define _PAGE_SOFT      (1UL << 8)	/* Reserved for software */
#define _PAGE_NAPOT     (1UL << 63)     /* Svnapot */

/* Page protection bits */
#define _PAGE_BASE      (_PAGE_PRESENT | _PAGE_ACCESSED)

/*
 * [62:61] Svpbmt Memory Type definitions:
 *
 *  00 - PMA    Normal Cacheable, No change to implied PMA memory type
 *  01 - NC     Non-cacheable, idempotent, weakly-ordered Main Memory
 *  10 - IO     Non-cacheable, non-idempotent, strongly-ordered I/O memory
 *  11 - Rsvd   Reserved for future standard use
 */
#define _PAGE_SVPBMT_NOCACHE     (1UL << 61)
#define _PAGE_SVPBMT_IO          (1UL << 62)
#define _PAGE_SVPBMT_MTMASK      (_PAGE_SVPBMT_NOCACHE | _PAGE_SVPBMT_IO)

#define PAGE_READ               __pgprot(_PAGE_BASE | _PAGE_READ)
#define PAGE_WRITE              __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE)
#define PAGE_EXEC               __pgprot(_PAGE_BASE | _PAGE_EXEC)
#define PAGE_READ_EXEC          __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_EXEC)
#define PAGE_WRITE_EXEC         __pgprot(_PAGE_BASE | _PAGE_READ |      \
                                         _PAGE_EXEC | _PAGE_WRITE)

#define pte_is_valid(pte)   (((unsigned long)pte) & _PAGE_PRESENT)

#define pfn_to_phys(pfn)    (pfn << PAGE_SHIFT)

/*
 * when all of R/W/X are zero, the PTE is a pointer to the next level
 * of the page table; otherwise, it is a leaf PTE.
 */
#define _PAGE_LEAF (_PAGE_READ | _PAGE_WRITE | _PAGE_EXEC)

enum napot_order {
	NAPOT_8K = 1,
	NAPOT_16K,
	NAPOT_32K,
	NAPOT_64K,
	NAPOT_MAX
};
#define for_each_napot_order(order) \
	for (order = NAPOT_8K; order < NAPOT_MAX; order++)

static inline unsigned long pfn_pte(unsigned long pfn, pgprot_t prot)
{
	unsigned long prot_val = pgprot_val(prot);

	return ((pfn << _PAGE_PFN_SHIFT) | prot_val);
}

static inline unsigned long pte_pfn(unsigned long pte)
{
	return ((pte & _PAGE_PFN_MASK) >> _PAGE_PFN_SHIFT);
}

static inline int pmd_present(unsigned long pmd)
{
	return (pmd & (_PAGE_PRESENT | _PAGE_GLOBAL));
}

static inline int pmd_leaf(unsigned long pmd)
{
	return (pmd_present(pmd)
		&& (pmd & (_PAGE_READ | _PAGE_WRITE | _PAGE_EXEC)));
}

static inline int pmd_none(unsigned long pmd)
{
	return (pmd == 0);
}

#endif
