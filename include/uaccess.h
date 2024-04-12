#ifndef __UACCESS_H
#define __UACCESS_H

#include "asm/csr.h"
#include "string.h"

static inline int copy_from_user(void *user_ptr, void *addr, unsigned int size)
{
	write_csr(CSR_SSTATUS, SR_SUM);

	memcpy((char *)addr, (char *)user_ptr, size);

	csr_clear(CSR_SSTATUS, SR_SUM);

	return 0;
}

static inline int copy_to_user(void *user_ptr, void *addr, unsigned int size)
{
	write_csr(CSR_SSTATUS, SR_SUM);

	memcpy((char *)user_ptr, (char *)addr, size);

	csr_clear(CSR_SSTATUS, SR_SUM);

	return 0;
}

#endif
