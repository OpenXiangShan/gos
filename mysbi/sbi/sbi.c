#include <print.h>
#include <asm/csr.h>
#include <asm/trap.h>
#include <sbi_uart.h>
#include <string.h>
#include <device.h>
#include <sbi.h>
#include "sbi_clint.h"
#include "sbi_uart.h"
#include "sbi_trap.h"

static struct sbi_trap_hw_context *h_context[8] = { 0 };

extern void exception_vector(void);

void sbi_panic()
{
	sbi_print("sbi panic...\n");

	while (1) ;
}

static void sbi_trap_error(struct sbi_trap_regs *regs, const char *msg, int rc)
{
	sbi_print("mhartid:%d:\n", read_csr(mhartid));
	sbi_print("%s: %s (error %d)\n", __FUNCTION__, msg, rc);
	sbi_print("mcause: %x  mtval: %x \n",
		  read_csr(mcause), read_csr(mtval));

	sbi_print("mepc: %x mstatus : %x\n", regs->mepc, regs->mstatus);
	sbi_print(" gp : %x tp : %x t0 : %x\n", regs->gp, regs->tp, regs->t0);
	sbi_print(" t1 : %x t2 : %x t3 : %x\n", regs->t1, regs->t2, regs->s0);
	sbi_print(" s1 : %x a0 : %x a1 : %x\n", regs->s1, regs->a0, regs->a1);
	sbi_print(" a2 : %x a3 : %x a4 : %x\n", regs->a2, regs->a3, regs->a4);
	sbi_print(" a5 : %x a6 : %x a7 : %x\n", regs->a5, regs->a6, regs->a7);
	sbi_print(" s2 : %x s3 : %x s4 : %x\n", regs->s2, regs->s3, regs->s4);
	sbi_print(" s5 : %x s6 : %x s7 : %x\n", regs->s5, regs->s6, regs->s7);
	sbi_print(" s8 : %x s9 : %x s10: %x\n", regs->s8, regs->s9, regs->s10);
	sbi_print(" s11: %x t3 : %x t4: %x\n", regs->s11, regs->t3, regs->t4);
	sbi_print(" t5 : %x t6 : %x sp: %x\n", regs->t5, regs->t6, regs->sp);
	sbi_print(" ra: %x\n", regs->ra);

	sbi_panic();
}

static int sbi_hart_start(unsigned int hartid, unsigned long jump_addr)
{
	struct sbi_trap_hw_context *ctx;

	ctx = h_context[hartid];
	if (!ctx) {
		sbi_print("%s -- invalid hartid, hartid:%d\n", __FUNCTION__,
			  hartid);
		return -1;
	}

	ctx->next_addr = jump_addr;
	strcpy(ctx->next_mode, "S_MODE");
	ctx->wait_var = 1;

	return 0;
}

static int sbi_ecall_handle(unsigned int id, struct sbi_trap_regs *regs)
{
	unsigned long ret_value = 0;
	int ret = 0;
	struct sbi_trap_hw_context *ctx = h_context[read_csr(mhartid)];

	switch (id) {
	case SBI_SET_TIMER:
		clint_timer_event_start(ctx, regs->a0);
		ret = 0;
		break;
	case SBI_CONSOLE_PUTCHAR:
		sbi_uart_putc(regs->a0);
		ret = 0;
		break;
	case SBI_GET_CPU_CYCLE:
		ret_value = read_csr(mcycle);
		break;
	case SBI_GET_CPU_ID:
		ret_value = read_csr(mhartid);
		break;
	case SBI_HART_START:
		sbi_hart_start(regs->a0, regs->a1);
		break;
	}

	regs->a0 = ret_value;

	if (!ret)
		regs->mepc += 4;

	return ret;
}

static void sbi_ext_process()
{
	csr_clear(mie, MIP_MEIP);
	csr_set(mip, MIP_SEIP);
}

static void sbi_soft_process()
{

}

