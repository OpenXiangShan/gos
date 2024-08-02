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

#ifndef _ASM_RISCV_CSR_H
#define _ASM_RISCV_CSR_H

#define RISCV_XLEN 64

#define INSERT_FIELD(val, which, fieldval) \
	(((val) & ~(which)) | ((fieldval) * ((which) & ~((which)-1))))

#define PRV_U				(0UL)
#define PRV_S				(1UL)
#define PRV_M				(3UL)

#define MSTATUS_MPP_SHIFT	11
#define MSTATUS_MPP	(3UL << MSTATUS_MPP_SHIFT)
#define MSTATUS_MPIE	0x00000080UL

/* Status register flags */
#define SSTATUS_SPP_SHIFT	8
#define SSTATUS_SPP	(1UL << SSTATUS_SPP_SHIFT)

#define SR_SIE  0x2UL		/* Supervisor Interrupt Enable */
#define SR_SPIE 0x20UL		/* Previous Supervisor IE */
#define SR_SPP 0x100UL		/* Previously Supervisor */
#define SR_SUM	0x40000UL	/* Supervisor may access User Memory */
#define SR_FS  0x6000UL		/* Floating-point Status */
#define SR_XS  0x00018000UL	/* Extension Status */

/* Interrupt causes */
#define IRQ_S_SOFT     1
#define IRQ_VS_SOFT    2
#define IRQ_M_SOFT     3
#define IRQ_S_TIMER    5
#define IRQ_VS_TIMER   6
#define IRQ_M_TIMER    7
#define IRQ_S_EXT      9
#define IRQ_VS_EXT     10
#define IRQ_M_EXT      11
#define IRQ_S_GEXT     12
#define IRQ_PMU_OVF    13

/* Interrupt enable */
#define SIE_SSIE 0x2UL		/* IPI */
#define SIE_STIE 0x20UL		/* TIMER */
#define SIE_SEIE 0x200UL	/* EXTERN IRQ */

#define SCAUSE_INT (1UL << 63)
#define is_interrupt_fault(reg) (reg & SCAUSE_INT)

#define SCAUSE_EC (0xf)

//#define SATP_MODE_39 (1UL << 63)
/* symbolic CSR names: */
#define CSR_CYCLE               0xc00
#define CSR_TIME                0xc01
#define CSR_INSTRET             0xc02
#define CSR_HPMCOUNTER3         0xc03
#define CSR_HPMCOUNTER4         0xc04
#define CSR_HPMCOUNTER5         0xc05
#define CSR_HPMCOUNTER6         0xc06
#define CSR_HPMCOUNTER7         0xc07
#define CSR_HPMCOUNTER8         0xc08
#define CSR_HPMCOUNTER9         0xc09
#define CSR_HPMCOUNTER10        0xc0a
#define CSR_HPMCOUNTER11        0xc0b
#define CSR_HPMCOUNTER12        0xc0c
#define CSR_HPMCOUNTER13        0xc0d
#define CSR_HPMCOUNTER14        0xc0e
#define CSR_HPMCOUNTER15        0xc0f
#define CSR_HPMCOUNTER16        0xc10
#define CSR_HPMCOUNTER17        0xc11
#define CSR_HPMCOUNTER18        0xc12
#define CSR_HPMCOUNTER19        0xc13
#define CSR_HPMCOUNTER20        0xc14
#define CSR_HPMCOUNTER21        0xc15
#define CSR_HPMCOUNTER22        0xc16
#define CSR_HPMCOUNTER23        0xc17
#define CSR_HPMCOUNTER24        0xc18
#define CSR_HPMCOUNTER25        0xc19
#define CSR_HPMCOUNTER26        0xc1a
#define CSR_HPMCOUNTER27        0xc1b
#define CSR_HPMCOUNTER28        0xc1c
#define CSR_HPMCOUNTER29        0xc1d
#define CSR_HPMCOUNTER30        0xc1e
#define CSR_HPMCOUNTER31        0xc1f
#define CSR_MCOUNTEREN		0x306

