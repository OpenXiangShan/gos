#include "asm/type.h"
#include "asm/sbi.h"
#include "task.h"
#include "cpu.h"
#include "print.h"
#include "consumer_producer.h"
#include "consumer_producer_dummy_case.h"

#define BUFFER_SIZE 4096

static int dummy_consumer_producer_check_data(struct data_pool *p)
{
	int i = 0;
	char data;

	for (i = 0; i < BUFFER_SIZE; i++) {
		data = get_data_by_pos_u8(p, i);
		if (data != (char)i) {
			print("An error has occurred!! data:%d expected:%d\n", data, (char)i);
			return -1;
		}
	}

	return 0;
}

static int dummy_consumer_thread(void *data)
{
	struct data_pool *p = (struct data_pool *)data;
	int nn = 0;

	print("%s in cpu%d\n", __FUNCTION__, sbi_get_cpu_id());

	while (1) {
		consumer_producer_cond_wait(&p->producer_complete);
		if (!dummy_consumer_producer_check_data(p))
			print("%s %dtimes cpu:%d check pass!!\n", __FUNCTION__, nn++, sbi_get_cpu_id());
		else {
			print("%s %dtimes cpu:%d check fail!!\n", __FUNCTION__, nn++, sbi_get_cpu_id());
			return -1;
		}
		consumer_producer_cond_signal(&p->consumer_complete);
	}

	return 0;
}

static int dummy_producer_thread(void *data)
{
	int i = 0;
	struct data_pool *p = (struct data_pool *)data;
	int nn = 0;

	print("%s in cpu%d\n", __FUNCTION__, sbi_get_cpu_id());

	while (1) {
		consumer_producer_cond_wait(&p->consumer_complete);
		print("%s %dtimes cpu:%d\n", __FUNCTION__, nn++, sbi_get_cpu_id());
		for (i = 0; i < BUFFER_SIZE; i++)
			set_data_by_pos_u8(p, i, (char)i);
		consumer_producer_cond_signal(&p->producer_complete);
	}

	return 0;
}

static void create_dummy_consumer_producer_thread(int cpu, char *name, int (*thread)(void *data), void *data)
{
	create_task(name, thread, data, cpu, NULL, 0, NULL);

	if (sbi_get_cpu_id() != cpu)
		cpu_remote_kick(cpu);
}

void consumer_producer_dummy_case_handler(void)
{
	struct data_pool *share_data;

	share_data = consumer_producer_share_memory_init(0, BUFFER_SIZE);
	if (!share_data) {
		print("share memory pool init fail\n");
		return;
	}

	create_dummy_consumer_producer_thread(0, "dummy_consumer", dummy_consumer_thread, (void *)share_data);
	create_dummy_consumer_producer_thread(1, "dummy_producer", dummy_producer_thread, (void *)share_data);

	Sleep();

	consumer_producer_share_memory_deinit(share_data);
}
