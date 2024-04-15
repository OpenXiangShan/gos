#ifndef PGTABLE_H
#define PGTABLE_H

extern int pgtable_l5_enabled;
extern int pgtable_l4_enabled;

extern unsigned long va_pa_offset;

#define FIXMAP_START 0xffffffc6fea00000
#define FIXMAP_LEN (6 * 1024 * 1024)

#define FIXMAP_DTB_START FIXMAP_START

#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & PAGE_MASK)

#define N_PAGE(size)     size % PAGE_SIZE == 0 ? size/PAGE_SIZE : (size/PAGE_SIZE + 1)

#define phy_to_virt(phy) ((unsigned long)(phy) + va_pa_offset)
#define virt_to_phy(virt) ((unsigned long)(virt) - va_pa_offset)

#define PGDIR_SHIFT_L3  30
#define PGDIR_SHIFT_L4  39
#define PGDIR_SHIFT_L5  48

#define PAGE_SHIFT       12
#define PAGE_SIZE        (1UL << PAGE_SHIFT)
#define PAGE_MASK        (~(PAGE_SIZE - 1))

#define _PAGE_PFN_SHIFT 10
#define _PAGE_PFN_MASK (0xFFFFFC00ULL)

#define PGDIR_SHIFT     (pgtable_l5_enabled ? PGDIR_SHIFT_L5 : \
                (pgtable_l4_enabled ? PGDIR_SHIFT_L4 : PGDIR_SHIFT_L3))

#define SATP_MODE_39    0x8000000000000000UL
#define SATP_MODE_48    0x9000000000000000UL
#define SATP_MODE_57    0xa000000000000000UL

#define SATP_MODE (pgtable_l5_enabled ? SATP_MODE_57 : \
		(pgtable_l4_enabled ? SATP_MODE_48 : SATP_MODE_39))

#define HGATP_MODE_39    0x8000000000000000UL
#define HGATP_MODE_48    0x9000000000000000UL
#define HGATP_MODE_57    0xa000000000000000UL

#define HGATP_MODE (pgtable_l5_enabled ? HGATP_MODE_57 : \
		(pgtable_l4_enabled ? HGATP_MODE_48 : HGATP_MODE_39))

typedef struct {
	unsigned long pgprot;
} pgprot_t;

#define pgprot_val(x)   ((x).pgprot)

#define __pgprot(x)     ((pgprot_t) { (x) } )

#define _PAGE_PRESENT   (1 << 0)
#define _PAGE_READ      (1 << 1)	/* Readable */
#define _PAGE_WRITE     (1 << 2)	/* Writable */
#define _PAGE_EXEC      (1 << 3)	/* Executable */
#define _PAGE_USER      (1 << 4)	/* User */
#define _PAGE_GLOBAL    (1 << 5)	/* Global */
#define _PAGE_ACCESSED  (1 << 6)	/* Set by hardware on any access */
#define _PAGE_DIRTY     (1 << 7)	/* Set by hardware on any write */
#define _PAGE_SOFT      (1 << 8)	/* Reserved for software */

/* Page protection bits */
#define _PAGE_BASE      (_PAGE_PRESENT | _PAGE_ACCESSED)

#define PAGE_READ               __pgprot(_PAGE_BASE | _PAGE_READ)
#define PAGE_WRITE              __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE)
#define PAGE_EXEC               __pgprot(_PAGE_BASE | _PAGE_EXEC)
#define PAGE_READ_EXEC          __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_EXEC)
#define PAGE_WRITE_EXEC         __pgprot(_PAGE_BASE | _PAGE_READ |      \
                                         _PAGE_EXEC | _PAGE_WRITE)

#define pfn_to_phys(pfn)    (pfn << PAGE_SHIFT)

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
