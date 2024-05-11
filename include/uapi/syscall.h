#ifndef _UAPI_SYSCALL_H
#define _UAPI_SYSCALL_H

#define __NR_syscalls 64

unsigned long syscall(int syscall_nr, ...);

#define __NR_print 0
#define __NR_mmap 1
#define __NR_unmap 2
#define __NR_mmap_pg 3

#endif
