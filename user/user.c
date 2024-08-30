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
#include "asm/ptregs.h"
#include "percpu.h"
#include "asm/sbi.h"
#include "asm/pgtable.h"
#include "asm/tlbflush.h"
#include "task.h"

#define USER_SPACE_SHARE_MEMORY 0x0
#define USER_SPACE_SHARE_MEMORY_SIZE 0x1000

extern void do_exception_vector(void);
extern char user_bin[];
static DEFINE_PER_CPU(struct list_head, user_list);
static unsigned long userid_bitmap;

static void user_userid_init(void)
{
	userid_bitmap = 0;
}

static void user_update_run_params(struct user *user)
{
	struct user_run_params *params = user->run_params;
	struct user_run_params *s_params = &user->s_run_params;

	if (!params)
		return;

	if (params->busy == 1)
		return;

	if (s_params->busy == 1) {
		__smp_rmb();
		memcpy((char *)params, (char *)s_params,
		       sizeof(struct user_run_params));
		__smp_wmb();
		params->busy = 1;
		s_params->busy = 0;
	}
}

static int user_set_run_params(struct user *user, struct user_run_params *cmd)
{
	struct user_run_params *params = &user->s_run_params;

	while (params->busy == 1) ;

	__smp_rmb();

	memcpy((char *)params, (char *)cmd, sizeof(struct user_run_params));

	__smp_wmb();

	params->busy = 1;

	return 0;
}

static struct user *__user_create(void)
{
	struct user *user;
	struct user_cpu_context *u_context;
	struct list_head *users;

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

	users = &per_cpu(user_list, sbi_get_cpu_id());
	list_add_tail(&user->list, users);

	return user;
}

static int find_free_userid(unsigned long *userid)
{
	unsigned long bitmap = *userid;
	int pos = 0;

	while (bitmap & 0x01) {
		if (pos == 64)
			return -1;
		bitmap = bitmap >> 1;
		pos++;
	}

	*userid |= (1UL) << pos;

	return pos;
}

static int user_alloc_userid(int cpu)
{

	return find_free_userid(&userid_bitmap);
}

static void user_update_userid(struct user *user)
{
	user->user_id = user_alloc_userid(sbi_get_cpu_id());
}

static void __dump_user_info(int cpu)
{
	struct user *user;
	struct list_head *users;

	users = &per_cpu(user_list, cpu);
	if (!users) {
		print("Invalid hart id: %d\n", cpu);
		return;
	}

	print("+++++++++++++ user info on cpu%d +++++++++++++\n", cpu);
	list_for_each_entry(user, users, list) {
		print("@@@@@@@@@@@@@@ user%d info: @@@@@@@@@@@@@@\n", user->user_id);
		print("- userid : %d\n", user->user_id);
		print("- pgdp : 0x%lx\n", user->pgdp);
		print("- memory info:\n");
		print("    code_va : 0x%lx\n", user->user_code_va);
		print("    code_user_va : 0x%lx\n", user->user_code_user_va);
		print("    code_pa : 0x%lx\n", user->user_code_pa);
		print("    share_mem_va : 0x%lx\n", user->user_share_memory_va);
		print("    share_mem_user_va : 0x%lx\n", user->user_share_memory_user_va);
		print("    share_mem_pa : 0x%lx\n", user->user_share_memory_pa);
		print("\n");
	}
}

struct user *get_user(int userid, int cpu)
{
	struct user *user;
	struct list_head *users;

	users = &per_cpu(user_list, cpu);

	list_for_each_entry(user, users, list) {
		if (user->user_id == userid)
			return user;
	}

	return NULL;
}

void dump_user_info_on_all_cpu(void)
{
	int cpu;

	for_each_online_cpu(cpu)
		__dump_user_info(cpu);
}

void dump_user_info_on_cpu(int cpu)
{
	__dump_user_info(cpu);
}

struct user *user_create_force(void)
{
	return __user_create();
}

struct user *user_create(void)
{
	struct user *user;
	struct list_head *users;

	users = &per_cpu(user_list, sbi_get_cpu_id());

	if (list_empty(users))
		goto create_user;

	user = list_entry(list_first(users), struct user, list);

	return user;

create_user:
	return __user_create();
}

int user_mode_run(struct user *user, struct user_run_params *params)
{
	extern unsigned long __user_payload_start;
	extern unsigned long __user_payload_end;
	int user_bin_size =
	    (char *)&__user_payload_end - (char *)&__user_payload_start;
	char *user_bin_ptr = user_bin;
	struct user_cpu_context *u_context = &user->cpu_context.u_context;
	struct pt_regs *regs;

	if (user->mapping == 1) {
		return user_set_run_params(user, params);
	}

	user_update_userid(user);
	print("userid: %d\n", user->user_id);

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

	local_flush_tlb_all();

	if (-1 ==
	    add_user_space_memory(user, user->user_share_memory_user_va,
				  USER_SPACE_SHARE_MEMORY_SIZE)) {
		print("user space memory overlay!! start:0x%lx, len: 0x%x\n",
		      user->user_share_memory_user_va,
		      USER_SPACE_SHARE_MEMORY_SIZE);
		return -1;
	}

	memcpy((char *)user->user_code_va, user_bin_ptr, user_bin_size);
	if (params) {
		params->userid = user->user_id;
		params->cpu = sbi_get_cpu_id();
		memcpy((char *)user->user_share_memory_va, (void *)params,
		       sizeof(struct user_run_params));
		user->run_params =
		    (struct user_run_params *)user->user_share_memory_va;
		user->run_params->busy = 1;
	}

	/* Update user mode entry and param */
	u_context->sepc = user->user_code_user_va;
	u_context->a0 = user->user_share_memory_user_va;

	user->mapping = 1;

	regs = mm_alloc(sizeof(struct pt_regs));
	if (!regs) {
		print("user space alloc pt_regs failed !\n");
		return -1;
	}
        memset((char *)regs, 0, sizeof(struct pt_regs));

	user->pgdp = get_current_task()->pgdp;

	while (1) {
		user_update_run_params(user);
		disable_local_irq();
		user_mode_switch_to(&user->cpu_context);
		if (do_user_exception(user, regs) == -1) {
			enable_local_irq();
			break;
		}
		enable_local_irq();
	}

	mm_free((void *)regs, sizeof(struct pt_regs));

	return 0;
}

void user_init(void)
{
	int cpu;
	struct list_head *users;

	for_each_online_cpu(cpu) {
		users = &per_cpu(user_list, cpu);
		INIT_LIST_HEAD(users);
	}

	user_userid_init();
}
