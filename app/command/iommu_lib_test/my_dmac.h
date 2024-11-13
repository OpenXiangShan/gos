#ifndef __MY_DMAC_H
#define __MY_DMAC_H

#include "dmac_reg.h"
#include "asm/type.h"

int dmac_check_data(char *src, char *dst, int size);
void dmac_test_data_init(char *data);
void dmac_wait_for_complete(void);
void dmac_init(void);
void dmac_ch2_single_transfer(u32 type, u32 src_hs, u32 des_hs, u32 src_addr,
			      u32 des_addr, u32 blockTS, u32 src_addr_inc,
			      u32 des_addr_inc, u32 src_width, u32 des_width,
			      u32 src_burstsize, u32 des_burstsize);
void dmac_ch1_single_transfer(u32 type, u32 src_hs, u32 des_hs, u64 src_addr,
			      u64 des_addr, u32 blockTS, u32 src_addr_inc,
			      u32 des_addr_inc, u32 src_width, u32 des_width,
			      u32 src_burstsize, u32 des_burstsize);

#endif
