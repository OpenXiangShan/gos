#ifndef __VIRT_H
#define __VIRT_H

#include "asm/ptregs.h"
#include "../virt/machine.h"

struct vcpu {
	struct pt_regs host_context;
	struct pt_regs guest_context;
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
};

struct vcpu *vcpu_create(void);
int vcpu_run(struct vcpu *vcpu, unsigned long pc);
void vcpu_switch_to(struct pt_regs *host, struct pt_regs *guest);
void __vcpu_switch_return(void);
int gstage_page_mapping(unsigned long *pgdp, unsigned long hpa,
			unsigned long gpa, unsigned int size);

#endif
