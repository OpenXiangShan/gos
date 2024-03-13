#include <device.h>
#include <asm/type.h>
#include <string.h>
#include <mm.h>
#include <print.h>
#include "irq.h"
#include "dma_mapping.h"

static struct devices _devices;
static struct drivers _drivers;

static struct device *create_device(struct device_init_entry *entry)
{
	struct device *new, *old;
	struct device *dev = NULL;
	int remain;
	struct driver *drv;

retry:
	dev = _devices.p_devices;
	remain = _devices.total;
	while (remain > sizeof(struct device)) {
		if (!dev->in_used) {
			goto found;
		}

		dev++;
		remain -= sizeof(struct device);
	}

	new = mm_alloc(_devices.total + PAGE_SIZE);
	if (!new)
		return NULL;

	memset((char *)new, 0, _devices.total + PAGE_SIZE);
	memcpy((char *)new, (char *)_devices.p_devices, _devices.total);
	old = _devices.p_devices;
	mm_free(old, _devices.total);
	_devices.p_devices = new;
	_devices.total += PAGE_SIZE;

	drv = _drivers.p_drivers;
	remain = _drivers.total;
	while (remain > sizeof(struct driver)) {
		if (!drv->in_used)
			continue;

		if (new > old)
			drv->dev += new - old;
		else
			drv->dev -= old - new;

		drv++;
		remain -= sizeof(struct driver);
	}

	goto retry;

found:
	dev->in_used = 1;
	dev->start = entry->start;
	dev->len = entry->len;
	dev->irqs = entry->irq;
	dev->irq_num = entry->irq_num;
	dev->iommu.dev_id = entry->dev_id;
	dev->irq_domain = find_irq_domain(entry->irq_parent);
	_devices.avail++;

	return dev;
}

static struct driver *create_driver(struct driver_init_entry *entry)
{
	struct driver *new, *old;
	struct driver *drv;
	int remain = _drivers.total;
	int new_size = _drivers.total + PAGE_SIZE;
	struct device *dev;

retry:
	drv = _drivers.p_drivers;
	remain = _drivers.total;
	while (remain > sizeof(struct driver)) {
		if (!drv->in_used) {
			goto found;
		}
		drv++;
		remain -= sizeof(struct driver);
	}

	new = mm_alloc(new_size);
	if (!new)
		return NULL;

	memset((char *)new, 0, new_size);
	memcpy((char *)new, (char *)_drivers.p_drivers, _drivers.total);
	old = _drivers.p_drivers;
	mm_free(old, _drivers.total);
	_drivers.p_drivers = new;
	_drivers.total = new_size;

	dev = _devices.p_devices;
	remain = _devices.total;
	while (remain < sizeof(struct device)) {
		if (!dev->in_used)
			continue;

		if (new > old)
			dev->drv += new - old;
		else
			dev->drv -= old - new;

		dev++;
		remain -= sizeof(struct device);
	}

	goto retry;

found:
	drv->in_used = 1;
	_drivers.avail++;

	return drv;
}

static int __probe_device_table(struct driver_init_entry *driver_head,
				struct driver_init_entry *driver_end,
				struct device_init_entry *hw)
{
	struct driver_init_entry *driver_entry;
	struct device_init_entry *device_entry = hw;
	struct device *dev;
	struct driver *drv;
	int driver_nr = driver_end - driver_head;
	int driver_nr_tmp;

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		driver_nr_tmp = driver_nr;
		dev = create_device(device_entry);
		for (driver_entry = driver_head; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			drv = create_driver(driver_entry);
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {
				strcpy(dev->compatible,
				       device_entry->compatible);
				dev->drv = drv;
				dev->probe = 1;
				drv->dev = dev;
				drv->probe = 1;
				dma_mapping_probe_device(dev);
				driver_entry->init(dev, device_entry->data);
			}
		}
		device_entry++;
	}

	return 0;
}

int device_driver_init(struct device_init_entry *hw)
{
	extern struct driver_init_entry DRIVER_INIT_TABLE,
	    DRIVER_INIT_TABLE_END;
	struct device *p_devices;
	struct driver *p_drivers;

	/* alloc buffer for devices */
	p_devices = (struct device *)mm_alloc(PAGE_SIZE);
	if (!p_devices)
		return -1;
	memset((char *)p_devices, 0, PAGE_SIZE);
	_devices.p_devices = p_devices;
	_devices.total = PAGE_SIZE;
	_devices.avail = 0;

	/* alloc buffer for drivers */
	p_drivers = (struct driver *)mm_alloc(PAGE_SIZE);
	if (!p_drivers)
		return -1;
	memset((char *)p_drivers, 0, PAGE_SIZE);
	_drivers.p_drivers = p_drivers;
	_drivers.total = PAGE_SIZE;
	_drivers.avail = 0;

	/* probe devices and drivers */
	__probe_device_table((struct driver_init_entry *)&DRIVER_INIT_TABLE,
			     (struct driver_init_entry *)&DRIVER_INIT_TABLE_END,
			     hw);

	return 0;
}

int open(char *name)
{
	struct driver *drv;
	struct driver *p_tmp = _drivers.p_drivers;
	int nr = _drivers.avail;

	for_each_driver(drv, p_tmp, nr) {
		if (!drv->probe)
			continue;

		if (!strncmp(name, drv->name, 64))
			return drv - _drivers.p_drivers;
	}

	return -1;
}

int read(int fd, char *buf, unsigned long offset, unsigned int len, int flag)
{
	struct driver *drv = &_drivers.p_drivers[fd];

	if (!drv->ops->read) {
		return NULL;
	}

	return drv->ops->read(buf, offset, len, flag);

}

int write(int fd, char *buf, unsigned long offset, unsigned int len)
{
	struct driver *drv = &_drivers.p_drivers[fd];

	if (!drv->ops->write) {
		return NULL;
	}

	return drv->ops->write(buf, offset, len);
}

int ioctl(int fd, unsigned int cmd, void *arg)
{
	struct driver *drv = &_drivers.p_drivers[fd];

	if (!drv->ops || !drv->ops->ioctl) {
		return NULL;
	}

	return drv->ops->ioctl(cmd, arg);
}

void walk_devices()
{
	struct device *dev;
	int nr = _devices.avail;
	int id = 0, i;

	print("================= walk devices =================\n");
	for_each_device(dev, _devices.p_devices, nr) {
		print("device %d\n", id++);
		print("    name: %s\n", dev->name);
		print("    base address: 0x%x\n", dev->start);
		for (i = 0; i < dev->irq_num; i++)
			print("    irq[i]: %d\n", dev->irqs[i]);
		print("    probe: %d\n", dev->probe);
	}
}
