#include "pci.h"
#include "list.h"
#include "asm/type.h"
#include "pci_device_driver.h"
#include "device.h"

static LIST_HEAD(pci_devices);

int pci_register_device(struct pci_device *pci_dev)
{
	list_add_tail(&pci_dev->dev.list, &pci_devices);

	return 0;
}

int pci_probe_driver(void)
{
	extern struct pci_driver_init_entry PCI_DRIVER_INIT_TABLE,
		PCI_DRIVER_INIT_TABLE_END;
	struct device *dev;

	list_for_each_entry(dev, &pci_devices, list) {
		struct pci_driver_init_entry *entry;
		struct pci_device *pci_dev;
		struct pci_driver_init_entry *from = &PCI_DRIVER_INIT_TABLE;
		struct pci_driver_init_entry *to = &PCI_DRIVER_INIT_TABLE_END;
		int nr = to - from;

		pci_dev = to_pci_dev(dev);
		for (entry = from; nr; entry++, nr--) {
			if (entry->vendor_id == pci_dev->vendor &&
			    entry->device_id == pci_dev->device)
				entry->init(pci_dev, NULL);
		}
	}

	return 0;
}
