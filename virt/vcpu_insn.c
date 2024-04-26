#include "vcpu_insn.h"
#include "virt.h"
#include "print.h"

int vcpu_mmio_load(struct vcpu *vcpu, unsigned long fault_addr,
		   unsigned long htinst)
{
	unsigned long insn;
	int len = 0, shift = 0, insn_len = 0;
	struct memory_region *region;
	unsigned long data;

	if (htinst & 0x1) {
		insn = htinst | INSN_16BIT_MASK;
		insn_len = (htinst & (1UL << 1)) ? INSN_LEN(insn) : 2;
	} else
		return -1;

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
	//int insn_len = 0;
	unsigned long insn;
	struct memory_region *region;

	if (htinst & 0x1) {
		insn = htinst | INSN_16BIT_MASK;
		insn_len = (htinst & (1UL << 1)) ? INSN_LEN(insn) : 2;
	} else
		return -1;

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
