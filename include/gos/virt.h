#ifndef __VIRT_H
#define __VIRT_H

#include "asm/ptregs.h"
#include "../virt/machine.h"
#include "spinlocks.h"
#include "gos.h"

enum {
	VCPU_REQ_FENCE_GVMA_ALL,
	VCPU_REQ_FENCE_GVMA_GPA,
	VCPU_REQ_FENCE_GVMA_VMID,
	VCPU_REQ_FENCE_GVMA_VMID_GPA,
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

struct virt_floating{
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

struct cpu_context {
	struct virt_cpu_context host_context;
	struct virt_cpu_context guest_context;
	struct virt_floating h_floating;
	struct virt_floating g_floating;
	unsigned long host_scratch;
	unsigned long host_stvec;
	unsigned long vsscratch;
	unsigned long vsstatus;
	unsigned long vstvec;
	unsigned long vsepc;
	unsigned long vstval;
	unsigned long hvip;
	unsigned long vsie;
	unsigned long vsip;
	unsigned long vsatp;
};

struct virt_run_params {
	char command[64];
	int argc;
	char argv[16][64];
	int busy;
	int ready;
	int vmid;
	int cpu;
};

struct vcpu_timer {
	void (*timer_handler)(void *data);
	void (*next_event)(unsigned long next, void *data);
	void *data;
};

struct vcpu_gpa {
	unsigned long gpa;
	struct list_head v_list;
};

struct vcpu {
	struct list_head list;
	int last_cpu;
	int cpu;
	int vmid;
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

	unsigned long host_memory_test_va1;
	unsigned long host_memory_test_pa1;
	unsigned long guest_memory_test_pa1;

	/* vcpu run command params */
	struct virt_run_params *run_params;
	struct virt_run_params host_run_params;
	/* vcpu timer */
	struct vcpu_timer timer;
	unsigned long time_delta;
	/* vcpu state */
	unsigned long request;
	unsigned long irq_pending;
#if CONFIG_VIRT_ENABLE_AIA
	/* aia */
	unsigned long hgei;
	unsigned long vs_interrupt_file_va;
	unsigned long vs_interrupt_file_pa;
	unsigned long vs_interrupt_file_gpa;
	unsigned int vs_interrupt_file_size;
#endif
	struct vcpu_gpa *v_gpa;
	int running;
};

static LIST_HEAD(vgpa_list);

struct vcpu_machine_device {
	unsigned long base;
	unsigned int size;
	char compatible[128];
	void *data;
	struct memory_region_ops *ops;
};

static inline void vcpu_clear_interrupt(struct vcpu *vcpu, int irq)
{
	vcpu->irq_pending &= ~(1UL << irq);
}

static inline void vcpu_set_interrupt(struct vcpu *vcpu, int irq)
{
	vcpu->irq_pending |= (1UL << irq);
}

static inline int vcpu_check_irq_pending(unsigned long pending, int irq)
{
	return ((pending >> irq) & (1UL));
}

static inline int vcpu_check_request(unsigned long req, unsigned int flag)
{
	return ((req >> flag) & (1UL));
}

void vcpu_set_request(struct vcpu *vcpu, unsigned int req);
struct vcpu *vcpu_create(void);
struct vcpu *vcpu_create_force(void);
int vcpu_run(struct vcpu *vcpu, struct virt_run_params *params);
void vcpu_switch_to(struct cpu_context *cpu_ctx);
void __vcpu_switch_return(void);
int gstage_page_mapping(unsigned long *pgdp, unsigned long hpa,
			unsigned long gpa, unsigned int size);
int gstage_page_mapping_2M(unsigned long *pgdp, unsigned long hpa,
			   unsigned long gpa, unsigned int size);
int gstage_page_mapping_1G(unsigned long *pgdp, unsigned long hpa,
			   unsigned long gpa, unsigned int size);
void dump_vcpu_info_on_all_cpu(void);
void dump_vcpu_info_on_cpu(int cpu);
struct vcpu *get_vcpu(int vmid, int cpu);
int vcpu_add_machine_device(struct vcpu *vcpu,
			    struct vcpu_machine_device *device);
void vcpu_enable_hgei(int hgei);
void vcpu_disable_hgei(int hgei);
void vcpu_init(void);

#endif
