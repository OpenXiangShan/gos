#include "user.h"
#include "mm.h"
#include "print.h"
#include "string.h"
#include "asm/csr.h"
#include "list.h"
#include "spinlocks.h"
#include "irq.h"
#include "asm/type.h"
#include "user_memory.h"
#include "user_exception.h"

#define USER_SPACE_SHARE_MEMORY 0x0
#define USER_SPACE_SHARE_MEMORY_SIZE 0x1000

extern void do_exception_vector(void);
extern char user_bin[];

struct user *p_user = NULL;

struct user *user_create(void)
{
	struct user *user;
	struct user_cpu_context *u_context;

	if (p_user)
		return p_user;

	user = (struct user *)mm_alloc(sizeof(struct user));
	if (!user) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return NULL;
	}
	memset((char *)user, 0, sizeof(struct user));

	u_context = &user->cpu_context.u_context;
	u_context->sstatus = read_csr(CSR_SSTATUS);
	u_context->sstatus &= ~SR_SPP;
	u_context->sstatus |= SR_SPIE;

	INIT_LIST_HEAD(&user->memory_region);
	__SPINLOCK_INIT(&user->lock);

	p_user = user;

	return user;
}

int user_mode_run(struct user *user, char *cmd)
{
	extern unsigned long __user_payload_start;
	extern unsigned long __user_payload_end;
	int user_bin_size =
	    (char *)&__user_payload_end - (char *)&__user_payload_start;
	char *user_bin_ptr = user_bin;
	struct user_cpu_context *u_context = &user->cpu_context.u_context;

	if (user->mapping == 1)
		goto running;

	/* map user code */
	user->user_code_user_va = USER_SPACE_CODE_START;
	if (user->user_code_user_va + user_bin_size >
	    USER_SPACE_FIXED_MMAP + USER_SPACE_FIXED_MMAP_SIZE) {
		print("%s -- code is too large! 0x%lx bytes\n", __FUNCTION__,
		      user_bin_size);
		return -1;
	}
	user->user_code_va = (unsigned long)mm_alloc(user_bin_size);
	if (!user->user_code_va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	user->user_code_pa = virt_to_phy(user->user_code_va);

	print
	    ("user space user mode page mapping -- va: 0x%lx --> pa: 0x%lx, size:0x%x\n",
	     user->user_code_user_va, user->user_code_pa, user_bin_size);
	user_page_mapping(user->user_code_pa, user->user_code_user_va,
			  user_bin_size);

	if (-1 ==
	    add_user_space_memory(user, user->user_code_user_va,
				  user_bin_size)) {
		print("user space memory overlay!! start:0x%lx, len: 0x%x\n",
		      user->user_code_user_va, user_bin_size);
		return -1;
	}

	/* map share memory */
	user->user_share_memory_va =
	    (unsigned long)mm_alloc(USER_SPACE_SHARE_MEMORY_SIZE);
	if (!user->user_share_memory_va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	user->user_share_memory_pa = virt_to_phy(user->user_share_memory_va);
	user->user_share_memory_user_va = USER_SPACE_SHARE_MEMORY;

	user_page_mapping(user->user_share_memory_pa,
			  user->user_share_memory_user_va,
			  USER_SPACE_SHARE_MEMORY_SIZE);

	__asm__ __volatile("sfence.vma":::"memory");

	if (-1 ==
	    add_user_space_memory(user, user->user_share_memory_user_va,
				  USER_SPACE_SHARE_MEMORY_SIZE)) {
		print("user space memory overlay!! start:0x%lx, len: 0x%x\n",
		      user->user_share_memory_user_va,
		      USER_SPACE_SHARE_MEMORY_SIZE);
		return -1;
	}

	memcpy((char *)user->user_code_va, user_bin_ptr, user_bin_size);
	if (cmd)
		strcpy((char *)user->user_share_memory_va, cmd);

	/* Update user mode entry and param */
	u_context->sepc = user->user_code_user_va;
	u_context->a0 = user->user_share_memory_user_va;

	user->mapping = 1;

running:
	while (1) {
		disable_local_irq();
		user_mode_switch_to(&user->cpu_context);
		do_user_exception(user);
		enable_local_irq();
	}

	return 0;
}
