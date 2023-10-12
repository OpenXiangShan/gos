#ifndef _MMIO_H
#define _MMIO_H

#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_putl(a,v)		(*(volatile unsigned int *)(a) = (v))

#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_putb(a,v)		(*(volatile unsigned char *)(a) = (v))

#define __arch_getq(a)			(*(volatile unsigned long *)(a))
#define __arch_putq(a,v)		(*(volatile unsigned long *)(a) = (v))

#define dmb()		__asm__ __volatile__ ("" : : : "memory")
#define __iormb()	dmb()
#define __iowmb()	dmb()

#define readl(a)	({ unsigned int  __v = __arch_getl((unsigned int)a); __iormb(); __v; })
#define writel(a,v)	({ unsigned int  __v = v; __iowmb(); __arch_putl((unsigned int)a, __v);})

#define readb(a)	({ unsigned char  __v = __arch_getb((unsigned int)a); __iormb(); __v; })
#define writeb(a,v)	({ unsigned char  __v = v; __iowmb(); __arch_putb((unsigned int)a,__v);})

#define readq(a)	({ unsigned long  __v = __arch_getq((unsigned int)a); __iormb(); __v; })
#define writeq(a,v)	({ unsigned long  __v = v; __iowmb(); __arch_putq((unsigned int)a, __v);})

#endif
