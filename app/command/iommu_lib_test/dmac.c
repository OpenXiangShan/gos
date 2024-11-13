#include "dmac.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "print.h"

void dmac_ch1_single_transfer(u32 type, u32 src_hs, u32 des_hs, u64 src_addr,
			      u64 des_addr, u32 blockTS, u32 src_addr_inc,
			      u32 des_addr_inc, u32 src_width, u32 des_width,
			      u32 src_burstsize, u32 des_burstsize)
{
	int size = (blockTS + 1) << src_width;
	struct dmac_device *dmac;

	print("dma start -- src:0x%lx dst:0x%lx size:%d\n", src_addr, des_addr,
	      size);
	dmac = get_dmac("DMAC0");
	if (!dmac) {
		print("get dmac fail\n");
		return;
	}

	dmac->ops->transfer_m2m(src_addr, des_addr, size, dmac->priv);
}

void dmac_ch2_single_transfer(u32 type, u32 src_hs, u32 des_hs, u32 src_addr,
			      u32 des_addr, u32 blockTS, u32 src_addr_inc,
			      u32 des_addr_inc, u32 src_width, u32 des_width,
			      u32 src_burstsize, u32 des_burstsize)
{
}

void dmac_init()
{
}

void dmac_test_data_init(char *data)
{
	char *tmp = (char *)data;
	char nn = 0;
	for (int i = 0; i < 4096; i++) {
		tmp[i] = nn++;
	}
}

void dmac_wait_for_complete(void)
{
}

int dmac_check_data(char *src, char *dst, int size)
{
	for (int i = 0; i < 16; i++)
		print("src[%d]:%d -- dst[%d]:%d\n", i, src[i], i, dst[i]);

	while (size--)
		if (*src++ != *dst++)
			return 0;

	return 1;
}
