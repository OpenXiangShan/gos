#ifndef __SBI_IMSIC_H__
#define __SBI_IMSIC_H__

#include "sbi_trap.h"

#define CSR_MISELECT            0x350
#define CSR_MIREG               0x351

#define CSR_MTOPEI               0x35C

#define TOPEI_ID_SHIFT          16

#define IMSIC_DISABLE_EIDELIVERY		0
#define IMSIC_ENABLE_EIDELIVERY			1
#define IMSIC_DISABLE_EITHRESHOLD		1
#define IMSIC_ENABLE_EITHRESHOLD		0

#define IMSIC_EIDELIVERY                0x70

#define IMSIC_EITHRESHOLD               0x72

#define IMSIC_EIP0                      0x80
#define IMSIC_EIP63                     0xbf
#define IMSIC_EIPx_BITS                 32

#define IMSIC_EIE0                      0xc0
#define IMSIC_EIE63                     0xff
#define IMSIC_EIEx_BITS                 32

#define imsic_csr_write(__c, __v)		\
do {						\
	write_csr(CSR_MISELECT, __c);		\
	write_csr(CSR_MIREG, __v);		\
} while (0)

#define imsic_csr_read(__c)			\
({						\
	unsigned long __v;			\
	write_csr(CSR_MISELECT, __c);		\
	__v = read_csr(CSR_MIREG);		\
	__v;					\
})

#define imsic_csr_set(__c, __v)			\
do {						\
	write_csr(CSR_MISELECT, __c);		\
	csr_set(CSR_MIREG, __v);			\
} while (0)

#define imsic_csr_clear(__c, __v)		\
do {						\
	write_csr(CSR_MISELECT, __c);		\
	csr_clear(CSR_MIREG, __v);		\
} while (0)

int sbi_imsic_enable(int id, struct sbi_trap_hw_context *ctx);
int sbi_imsic_disable(int id, struct sbi_trap_hw_context *ctx);
int sbi_imsic_alloc_irqs(int nr_irqs, struct sbi_trap_hw_context *ctx);
unsigned long sbi_imsic_get_mmio(struct sbi_trap_hw_context *ctx);
int sbi_imsic_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);

#endif
