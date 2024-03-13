#ifndef _APLIC_H
#define _APLIC_H

#include "irq.h"
#include "../imsic/imsic.h"

#define APLIC_DIRECT_MODE  0
#define APLIC_MSI_MODE     1

#define APLIC_MAX_IDC                   (1U << 14)
#define APLIC_MAX_SOURCE                1024

#define APLIC_DEFAULT_PRIORITY          1

#define APLIC_DOMAINCFG                 0x0000
#define APLIC_DOMAINCFG_RDONLY          0x80000000
#define APLIC_DOMAINCFG_IE              (1U << 8)
#define APLIC_DOMAINCFG_DM              (1U << 2)
#define APLIC_DOMAINCFG_BE              (1U << 0)

#define APLIC_SOURCECFG_BASE            0x0004
#define APLIC_SOURCECFG_D               (1U << 10)
#define APLIC_SOURCECFG_CHILDIDX_MASK   0x000003ff
#define APLIC_SOURCECFG_SM_MASK         0x00000007
#define APLIC_SOURCECFG_SM_INACTIVE     0x0
#define APLIC_SOURCECFG_SM_DETACH       0x1
#define APLIC_SOURCECFG_SM_EDGE_RISE    0x4
#define APLIC_SOURCECFG_SM_EDGE_FALL    0x5
#define APLIC_SOURCECFG_SM_LEVEL_HIGH   0x6
#define APLIC_SOURCECFG_SM_LEVEL_LOW    0x7

#define APLIC_IRQBITS_PER_REG           32

#define APLIC_SETIP_BASE                0x1c00
#define APLIC_SETIPNUM                  0x1cdc

#define APLIC_CLRIP_BASE                0x1d00
#define APLIC_CLRIPNUM                  0x1ddc

#define APLIC_SETIE_BASE                0x1e00
#define APLIC_SETIENUM                  0x1edc

#define APLIC_CLRIE_BASE                0x1f00
#define APLIC_CLRIENUM                  0x1fdc

#define APLIC_SETIPNUM_LE               0x2000
#define APLIC_SETIPNUM_BE               0x2004

#define APLIC_GENMSI                    0x3000

#define APLIC_TARGET_BASE               0x3004
#define APLIC_TARGET_HART_IDX_SHIFT     18
#define APLIC_TARGET_HART_IDX_MASK      0x3fff
#define APLIC_TARGET_GUEST_IDX_SHIFT    12
#define APLIC_TARGET_GUEST_IDX_MASK     0x3f
#define APLIC_TARGET_IPRIO_MASK         0xff
#define APLIC_TARGET_EIID_MASK          0x7ff

struct aplic_priv_data {
	int mode;
	int nr_irqs;
	struct imsic_priv_data *imsic_data;
};

struct aplic {
	struct irq_domain domain;
	char *name;
	unsigned long base;
	int mode;
	int nr_irqs;
	struct irq_domain *parent;
	unsigned int imsic_guest_index_bits;
	unsigned int imsic_hart_index_bits;
	unsigned int imsic_group_index_bits;
};

void aplic_hw_mode_init(struct aplic *p_aplic);
void aplic_irq_mask(int hwirq, void *data);
void aplic_irq_unmask(int hwirq, void *data);

#endif
