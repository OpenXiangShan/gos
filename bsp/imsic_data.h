#ifndef __IMSIC_DATA_H
#define __IMSIC_DATA_H

struct imsic_priv_data {
	unsigned int guest_index_bits;
	unsigned int hart_index_bits;
	unsigned int group_index_bits;
	int nr_ids;
	int nr_harts;
	int nr_guests;
};
extern struct imsic_priv_data imsic_hw_data;
extern struct imsic_priv_data imsic_hw_data_m;

#endif
