#ifndef DMAC_H
#define DMAC_H

enum DMA_TYPE {
	MEM_TO_MEM = 0,
};

struct dmac_ioctl_data {
	void *src;
	void *dst;
	unsigned int blockTS;
	unsigned int src_addr_inc;
	unsigned int des_addr_inc;
	unsigned int src_width;
	unsigned int des_width;
	unsigned int src_burstsize;
	unsigned int des_burstsize;
	unsigned int burst_len;
};

int memcpy_hw(char *dst, char *src, unsigned int size);
int dma_transfer(char *dst, char *src, unsigned int size,
		 unsigned int data_width, unsigned int burst_len);

#endif