#ifndef __ALIGN_H__
#define __ALIGN_H__

#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(p, a) ALIGN_MASK(p, (typeof(p))(a) - 1)
#define PTR_ALIGN(p, a) (typeof(p))ALIGN((unsigned long)p, a)

#endif