#define CSR_SSCOUNTOVF          0xda0

/* Supervisor Trap Setup */
#define CSR_SSTATUS			0x100
#define CSR_SEDELEG			0x102
#define CSR_SIDELEG			0x103
#define CSR_SIE				0x104
#define CSR_STVEC			0x105
#define CSR_SCOUNTEREN			0x106

/* Supervisor Trap Handling */
#define CSR_SSCRATCH			0x140
#define CSR_SEPC			0x141
#define CSR_SCAUSE			0x142
#define CSR_STVAL			0x143
#define CSR_SIP				0x144

/* Supervisor Protection and Translation */
#define CSR_SATP			0x180
#define SATP_ASID_SHIFT                 44
#define SATP_ASID_MASK			0xffff

/* AIA csr */
/* Virtual Interrupts and Interrupt Priorities (H-extension with AIA) */
#define CSR_HVIEN		0x608
#define CSR_HVICTL		0x609
#define CSR_HVIPRIO1		0x646
#define CSR_HVIPRIO2		0x647
/* Supervisor-Level Window to Indirectly Accessed Registers (AIA) */
#define CSR_SISELECT            0x150
#define CSR_SIREG               0x151
/* Supervisor-Level Interrupts (AIA) */
#define CSR_STOPEI              0x15c
#define CSR_STOPI               0xdb0
/* Supervisor-Level High-Half CSRs (AIA) */
#define CSR_SIEH                0x114
#define CSR_SIPH                0x154

#define CSR_IEH        CSR_SIEH
#define CSR_ISELECT    CSR_SISELECT
#define CSR_IREG       CSR_SIREG
#define CSR_IPH        CSR_SIPH
#define CSR_TOPEI      CSR_STOPEI
#define CSR_TOPI       CSR_STOPI

/* VS csr */
#define CSR_VSSTATUS 0x200
#define CSR_VSIE 0x204
#define CSR_VSTVEC 0x205
#define CSR_VSSCRATCH 0x240
#define CSR_VSEPC 0x241
#define CSR_VSCAUSE 0x242
#define CSR_VSTVAL 0x243
#define CSR_VSIP 0x244
#define CSR_VSATP 0x280

/* Exception causes */
#define EXC_INST_MISALIGNED	0
#define EXC_INST_ACCESS		1
#define EXC_INST_ILLEGAL	2
#define EXC_BREAKPOINT		3
#define EXC_LOAD_MISALIGNED	4
#define EXC_LOAD_ACCESS		5
#define EXC_STORE_MISALIGNED	6
#define EXC_STORE_ACCESS	7
#define EXC_SYSCALL		8
#define EXC_HYPERVISOR_SYSCALL	9
#define EXC_SUPERVISOR_SYSCALL	10
#define EXC_INST_PAGE_FAULT	12
#define EXC_LOAD_PAGE_FAULT	13
#define EXC_STORE_PAGE_FAULT	15
#define EXC_INST_GUEST_PAGE_FAULT	20
#define EXC_LOAD_GUEST_PAGE_FAULT	21
#define EXC_VIRTUAL_INST_FAULT		22
#define EXC_STORE_GUEST_PAGE_FAULT	23

/* HS csr */
#define CSR_HSTATUS 0x600

#define HSTATUS_VSTR_SHIFT 22
#define HSTATUS_VSTR     (1UL << HSTATUS_VSTR_SHIFT)

#define HSTATUS_VTW_SHIFT 21
#define HSTATUS_VTW     (1UL << HSTATUS_VTW_SHIFT)

#define HSTATUS_VTVM_SHIFT 22
#define HSTATUS_VTVM     (1UL << HSTATUS_VTVM_SHIFT)

#define HSTATUS_VEGIN_SHIFT 12

