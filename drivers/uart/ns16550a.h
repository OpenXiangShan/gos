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

#ifndef NS16550A_H
#define NS16550A_H

#define SYNC_ADDR 0xbc

#define RBR       0x00
#define THR       0x00
#define DLL       0x00
#define DLH       0x04
#define IER       0x04
#define IIR       0x08
#define FCR       0x08
#define LCR       0x0c
#define MCR       0x10
#define LSR       0x14
#define MSR       0x18
#define SCR_UART  0x1c
#define LPDLL     0x20
#define LPDLH     0x24
#define USR       0x7c
#define HTX       0xa4
#define DMASA     0xa8
#define UCV       0xf8
#define CTR       0xfc

#define REG_STATUS_RX 0x01
#define REG_STATUS_TX 0x20
#define LSR_ERROR      0x80
#define LSR_EMPTY      0x40

#define RFE  0x80
#define TEMT 0x40
#define THRE 0x20
#define BI   0x10
#define FE   0X08
#define PE   0X04
#define OE   0X02
#define DR   0X01
#define DTR  0X01
#define RTS  0X02
#define AFCE 0X20
#define SIRE 0X40
#define CTS  0X10

unsigned long ns16550a_get_base(void);

#endif
