#include "irq.h"
#include "device.h"
#include "string.h"
#include "print.h"
#include "asm/type.h"
#include "asm/mmio.h"
#include "asm/sbi.h"

struct msi_msg {
	unsigned long msi_addr;
	unsigned long msi_data;
};

static struct msi_msg msi_msg[3];
static int base_hwirq;

static void imsic_test_irq_handler0(void *data)
{
	print("###### enter %s (cpu%d)\n", __FUNCTION__, sbi_get_cpu_id());
}

static void imsic_test_irq_handler1(void *data)
{
	print("###### enter %s (cpu%d)\n", __FUNCTION__, sbi_get_cpu_id());
}

static void imsic_test_irq_handler2(void *data)
{
	print("###### enter %s (cpu%d)\n", __FUNCTION__, sbi_get_cpu_id());
}

static void imsic_test_write_msi_msg(unsigned long msi_addr,
				     unsigned long msi_data, int hwirq,
				     void *priv)
{
	static int ii = 0;

	print
	    ("##### %s %d base_hwirq:%d hwirq:%d msi_addr:0x%x msi_data:0x%x\n",
	     __FUNCTION__, __LINE__, base_hwirq, hwirq, msi_addr, msi_data);

	if (ii++ == 0)
		base_hwirq = hwirq;

	msi_msg[hwirq - base_hwirq].msi_addr = msi_addr;
	msi_msg[hwirq - base_hwirq].msi_data = msi_data;
}

static int imsic_test_ioctl(unsigned int cmd, void *arg)
{
	unsigned long msi_addr, msi_data;

	switch (cmd) {
	case 0:
		msi_addr = msi_msg[0].msi_addr;
		msi_data = msi_msg[0].msi_data;
		break;
	case 1:
		msi_addr = msi_msg[1].msi_addr;
		msi_data = msi_msg[1].msi_data;
		break;
	case 2:
		msi_addr = msi_msg[2].msi_addr;
		msi_data = msi_msg[2].msi_data;
		break;
	default:
		return -1;
	}

	writel(msi_addr, msi_data);

	return 0;
}

static const struct driver_ops imsic_test_ops = {
	.ioctl = imsic_test_ioctl,
};

int imsic_test_init(struct device *dev, void *data)
{
	int hwirq;
	struct driver *drv;

	hwirq = msi_get_hwirq_affinity(dev, 3, imsic_test_write_msi_msg, 3);
	if (hwirq == -1) {
		print("%s -- msi_get_hwirq failed\n", __FUNCTION__);
		return -1;
	}
	register_device_irq(dev->irq_domain, hwirq, imsic_test_irq_handler0,
			    NULL);
	register_device_irq(dev->irq_domain, hwirq + 1, imsic_test_irq_handler1,
			    NULL);
	register_device_irq(dev->irq_domain, hwirq + 2, imsic_test_irq_handler2,
			    NULL);

	drv = dev->drv;
	strcpy(dev->name, "IMSIC_TEST");
	strcpy(drv->name, "IMSIC_TEST");
	drv->ops = &imsic_test_ops;

	return 0;
}

DRIVER_REGISTER(imsic_test, imsic_test_init, "imsic,test");