void sbi_trap_handler(struct sbi_trap_regs *regs)
{
	int rc = -1;
	unsigned long mcause = read_csr(mcause);
	unsigned long ecall_id = regs->a7;
	const char *msg = "unsupported mcause";

	//sbi_print("%s -- %s:%d mcause:0x%x\n", __FILE__, __FUNCTION__, __LINE__, mcause);

	if (mcause & MCAUSE_IRQ) {
		mcause &= ~MCAUSE_IRQ;

		switch (mcause) {
		case IRQ_M_TIMER:
			sbi_timer_process();
			break;

		case IRQ_M_EXT:
			sbi_ext_process();
			break;

		case IRQ_M_SOFT:
			sbi_soft_process();
			break;

		default:
			msg = "unhandled external interrupt";
			goto trap_error;
		}

		return;
	}

	switch (mcause) {
	case CAUSE_SUPERVISOR_ECALL:
		rc = sbi_ecall_handle(ecall_id, regs);
		msg = "ecall handler failed";
		break;
	case CAUSE_LOAD_ACCESS:
	case CAUSE_STORE_ACCESS:
		msg = "load store access failed";
		break;
	default:
		break;
	}

trap_error:
	if (rc) {
		sbi_trap_error(regs, msg, rc);
	}
}

static void sbi_trap_init(void)
{
	write_csr(mtvec, exception_vector);
	sbi_print("mtvec=0x%x, 0x%x\n", read_csr(mtvec), exception_vector);

	write_csr(mie, 0);
}

static void sbi_setup_pmp(void)
{
	// Set up a PMP to permit access to all of memory.
	// Ignore the illegal-instruction trap if PMPs aren't supported.
	unsigned long pmpc = PMP_A_NAPOT | PMP_R | PMP_W | PMP_X;
	asm volatile ("la t0, 1f\n\t"
		      "csrrw t0, mtvec, t0\n\t"
		      "csrw pmpaddr0, %1\n\t"
		      "csrw pmpcfg0, %0\n\t"
		      ".align 2\n\t"
		      "1: csrw mtvec, t0"::"r" (pmpc), "r"(-1UL):"t0");
}

static void sbi_set_next(struct sbi_trap_hw_context *ctx)
{
	extern unsigned long __payload_start;
	unsigned long payload_start = (unsigned long)&__payload_start;

#ifndef PAYLOAD_RUN_IN_M_MODE
	ctx->next_addr = payload_start;
	strcpy(ctx->next_mode, "S_MODE");
#else
	ctx->next_addr = payload_start;
	strcpy(ctx->next_mode, "M_MODE");
#endif

	sbi_print("%s -- next_mode: %s, next_addr:0x%x\n", __FUNCTION__,
		  ctx->next_mode, ctx->next_addr);
}

static inline int next_is_s_mode(char *MODE)
{
	return !strncmp(MODE, "S_MODE", 16);
}

static inline int next_is_m_mode(char *MODE)
{
	return !strncmp(MODE, "M_MODE", 16);
}

static void sbi_jump_to_supervisor(unsigned int hart_id,
				   unsigned long hw_info, unsigned long addr)
{
	unsigned long val;

	val = read_csr(mstatus);
	val = INSERT_FIELD(val, MSTATUS_MPP, PRV_S);
	val = INSERT_FIELD(val, MSTATUS_MPIE, 0);
	write_csr(mstatus, val);

	write_csr(mepc, addr);
	write_csr(stvec, addr);
	write_csr(satp, 0);
	//write_csr(sie, 0);

	register unsigned long a0 asm("a0") = hart_id;
	register unsigned long a1 asm("a1") = hw_info;
	asm volatile ("mret"::"r" (a0), "r"(a1));
}

static void sbi_jump_to_machine(unsigned long addr)
{
	typedef void (*fn_t)(void);
	fn_t fn;

	fn = (fn_t) addr;

	sbi_print("%s %d\n", __FUNCTION__, __LINE__);
	fn();
}

