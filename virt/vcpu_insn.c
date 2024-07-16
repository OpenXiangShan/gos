#include "vcpu_insn.h"
#include "virt.h"
#include "print.h"

static unsigned long vcpu_unpriv_read(struct vcpu *vcpu,
				      int read_insn,
				      unsigned long guest_addr,
				      struct vcpu_trap *trap)
{
	register unsigned long taddr asm("a0") = (unsigned long)trap;
	register unsigned long ttmp asm("a1");
	unsigned long flags, val, tmp, old_hstatus;

	local_irq_save(flags);

	old_hstatus = csr_swap(CSR_HSTATUS, vcpu->cpu_ctx.guest_context.hstatus);

	if (read_insn) {
		/*
		 * HLVX.HU instruction
		 * 0110010 00011 rs1 100 rd 1110011
		 */
		asm volatile ("\n"
			".option push\n"
			".option norvc\n"
			"add %[ttmp], %[taddr], 0\n"
			HLVX_HU(%[val], %[addr])
			"andi %[tmp], %[val], 3\n"
			"addi %[tmp], %[tmp], -3\n"
			"bne %[tmp], zero, 2f\n"
			"addi %[addr], %[addr], 2\n"
			HLVX_HU(%[tmp], %[addr])
			"sll %[tmp], %[tmp], 16\n"
			"add %[val], %[val], %[tmp]\n"
			"2:\n"
			".option pop"
		: [val] "=&r" (val), [tmp] "=&r" (tmp),
		  [taddr] "+&r" (taddr), [ttmp] "+&r" (ttmp),
		  [addr] "+&r" (guest_addr) : : "memory");

		if (trap->scause == EXC_LOAD_PAGE_FAULT)
			trap->scause = EXC_INST_PAGE_FAULT;
	} else {
		/*
		 * HLV.D instruction
		 * 0110110 00000 rs1 100 rd 1110011
		 *
		 * HLV.W instruction
		 * 0110100 00000 rs1 100 rd 1110011
		 */
		asm volatile ("\n"
			".option push\n"
			".option norvc\n"
			"add %[ttmp], %[taddr], 0\n"
			HLV_D(%[val], %[addr])
			".option pop"
		: [val] "=&r" (val),
		  [taddr] "+&r" (taddr), [ttmp] "+&r" (ttmp)
		: [addr] "r" (guest_addr) : "memory");
	}

	write_csr(CSR_HSTATUS, old_hstatus);

	local_irq_restore(flags);

	return val;

}

int vcpu_mmio_load(struct vcpu *vcpu, unsigned long fault_addr,
		   unsigned long htinst)
{
	unsigned long insn;
	int len = 0, shift = 0, insn_len = 0;
	struct memory_region *region;
	unsigned long data;
	struct vcpu_trap trap = { 0 };

	if (htinst & 0x1) {
		insn = htinst | INSN_16BIT_MASK;
		insn_len = (htinst & (1UL << 1)) ? INSN_LEN(insn) : 2;
	} else {
		insn = vcpu_unpriv_read(vcpu, 1, vcpu->cpu_ctx.guest_context.sepc, &trap);
		insn_len = INSN_LEN(insn);
	}

	if ((insn & INSN_MASK_LW) == INSN_MATCH_LW) {
		len = 4;
	} else if ((insn & INSN_MASK_LB) == INSN_MATCH_LB) {
		len = 1;
	} else if ((insn & INSN_MASK_LBU) == INSN_MATCH_LBU) {
		len = 1;
	} else if ((insn & INSN_MASK_LD) == INSN_MATCH_LD) {
		len = 8;
	} else if ((insn & INSN_MASK_LWU) == INSN_MATCH_LWU) {
		len = 4;
	} else if ((insn & INSN_MASK_LH) == INSN_MATCH_LH) {
		len = 2;
	} else if ((insn & INSN_MASK_LHU) == INSN_MATCH_LHU) {
		len = 2;
	} else if ((insn & INSN_MASK_C_LD) == INSN_MATCH_C_LD) {
		len = 8;
		insn = RVC_RS2S(insn) << SH_RD;
	} else if ((insn & INSN_MASK_C_LDSP) == INSN_MATCH_C_LDSP &&
		   ((insn >> SH_RD) & 0x1f)) {
		len = 8;
	} else if ((insn & INSN_MASK_C_LW) == INSN_MATCH_C_LW) {
		len = 4;
		insn = RVC_RS2S(insn) << SH_RD;
	} else if ((insn & INSN_MASK_C_LWSP) == INSN_MATCH_C_LWSP &&
		   ((insn >> SH_RD) & 0x1f)) {
		len = 4;
	} else {
		return -1;
	}

	shift = 8 * (sizeof(unsigned long) - len);

	region = find_memory_region(&vcpu->machine, fault_addr);
	if (!region) {
		print("%s -- cannot find region, fault_addr:0x%lx\n",
		      __FUNCTION__, fault_addr);
		return -1;
	}

	if (region->ops && region->ops->read)
		data = region->ops->read(region, fault_addr, len);

	SET_RD(insn, &vcpu->cpu_ctx.guest_context, data << shift >> shift);

	vcpu->cpu_ctx.guest_context.sepc += insn_len;

	return 0;
}

