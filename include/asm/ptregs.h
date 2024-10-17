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

#ifndef __PTREGS_H__
#define __PTREGS_H__

struct pt_regs {
	unsigned long sepc;
	unsigned long ra;
	unsigned long sp;
	unsigned long gp;
	unsigned long tp;
	unsigned long t0;
	unsigned long t1;
	unsigned long t2;
	unsigned long s0;
	unsigned long s1;
	unsigned long a0;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
	unsigned long a4;
	unsigned long a5;
	unsigned long a6;
	unsigned long a7;
	unsigned long s2;
	unsigned long s3;
	unsigned long s4;
	unsigned long s5;
	unsigned long s6;
	unsigned long s7;
	unsigned long s8;
	unsigned long s9;
	unsigned long s10;
	unsigned long s11;
	unsigned long t3;
	unsigned long t4;
	unsigned long t5;
	unsigned long t6;
	/* Supervisor CSRs */
	unsigned long sstatus;
	unsigned long sbadaddr;
	unsigned long scause;
	unsigned long hstatus;
	unsigned long satp;
	unsigned long f0;
	unsigned long f1;
	unsigned long f2;
	unsigned long f3;
	unsigned long f4;
	unsigned long f5;
	unsigned long f6;
	unsigned long f7;
	unsigned long f8;
	unsigned long f9;
	unsigned long f10;
	unsigned long f11;
	unsigned long f12;
	unsigned long f13;
	unsigned long f14;
	unsigned long f15;
	unsigned long f16;
	unsigned long f17;
	unsigned long f18;
	unsigned long f19;
	unsigned long f20;
	unsigned long f21;
	unsigned long f22;
	unsigned long f23;
	unsigned long f24;
	unsigned long f25;
	unsigned long f26;
	unsigned long f27;
	unsigned long f28;
	unsigned long f29;
	unsigned long f30;
	unsigned long f31;
	unsigned long vtype;
	unsigned long vlenb;
	unsigned long vl;
	unsigned long vstart;
	unsigned long vxstate;
	unsigned long vxrm;
	unsigned long reserve;
	unsigned long v0[2];
	unsigned long v1[2];
	unsigned long v2[2];
	unsigned long v3[2];
	unsigned long v4[2];
	unsigned long v5[2];
	unsigned long v6[2];
	unsigned long v7[2];
	unsigned long v8[2];
	unsigned long v9[2];
	unsigned long v10[2];
	unsigned long v11[2];
	unsigned long v12[2];
	unsigned long v13[2];
	unsigned long v14[2];
	unsigned long v15[2];
	unsigned long v16[2];
	unsigned long v17[2];
	unsigned long v18[2];
	unsigned long v19[2];
	unsigned long v20[2];
	unsigned long v21[2];
	unsigned long v22[2];
	unsigned long v23[2];
	unsigned long v24[2];
	unsigned long v25[2];
	unsigned long v26[2];
	unsigned long v27[2];
	unsigned long v28[2];
	unsigned long v29[2];
	unsigned long v30[2];
	unsigned long v31[2];
};

#endif /*__PTREGS_H__*/
