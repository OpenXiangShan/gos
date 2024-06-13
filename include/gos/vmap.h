#ifndef __VMAP_H
#define __VMAP_H

void *ioremap(void *addr, unsigned int size, int gfp);
void iounmap(void *addr, unsigned int size);
void *vmem_map(void *addr, unsigned int size, int gfp);
void *vmap_alloc(unsigned int size);
void vmap_free(void *addr, unsigned int size);
void *vmem_alloc(unsigned int size, int gfp);
void *vmap_alloc_align(unsigned long align, unsigned int size);
void vmem_free(void *addr, unsigned int size);
void *vmem_alloc_lazy(unsigned int size, int gfp);
void *vmem_alloc_huge(unsigned int size, int page_size, int gfp);
void vmem_free_huge(void *addr, unsigned int size, int page_size);

#endif