#define HSTATUS_SPVP_SHIFT	8
#define HSTATUS_SPVP	(1UL << HSTATUS_SPVP_SHIFT)

#define HSTATUS_SPV_SHIFT	7
#define HSTATUS_SPV	(1UL << HSTATUS_SPV_SHIFT)

#define HSTATUS_GVA_SHIFT 6
#define HSTATUS_GVA     (1UL << HSTATUS_GVA_SHIFT)

#define HSTATUS_VGEIN        (0x0003f000UL)
#define HSTATUS_VGEIN_SHIFT  12

#define CSR_HEDELEG 0x602
#define CSR_HIDELEG 0x603
#define CSR_HIE 0x604
#define CSR_HTIMEDELTA 0x605
#define CSR_HTIMEDELTAH 0x615
#define CSR_HCOUNTEREN 0x606
#define CSR_HGEIE 0x607
#define CSR_HTVAL 0x643
#define CSR_HIP 0x644
#define CSR_HVIP 0x645
#define CSR_HTINST 0x64A
#define CSR_HGATP 0x680

#define CSR_HGEIP 0xE07

#define CSR_STIMECMP 0x14D
#define CSR_VSTIMECMP 0x24D

#define HCOUNTEREN_TM_SHIFT  1
#define HCOUNTEREN_TM       (1UL << HCOUNTEREN_TM_SHIFT)

#define HENVCFG_STCE_SHIFT  63
#define HENVCFG_STCE        (1UL << HENVCFG_STCE_SHIFT)

/* Machine Memory Protection */
#define MAX_CSR_PMP     8

#define CSR_PMPCFG0	0x3a0
#define CSR_PMPADDR0	0x3b0
#define CSR_PMPADDR1	0x3b1
#define CSR_PMPADDR2	0x3b2
#define CSR_PMPADDR3	0x3b3
#define CSR_PMPADDR4	0x3b4
#define CSR_PMPADDR5	0x3b5
#define CSR_PMPADDR6	0x3b6
#define CSR_PMPADDR7	0x3b7

#define PMP_R	0x01UL
#define PMP_W	0x02UL
#define PMP_X	0x04UL
#define PMP_A	0x18UL
#define PMP_A_TOR 0x08UL
#define PMP_A_NA4 0x10UL
#define PMP_A_NAPOT 0x18UL
#define PMP_L	 0x80UL
#define PMP_RWX (PMP_R | PMP_W | PMP_X)
#define PMP_SHIFT 2

#define __ASM_STR(x)    #x

#define read_csr(csr)						\
({								\
	register unsigned long __v;				\
	__asm__ __volatile__ ("csrr %0, "  __ASM_STR(csr)			\
			      : "=r" (__v) :			\
			      : "memory");			\
	__v;							\
})

#define write_csr(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrw "__ASM_STR(csr)", %0"		\
			      : : "rK" (__v)			\
			      : "memory");			\
})

#define csr_swap(csr, val)                                      \
({                                                              \
        unsigned long __v = (unsigned long)(val);               \
        __asm__ __volatile__ ("csrrw %0, " __ASM_STR(csr) ", %1"\
                              : "=r" (__v) : "rK" (__v)         \
                              : "memory");                      \
        __v;                                                    \
})

#define csr_set(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrs "__ASM_STR(csr)", %0"		\
			      : : "rK" (__v)			\
			      : "memory");			\
})

#define csr_clear(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrc "__ASM_STR(csr)", %0"		\
			      : : "rK" (__v)			\
			      : "memory");			\
})

#define csr_read_clear(csr, val)				\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrrc %0, " __ASM_STR(csr) ", %1"\
			      : "=r" (__v) : "rK" (__v)		\
			      : "memory");			\
	__v;							\
})

#define read_reg(reg)                             \
({                                                \
	unsigned long _v;                         \
	__asm__ __volatile__ ("mv %0, ra\n\t"     \
				: "=r" (_v) :     \
				: "memory");      \
	_v;                                       \
})

#endif
