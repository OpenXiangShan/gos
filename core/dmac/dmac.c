/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
