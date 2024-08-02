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

#include "uapi/syscall.h"
#include "user.h"
#include "uaccess.h"
#include "print.h"
#include "user_memory.h"
#include "asm/pgtable.h"

unsigned long sys_print(char *user_ptr, unsigned int pos)
{
	char tmp[128];

	if (pos >= 128)
		pos = 128;

	copy_from_user(user_ptr, tmp, pos);

	print("%s", tmp);

	return 0;
}

unsigned long sys_mmap(unsigned int size)
{
	void *ptr;

	ptr = user_space_mmap(size);

	return (unsigned long)ptr;
}

unsigned long sys_mmap_pg(unsigned int size, pgprot_t pgprot)
{
	void *ptr;

	ptr = user_space_mmap_pg(size, pgprot);

	return (unsigned long)ptr;
}

unsigned long sys_unmap(void *addr, unsigned int size)
{
	user_space_unmap(addr, size);

	return 0;
}

static const int sys_ni_syscall(void)
{
	return -1;
}

#undef __SYSCALL
#define __SYSCALL(nr, call)     [nr] = (call),

const void *syscall_table[__NR_syscalls] = {
	[0 ... __NR_syscalls - 1] = sys_ni_syscall,
	__SYSCALL(__NR_print, sys_print)
	    __SYSCALL(__NR_mmap, sys_mmap)
	    __SYSCALL(__NR_mmap_pg, sys_mmap_pg)
	    __SYSCALL(__NR_unmap, sys_unmap)
};
