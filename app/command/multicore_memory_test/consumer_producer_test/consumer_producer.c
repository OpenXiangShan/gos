#include "asm/type.h"
#include "mm.h"
#include "spinlocks.h"
#include "consumer_producer.h"

static void consumer_producer_cond_init(struct consumer_producer_cond_t *cond, int flag)
{
	cond->flag = flag;
	__SPINLOCK_INIT(&cond->lock);
}

static void consumer_producer_mutex_init(struct consumer_producer_mutex_t *mutex)
{
	__SPINLOCK_INIT(&mutex->lock);
}

void consumer_producer_cond_wait(struct consumer_producer_cond_t *cond)
{
	while (1) {
		spin_lock(&cond->lock);
		if (cond->flag == 1) {
			cond->flag = 0;
			spin_unlock(&cond->lock);
			break;
		}
		spin_unlock(&cond->lock);
	}
}

void consumer_producer_cond_signal(struct consumer_producer_cond_t *cond)
{
	spin_lock(&cond->lock);
	cond->flag = 1;
	spin_unlock(&cond->lock);
}

struct data_pool *consumer_producer_share_memory_init(unsigned long addr, int size)
{
	struct data_pool *share_data;

	share_data = (struct data_pool *)mm_alloc(sizeof(struct data_pool));
	if (!share_data)
		return NULL;

	if (addr == 0)
		share_data->buffer = mm_alloc(size);
	else
		share_data->buffer = mm_alloc_fix(addr, size);
	if (!share_data->buffer)
		return NULL;

	share_data->size = size;
	if (mmu_is_on) {
		share_data->va = (unsigned long)share_data->buffer;
		share_data->pa = virt_to_phy(share_data->va);
	}
	else
		share_data->pa = (unsigned long)share_data->buffer;

	consumer_producer_cond_init(&share_data->consumer_complete, 1);
	consumer_producer_cond_init(&share_data->producer_complete, 0);
	consumer_producer_mutex_init(&share_data->mutex);

	return share_data;
}

void consumer_producer_share_memory_deinit(struct data_pool *p)
{
	if (!p)
		return;

	mm_free(p->buffer, p->size);
	mm_free((void *)p, sizeof(struct data_pool));
}

unsigned char get_data_by_pos_u8(struct data_pool *p, unsigned long pos)
{
	unsigned char *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned char *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned char) > base + p->size)
		return -1;

	return *addr;
}

unsigned short get_data_by_pos_u16(struct data_pool *p, unsigned long pos)
{
	unsigned short *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned short *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned short) > base + p->size)
		return -1;

	return *addr;
}

unsigned int get_data_by_pos_u32(struct data_pool *p, unsigned long pos)
{
	unsigned int *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned int *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned int) > base + p->size)
		return -1;

	return *addr;
}

unsigned long get_data_by_pos_u64(struct data_pool *p, unsigned long pos)
{
	unsigned long *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned long *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned long) > base + p->size)
		return -1;

	return *addr;
}

int set_data_by_pos_u8(struct data_pool *p, unsigned long pos, unsigned char data)
{
	unsigned char *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned char *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned char) > base + p->size)
		return -1;

	*addr = data;

	return 0;
}

int set_data_by_pos_u16(struct data_pool *p, unsigned long pos, unsigned short data)
{
	unsigned short *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned short *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned short) > base + p->size)
		return -1;

	*addr = data;

	return 0;

}

int set_data_by_pos_u32(struct data_pool *p, unsigned long pos, unsigned int data)
{
	unsigned int *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned int *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned int) > base + p->size)
		return -1;

	*addr = data;

	return 0;
}

int set_data_by_pos_u64(struct data_pool *p, unsigned long pos, unsigned long data)
{
	unsigned long *addr;
	unsigned long base;

	if (mmu_is_on)
		base = p->va;
	else
		base = p->pa;

	addr = (unsigned long *)(base + pos);
	if ((unsigned long)addr + sizeof(unsigned long) > base + p->size)
		return -1;

	*addr = data;

	return 0;
}
