#include <dmac.h>
#include <device.h>
#include <print.h>
#include <string.h>

static char dmaengine_name[] = "DMAC0";

int memcpy_hw(char *dst, char *src, unsigned int size)
{
	struct dmac_ioctl_data data;
	unsigned int dma_width = 0;
	unsigned int blockTS = (size >> dma_width) - 1;
	int fd = 0;

	fd = open(dmaengine_name);
	if (fd == -1) {
		print("open %s fail.\n", dmaengine_name);
		return -1;
	}

	memset((char *)&data, 0, sizeof(struct dmac_ioctl_data));

	data.src = src;
	data.dst = dst;
	data.blockTS = blockTS;
	data.src_addr_inc = 0;
	data.des_addr_inc = 0;
	data.src_width = dma_width;
	data.des_width = dma_width;
	data.src_burstsize = 0;
	data.des_burstsize = 0;
	data.burst_len = 7;

	return ioctl(fd, MEM_TO_MEM, &data);
}

int dma_transfer(char *dst, char *src, unsigned int size,
		 unsigned int data_width, unsigned int burst_len)
{
	struct dmac_ioctl_data data;
	unsigned int blockTS = (size >> data_width) - 1;
	int fd = 0;

	fd = open(dmaengine_name);
	if (fd == -1) {
		print("open %s fail.\n", dmaengine_name);
		return -1;
	}

	memset((char *)&data, 0, sizeof(struct dmac_ioctl_data));

	data.src = src;
	data.dst = dst;
	data.blockTS = blockTS;
	data.src_addr_inc = 0;
	data.des_addr_inc = 0;
	data.src_width = data_width;
	data.des_width = data_width;
	data.src_burstsize = 0;
	data.des_burstsize = 0;
	data.burst_len = burst_len;

	return ioctl(fd, MEM_TO_MEM, &data);
}
