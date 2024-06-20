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
};

#endif /*__PTREGS_H__*/
