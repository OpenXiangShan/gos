/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_BITOPS_H
#define _ASM_RISCV_BITOPS_H


#if (BITS_PER_LONG == 64)
#define __AMO(op)       "amo" #op ".d"
#elif (BITS_PER_LONG == 32)
#define __AMO(op)       "amo" #op ".w"
#else
#error "Unexpected BITS_PER_LONG"
#endif


#define BIT_MASK(nr)            ((1) << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)        ((1) << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)        ((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE           8


#define __test_and_op_bit_ord(op, mod, nr, addr, ord)           \
({                                                              \
        unsigned long __res, __mask;                            \
        __mask = BIT_MASK(nr);                                  \
        __asm__ __volatile__ (                                  \
                __AMO(op) #ord " %0, %2, %1"                    \
                : "=r" (__res), "+A" (addr[BIT_WORD(nr)])       \
                : "r" (mod(__mask))                             \
                : "memory");                                    \
        ((__res & __mask) != 0);                                \
})

#define __op_bit_ord(op, mod, nr, addr, ord)                    \
        __asm__ __volatile__ (                                  \
                __AMO(op) #ord " zero, %1, %0"                  \
                : "+A" (addr[BIT_WORD(nr)])                     \
                : "r" (mod(BIT_MASK(nr)))                       \
                : "memory");

#define __test_and_op_bit(op, mod, nr, addr)                    \
        __test_and_op_bit_ord(op, mod, nr, addr, .aqrl)
#define __op_bit(op, mod, nr, addr)                             \
        __op_bit_ord(op, mod, nr, addr, )

/* Bitmask modifiers */
#define __NOP(x)        (x)
#define __NOT(x)        (~(x))


/**
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation may be reordered on other architectures than x86.
 */
static inline int test_and_set_bit(int nr, volatile unsigned long *addr)
{
        return __test_and_op_bit(or, __NOP, nr, addr);
}


/**
 * test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation can be reordered on other architectures other than x86.
 */
static inline int test_and_clear_bit(int nr, volatile unsigned long *addr)
{
        return __test_and_op_bit(and, __NOT, nr, addr);
}

/**
 * set_bit - Atomically set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Note: there are no guarantees that this function will not be reordered
 * on non x86 architectures, so if you are writing portable code,
 * make sure not to rely on its reordering guarantees.
 *
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void set_bit(int nr, volatile unsigned long *addr)
{
        __op_bit(or, __NOP, nr, addr);
}

/**
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * Note: there are no guarantees that this function will not be reordered
 * on non x86 architectures, so if you are writing portable code,
 * make sure not to rely on its reordering guarantees.
 */
static inline void clear_bit(int nr, volatile unsigned long *addr)
{
        __op_bit(and, __NOT, nr, addr);
}



#endif /* _ASM_RISCV_BITOPS_H */