int vcpu_mmio_store(struct vcpu *vcpu, unsigned long fault_addr,
		    unsigned long htinst)
{
	unsigned long data;
	int len = 0, insn_len = 0;
	unsigned long insn;
	struct memory_region *region;
	struct vcpu_trap trap = { 0 };

	if (htinst & 0x1) {
		insn = htinst | INSN_16BIT_MASK;
		insn_len = (htinst & (1UL << 1)) ? INSN_LEN(insn) : 2;
	} else {
		insn = vcpu_unpriv_read(vcpu, 1, vcpu->cpu_ctx.guest_context.sepc, &trap);
		insn_len = INSN_LEN(insn);
	}

	data = GET_RS2(insn, &vcpu->cpu_ctx.guest_context);

	if ((insn & INSN_MASK_SW) == INSN_MATCH_SW) {
		len = 4;
	} else if ((insn & INSN_MASK_SB) == INSN_MATCH_SB) {
		len = 1;
	} else if ((insn & INSN_MASK_SD) == INSN_MATCH_SD) {
		len = 8;
	} else if ((insn & INSN_MASK_SH) == INSN_MATCH_SH) {
		len = 2;
	} else if ((insn & INSN_MASK_C_SD) == INSN_MATCH_C_SD) {
		len = 8;
		data = GET_RS2S(insn, &vcpu->cpu_ctx.guest_context);
	} else if ((insn & INSN_MASK_C_SDSP) == INSN_MATCH_C_SDSP &&
		   ((insn >> SH_RD) & 0x1f)) {
		len = 8;
		data = GET_RS2C(insn, &vcpu->cpu_ctx.guest_context);
	} else if ((insn & INSN_MASK_C_SW) == INSN_MATCH_C_SW) {
		len = 4;
		data = GET_RS2S(insn, &vcpu->cpu_ctx.guest_context);
	} else if ((insn & INSN_MASK_C_SWSP) == INSN_MATCH_C_SWSP &&
		   ((insn >> SH_RD) & 0x1f)) {
		len = 4;
		data = GET_RS2C(insn, &vcpu->cpu_ctx.guest_context);
	} else {
		return -1;
	}

	region = find_memory_region(&vcpu->machine, fault_addr);
	if (!region) {
		print("%s -- cannot find region, fault_addr:0x%lx\n",
		      __FUNCTION__, fault_addr);
		return -1;
	}

	if (region->ops && region->ops->write)
		region->ops->write(region, fault_addr, data, len);

	vcpu->cpu_ctx.guest_context.sepc += insn_len;

	return 0;
}
