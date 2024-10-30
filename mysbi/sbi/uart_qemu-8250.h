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

#ifndef _QEMU_8250_UART_H
#define _QEMU_8250_UART_H

#define UART_DAT    0x00
#define UART_IER    0x01
#define UART_IIR    0x02
#define UART_FCR    0x02
#define UART_LCR    0x03
#define UART_MCR    0x04
#define UART_LSR    0x05
#define UART_MSR    0x06

#define UART_DLL    0x00
#define UART_DLM    0x01

#define UART_LSR_ERROR      0x80
#define UART_LSR_EMPTY      0x40
#define UART_LSR_TFE	    0x20
#define UART_LSR_BI	    0x10
#define UART_LSR_FE	    0x08
#define UART_LSR_PE	    0x04
#define UART_LSR_OE	    0x02
#define UART_LSR_DR	    0x01

void uart_qemu_8250_init(unsigned long base, struct sbi_uart_ops *ops, void *data);

#endif
