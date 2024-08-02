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

#ifndef _SBI_H
#define _SBI_H

#include <print.h>

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 0x1
#define SBI_CONSOLE_GETCHAR 0x2
#define SBI_SET_MCOUNTEREN 0x3
#define SBI_GET_M_MSI_DATA 0x500
#define SBI_GET_M_MSI_ADDR 0x501
#define SBI_GET_M_MSI_DATA_IPI 0x502
#define SBI_GET_M_MSI_ADDR_IPI 0x503
#define SBI_GET_CPU_CYCLE 0x200
#define SBI_GET_CPU_ID 0x201
#define SBI_GET_CPU_MCOUNTEREN 0x202
#define SBI_HART_START 0x300
#define SBI_EXIT_VM_TEST 0x100

#define SBI_CALL(which, arg0, arg1, arg2) ({                    \
        register unsigned long a0 asm ("a0") = (unsigned long)(arg0);   \
        register unsigned long a1 asm ("a1") = (unsigned long)(arg1);   \
        register unsigned long a2 asm ("a2") = (unsigned long)(arg2);   \
        register unsigned long a7 asm ("a7") = (unsigned long)(which);  \
        asm volatile ("ecall"                                   \
                      : "+r" (a0)                               \
                      : "r" (a1), "r" (a2), "r" (a7)            \
                      : "memory");                              \
        a0;                                                     \
})

#define SBI_CALL_0(which) SBI_CALL(which, 0, 0, 0)
#define SBI_CALL_1(which, arg0) SBI_CALL(which, arg0, 0, 0)
#define SBI_CALL_2(which, arg0, arg1) SBI_CALL(which, arg0, arg1, 0)

static inline void sbi_set_timer(unsigned long stime_value)
{
	SBI_CALL_1(SBI_SET_TIMER, stime_value);
}

static inline void sbi_putchar(char c)
{
	SBI_CALL_1(SBI_CONSOLE_PUTCHAR, c);
}

static inline void sbi_put_string(char *str)
{
	int i;

	for (i = 0; str[i] != '\0'; i++)
		sbi_putchar((char)str[i]);
}

static inline unsigned long sbi_get_cpu_cycles()
{
	return SBI_CALL_0(SBI_GET_CPU_CYCLE);
}

static inline int sbi_get_cpu_id()
{
	return SBI_CALL_0(SBI_GET_CPU_ID);
}

static inline long sbi_get_cpu_mcounteren()
{
	return SBI_CALL_0(SBI_GET_CPU_MCOUNTEREN);
}

static inline void sbi_set_mcounteren(unsigned long m_value)
{
	SBI_CALL_1(SBI_SET_MCOUNTEREN, m_value);
}

static inline int sbi_get_m_msi_data(int nr)
{
	return SBI_CALL_1(SBI_GET_M_MSI_DATA, nr);
}

static inline unsigned long sbi_get_m_msi_addr()
{
	return SBI_CALL_0(SBI_GET_M_MSI_ADDR);
}
#endif
