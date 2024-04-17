#include "user.h"
#include "asm/csr.h"
#include "print.h"
#include "uaccess.h"

extern void *const syscall_table[];

typedef long (*syscall_t)(unsigned long, unsigned long,
			  unsigned long, unsigned long,
			  unsigned long, unsigned long, unsigned long);

static int syscall_handler(struct user_cpu_context *regs)
{
	syscall_t fn;
	unsigned long nr_sys = regs->a7;
	unsigned long orig_a0 = regs->a0;

	fn = syscall_table[nr_sys];
	regs->a0 = fn(orig_a0, regs->a1, regs->a2, regs->a3,
		      regs->a4, regs->a5, regs->a6);

	return 0;
}

int do_user_exception(struct user *user)
{
	struct user_cpu_context *u_context = &user->cpu_context.u_context;
	unsigned long scause;

	scause = read_csr(CSR_SCAUSE);
	print("-----------< scause: 0x%lx \n", scause);

	switch (scause) {
	case EXC_SYSCALL:
		syscall_handler(u_context);
		u_context->sepc += 4;
		break;
	}

	return 0;
}