void sbi_jump_to_next(struct sbi_trap_hw_context *ctx)
{
	if (next_is_s_mode(ctx->next_mode)) {
		sbi_jump_to_supervisor(ctx->hart_id, ctx->hw_info,
				       ctx->next_addr);
	} else if (next_is_m_mode(ctx->next_mode)) {
		sbi_jump_to_machine(ctx->next_addr);
	} else
		sbi_print("unsupported mode.\n");
}

static void sbi_get_hw_info(struct sbi_trap_hw_context *ctx)
{
	extern unsigned long DEVICE_INIT_TABLE, DEVICE_INIT_TABLE_END;
	int nr =
	    (struct device_init_entry *)&DEVICE_INIT_TABLE_END -
	    (struct device_init_entry *)&DEVICE_INIT_TABLE;
	struct device_init_entry *device_entry;
	int n = 0;

	nr -= 1;
	sbi_print("device_info: 0x%x\n",
		  (struct device_init_entry *)&DEVICE_INIT_TABLE);
	for (device_entry = (struct device_init_entry *)&DEVICE_INIT_TABLE; nr;
	     device_entry++, nr--) {
		sbi_print("device %d\n", n++);
		sbi_print("    compatible: %s\n", device_entry->compatible);
		sbi_print("    base_address: 0x%x, len:0x%x\n",
			  device_entry->start, device_entry->len);
		sbi_print("    hwirq: %d\n", device_entry->irq);
		sbi_print("    priv_data: 0x%x\n", device_entry->data);
	}

	ctx->hw_info = (unsigned long)&DEVICE_INIT_TABLE;
}

void delegate_traps(void)
{
	unsigned long interrupts;
	unsigned long exceptions;

	interrupts = MIP_SSIP | MIP_STIP | MIP_SEIP;
	exceptions =
	    (1UL << CAUSE_MISALIGNED_FETCH) | (1UL << CAUSE_FETCH_PAGE_FAULT) |
	    (1UL << CAUSE_BREAKPOINT) | (1UL << CAUSE_LOAD_PAGE_FAULT) | (1UL <<
									  CAUSE_STORE_PAGE_FAULT)
	    | (1UL << CAUSE_USER_ECALL) | (1UL << CAUSE_LOAD_ACCESS) | (1UL <<
									CAUSE_STORE_ACCESS)
	    | (1UL << CAUSE_ILLEGAL_INSTRUCTION) | (1UL <<
						    CAUSE_VIRTUAL_SUPERVISOR_ECALL)
	    | (1UL << CAUSE_LOAD_GUEST_PAGE_FAULT) | (1UL <<
						      CAUSE_STORE_GUEST_PAGE_FAULT)
	    | (1UL << CAUSE_FETCH_GUEST_PAGE_FAULT);

	write_csr(mideleg, interrupts);
	write_csr(medeleg, exceptions);
}

void sbi_exit(unsigned int hart_id)
{
	sbi_print("sbi exit... hart id:%d\n", hart_id);

	while (1) ;
}

void sbi_secondary_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx)
{
	extern unsigned long DEVICE_INIT_TABLE;

	sbi_print("sbi secondary init... hart_id: %d ctx:0x%x\n", hart_id, ctx);

	h_context[hart_id] = ctx;
	ctx->wait_var = 0;
	ctx->hart_id = hart_id;
	ctx->hw_info = (unsigned long)&DEVICE_INIT_TABLE;

	sbi_trap_init();
	sbi_setup_pmp();

	sbi_clint_init(hart_id, ctx);

	write_csr(mie, MIP_MSIP | MIP_MEIP | MIP_MTIP);
	delegate_traps();
}

void sbi_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx)
{
	h_context[hart_id] = ctx;

	if (-1 == sbi_uart_init(hart_id, ctx))
		return;

	sbi_print("sbi init... hartid: %d, ctx:%x\n", hart_id, ctx);

	sbi_trap_init();
	sbi_setup_pmp();

	if (-1 == sbi_clint_init(hart_id, ctx))
		sbi_print("sbi clint init failed.\n");

	sbi_set_next(ctx);
	sbi_get_hw_info(ctx);

	write_csr(mie, MIP_MSIP | MIP_MEIP | MIP_MTIP);

	delegate_traps();
}
