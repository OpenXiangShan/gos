#ifndef __IPI_H__
#define __IPI_H__

#include "ipi.h"
#include "list.h"
#include "spinlocks.h"

enum {
	IPI_DO_NOTHING = 0,
	IPI_DO_YIELD,
};

struct ipi_msg {
	struct list_head list;
	int id;
	void *data;
};

struct ipi_msg_ctrl {
	struct list_head ipi_msg;
	spinlock_t lock;
};

struct ipi_ops {
	int (*send_ipi)(int cpu, int id, void *data);
};

int process_ipi(int cpu);
void register_ipi(struct ipi_ops *ops);
int send_ipi(int cpu, int id, void *data);
void ipi_percpu_init(void);

#endif
