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

#ifndef SBI_H
#define SBI_H

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 0x1
#define SBI_CONSOLE_GETCHAR 0x2
#define SBI_SET_MCOUNTEREN 0x3
#define SBI_SET_MEDELEG 0x4
#define SBI_GET_MEDELEG 0x5
#define SBI_GET_CPU_CYCLE 0x200
#define SBI_GET_CPU_ID 0x201
#define SBI_GET_CPU_MCOUNTEREN 0x202
#define SBI_HART_START 0x300
#define CSR_MCOUNTEREN 0x306
#define SBI_SET_CSR_MIE 0x400
#define SBI_GET_CSR_MIE 0x401
#define SBI_GET_CSR_MENVCFG 0x402
#define SBI_GET_M_MSI_DATA 0x500
#define SBI_GET_M_MSI_ADDR 0x501
#define SBI_GET_M_MSI_DATA_IPI 0x502
#define SBI_GET_M_MSI_ADDR_IPI 0x503
#define SBI_HPM_TEST 0x600

typedef int (*do_ext_irq_t)(void);

struct sbi_trap_hw_context;
struct sbi_trap_regs;

void sbi_jump_to_next(struct sbi_trap_hw_context *ctx);
void sbi_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);
void sbi_trap_handler(struct sbi_trap_regs *regs);
void sbi_secondary_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);
void sbi_register_ext_irq_handler(do_ext_irq_t fn);

#endif
