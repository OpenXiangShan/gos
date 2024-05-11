#ifndef __USER_H
#define __USER_H

#include "list.h"
#include "spinlocks.h"
#include "asm/pgtable.h"

#define USER_SPACE_CODE_START 0x1000

#define USER_SPACE_FIXED_MMAP 0x0
#define USER_SPACE_FIXED_MMAP_SIZE (1 * 1024 * 1024 * 1024UL)	//1G
#define USER_SPACE_DYNAMIC_MMAP (USER_SPACE_FIXED_MMAP + USER_SPACE_FIXED_MMAP_SIZE)
#define USER_SPACE_DYNAMIC_MMAP_SIZE (1 * 1024 * 1024 * 1024UL)	//1G
#define USER_SPACE_DYNAMIC_MAP_NR 4096UL

struct user_cpu_context {
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

struct user_mode_cpu_context {
	struct user_cpu_context s_context;
	struct user_cpu_context u_context;
	unsigned long host_scratch;
	unsigned long host_stvec;
};

struct user_memory_region {
	struct list_head list;
	unsigned long start;
	unsigned long end;
};

struct user_run_params {
	char command[64];
	int argc;
	char argv[16][64];
	int busy;
};

struct user {
	struct user_mode_cpu_context cpu_context;
	unsigned long user_code_va;
	unsigned long user_code_pa;
	unsigned long user_code_user_va;
	unsigned long user_share_memory_va;
	unsigned long user_share_memory_pa;
	unsigned long user_share_memory_user_va;
	spinlock_t lock;
	struct list_head memory_region;
	int mapping;
	struct user_run_params *run_params;
	struct user_run_params s_run_params;
};

struct user *user_create(void);
int user_mode_run(struct user *user, struct user_run_params *params);
void user_mode_switch_to(struct user_mode_cpu_context *ctx);
int user_page_mapping(unsigned long phy, unsigned long virt, unsigned int size);
int user_page_mapping_pg(unsigned long phy, unsigned long virt, unsigned int size,
                pgprot_t pgprot);

#endif
