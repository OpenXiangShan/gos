#ifndef __CACHE_FLUSH_H__
#define __CACHE_FLUSH_H__

void cbo_cache_flush(unsigned long base);
void cbo_cache_inval(unsigned long base);
void cbo_cache_clean(unsigned long base);
void cbo_cache_zero(unsigned long base);
void prefetch_i(unsigned long base);
void prefetch_w(unsigned long base);
void prefetch_r(unsigned long base);

#endif
