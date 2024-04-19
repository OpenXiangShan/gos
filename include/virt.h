#ifndef __VIRT_H
#define __VIRT_H

#include "asm/ptregs.h"
#include "../virt/machine.h"
#include "spinlocks.h"

enum {
	VCPU_REQ_FENCE_GVMA_ALL,
	VCPU_REQ_FENCE_VVMA_ALL,
	VCPU_REQ_UPDATE_HGATP,
};

struct virt_cpu_context {
	unsigned long zero;
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
	unsigned long sepc;
	unsigned long sstatus;
	unsigned long hstatus;
};

struct cpu_context {
	struct virt_cpu_context host_context;
	struct virt_cpu_context guest_context;
	unsigned long host_scratch;
	unsigned long host_stvec;
};

struct virt_run_params {
	char command[64];
	int argc;
	char argv[16][64];
	int busy;
};

struct vcpu {
	struct cpu_context cpu_ctx;
	struct virt_machine machine;
	/* ddr */
	unsigned long host_memory_va;
	unsigned long host_memory_pa;
	unsigned long guest_memory_pa;
	unsigned int memory_size;
	/* sram */
	unsigned long host_sram_va;
	unsigned long host_sram_pa;
	unsigned long guest_sram_pa;
	unsigned int sram_size;
	/* memory test */
	unsigned long host_memory_test_va;
	unsigned long host_memory_test_pa;
	unsigned long guest_memory_test_pa;
	unsigned int memory_test_size;

	struct virt_run_params *run_params;
	struct virt_run_params host_run_params;

	unsigned long request;

	int running;
};

static inline int vcpu_check_request(unsigned long req, unsigned int flag)
{
	return ((req >> flag) & (1UL));
}

void vcpu_set_request(struct vcpu *vcpu, unsigned int req);
struct vcpu *vcpu_create(void);
int vcpu_run(struct vcpu *vcpu, struct virt_run_params *params);
void vcpu_switch_to(struct cpu_context *cpu_ctx);
void __vcpu_switch_return(void);
int gstage_page_mapping(unsigned long *pgdp, unsigned long hpa,
			unsigned long gpa, unsigned int size);

#endif
