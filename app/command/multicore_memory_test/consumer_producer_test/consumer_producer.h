#ifndef __CONSUMER_PRODUCER_H__
#define __CONSUMER_PRODUCER_H__

#include "spinlocks.h"

extern int mmu_is_on;

struct consumer_producer_cond_t {
	int flag;
	spinlock_t lock;
};

struct consumer_producer_mutex_t {
	spinlock_t lock;
};

struct data_pool{
	void *buffer;
	int size;
	unsigned long pa;
	unsigned long va;
	struct consumer_producer_cond_t consumer_complete;
	struct consumer_producer_cond_t producer_complete;
	struct consumer_producer_mutex_t mutex;
};

void consumer_producer_cond_wait(struct consumer_producer_cond_t *cond);
void consumer_producer_cond_signal(struct consumer_producer_cond_t *cond);

unsigned char get_data_by_pos_u8(struct data_pool *p, unsigned long pos);
unsigned short get_data_by_pos_u16(struct data_pool *p, unsigned long pos);
unsigned int get_data_by_pos_u32(struct data_pool *p, unsigned long pos);
unsigned long get_data_by_pos_u64(struct data_pool *p, unsigned long pos);
int set_data_by_pos_u8(struct data_pool *p, unsigned long pos, unsigned char data);
int set_data_by_pos_u16(struct data_pool *p, unsigned long pos, unsigned short data);
int set_data_by_pos_u32(struct data_pool *p, unsigned long pos, unsigned int data);
int set_data_by_pos_u64(struct data_pool *p, unsigned long pos, unsigned long data);

struct data_pool *consumer_producer_share_memory_init(unsigned long addr, int size);
void consumer_producer_share_memory_deinit(struct data_pool *p);

#endif
