#ifndef __DMA_H__
#define __DMA_H__

struct dma_ops {
	int (*dma_m2m)(unsigned long src, unsigned long dst, int size);
};

int set_dma_ops(struct dma_ops *ops);
int dma_m2m(void *src, void *dst, int size);

#endif
