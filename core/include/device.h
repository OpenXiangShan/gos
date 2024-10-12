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

#ifndef DEVICE_H
#define DEVICE_H

#include "list.h"

#define DRIVER_INIT_TABLE __driver_init_table
#define DRIVER_INIT_TABLE_END __driver_init_table_end

#define BLOCK 0
#define NONBLOCK 1

#define MAX_IRQ_NUM 16

struct irq_domain;
struct iommu;

struct resource {
	struct list_head list;
	unsigned long base;
	unsigned int end;
};

struct device {
	struct list_head list;
	int probe;
	char name[64];
	char compatible[128];
	unsigned long base;
	unsigned int len;
	int irqs[64];
	int irq_num;
	struct driver *drv;
	struct irq_domain *irq_domain;
	struct iommu *iommu;
	struct iommu_group *iommu_group;
	struct list_head iommu_group_list;
	int dev_id;
	int is_pci_device;
};

struct driver_ops {
	int (*write)(struct device *dev, char *buf, unsigned long offset, unsigned int len);
	int (*read)(struct device *dev, char *buf, unsigned long offset, unsigned int len,
		    int flag);
	int (*ioctl)(struct device *dev, unsigned int cmd, void *arg);
};

struct driver {
	struct list_head list;
	int index;
	int probe;
	char name[64];
	const struct driver_ops *ops;
	struct device *dev;
};

struct device_init_entry {
	char compatible[128];
	unsigned long start;
	unsigned int len;
	char irq_parent[128];
	int irq[16];
	int irq_num;
	char iommu[128];
	int dev_id;
	void *data;
};

typedef int (*driver_init)(struct device * dev, void *data);

struct driver_init_entry {
	char compatible[128];
	driver_init init;
};

#define DRIVER_REGISTER(name, init_fn, compat)                                \
	static const struct driver_init_entry __attribute__((used))           \
		__drvier_init_##name                                          \
		__attribute__((section(".driver_init_table"))) = {            \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}

#define for_each_device(entry) \
	list_for_each_entry(entry, get_devices(), list)
#define for_each_driver(entry) \
	list_for_each_entry(entry, get_drivers(), list)

#define is_pci_device(dev) (dev->is_pci_device)

struct list_head *get_devices(void);
struct list_head *get_drivers(void);
struct device *get_device(char *name);
int device_driver_init(struct device_init_entry *hw);
int earlycon_driver_init(void);
int regist_device_irq(unsigned long hwirq, void (*handler)(void *data),
		      void *priv);
int open(char *name);
int write(int fd, char *buf, unsigned long offset, unsigned int len);
int read(int fd, char *buf, unsigned long offset, unsigned int len, int flag);
int ioctl(int fd, unsigned int cmd, void *arg);
void walk_devices(void);

unsigned long get_cycles(void);

struct driver *create_driver(struct driver_init_entry *entry);
void add_driver(struct driver *drv);
void device_get_resource(struct device *dev, struct resource *res);

#endif
