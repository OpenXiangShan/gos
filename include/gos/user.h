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

#ifndef __USER_H
#define __USER_H

#include "list.h"
#include "spinlocks.h"
#include "asm/pgtable.h"
#include "gos.h"
#include "asm/type.h"

#define USER_SPACE_CODE_START 0x1000

#define USER_SPACE_START 0x0
#define USER_SPACE_FIXED_MMAP 0x0
#define USER_SPACE_FIXED_MMAP_SIZE (1 * 1024 * 1024 * 1024UL)	//1G
#define USER_SPACE_DYNAMIC_MMAP (USER_SPACE_FIXED_MMAP + USER_SPACE_FIXED_MMAP_SIZE)
#define USER_SPACE_DYNAMIC_MMAP_SIZE (1 * 1024 * 1024 * 1024UL)	//1G
#define USER_SPACE_TOTAL_SIZE (4 * 1024 * 1024 * 1024UL) //4G

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

struct user_floating {
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

struct user_mode_cpu_context {
	struct user_cpu_context s_context;
	struct user_cpu_context u_context;
	struct user_floating s_floating;
	struct user_floating u_floating;
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
	int userid;
	int cpu;
	int bg;
};

struct user {
	struct list_head list;
	int user_id;
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
	void *pgdp;
};

#if CONFIG_USER
struct user *user_create(void);
struct user *user_create_force(void);
int user_mode_run(struct user *user, struct user_run_params *params);
void user_mode_switch_to(struct user_mode_cpu_context *ctx);
int user_page_mapping(unsigned long phy, unsigned long virt, unsigned int size);
int user_page_mapping_pg(unsigned long phy, unsigned long virt, unsigned int size,
                pgprot_t pgprot);
void user_init(void);
void dump_user_info_on_all_cpu(void);
void dump_user_info_on_cpu(int cpu);
struct user *get_user(int userid, int cpu);
#else
void user_init(void) {return};
struct user *user_create(void) {return NULL;}
struct user *user_create_force(void) {return NULL;}
int user_mode_run(struct user *user, struct user_run_params *params) {return -1;}
void dump_user_info_on_all_cpu(void) {return;}
void dump_user_info_on_cpu(int cpu) {return;}
struct user *get_user(int userid, int cpu) {return NULL;}
#endif

#endif
