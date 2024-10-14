#ifndef __PCI_SIMPLE_H__
#define __PCI_SIMPLE_H__

#include "list.h"

#define PCI_ECAM_BUS_SHIFT	20 /* Bus number */
#define PCI_ECAM_DEVFN_SHIFT	12 /* Device and Function number */

#define PCI_ECAM_BUS_MASK	0xff
#define PCI_ECAM_DEVFN_MASK	0xff
#define PCI_ECAM_REG_MASK	0xfff /* Limit offset to a maximum of 4K */

#define PCI_ECAM_BUS(x)	(((x) & PCI_ECAM_BUS_MASK) << PCI_ECAM_BUS_SHIFT)
#define PCI_ECAM_DEVFN(x)	(((x) & PCI_ECAM_DEVFN_MASK) << PCI_ECAM_DEVFN_SHIFT)
#define PCI_ECAM_REG(x)	((x) & PCI_ECAM_REG_MASK)

#define PCI_ECAM_OFFSET(bus, devfn, where) \
	(PCI_ECAM_BUS(bus) | \
	 PCI_ECAM_DEVFN(devfn) | \
	 PCI_ECAM_REG(where))

#define PCI_SLOT(devfn)         (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)         ((devfn) & 0x07)

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND   0x04
#define  PCI_COMMAND_IO      0x1
#define  PCI_COMMAND_MEMORY  0x2
#define  PCI_COMMAND_MASTER  0x4
#define PCI_STATUS 0x06
#define PCI_CLASS_REVISION 0x08
#define PCI_HEAD_TYPE 0x0e
#define PCI_CAPABILITY_START 0x34

#define PCI_BASE_ADDRESS_0  0x10
#define PCI_BASE_ADDRESS_1  0x14
#define PCI_BASE_ADDRESS_2  0x18
#define PCI_BASE_ADDRESS_3  0x1c
#define PCI_BASE_ADDRESS_4  0x20
#define PCI_BASE_ADDRESS_5  0x24

#define  PCI_BASE_ADDRESS_SPACE		0x01	/* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO	0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY	0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK	0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32	0x00	/* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M	0x02	/* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64	0x04	/* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH	0x08	/* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK	(~0x0fUL)
#define  PCI_BASE_ADDRESS_IO_MASK	(~0x03UL)

#define  PCI_CAP_ID_MSIX	0x11	/* MSI-X */

/* MSI-X registers (in MSI-X capability) */
#define  PCI_MSIX_FLAGS		2	/* Message Control */
#define  PCI_MSIX_FLAGS_QSIZE	0x07FF	/* Table size */
#define  PCI_MSIX_FLAGS_MASKALL	0x4000	/* Mask all vectors for this function */
#define  PCI_MSIX_FLAGS_ENABLE	0x8000	/* MSI-X enable */
#define  PCI_MSIX_TABLE		4	/* Table offset */
#define  PCI_MSIX_TABLE_BIR	0x00000007 /* BAR index */
#define  PCI_MSIX_TABLE_OFFSET	0xfffffff8 /* Offset into specified BAR */
#define  PCI_MSIX_PBA		8	/* Pending Bit Array offset */
#define  PCI_MSIX_PBA_BIR	0x00000007 /* BAR index */
#define  PCI_MSIX_PBA_OFFSET	0xfffffff8 /* Offset into specified BAR */
#define  PCI_MSIX_FLAGS_BIRMASK	PCI_MSIX_PBA_BIR /* deprecated */
#define  PCI_CAP_MSIX_SIZEOF	12	/* size of MSIX registers */

#define PCI_MSIX_ENTRY_SIZE		16
#define PCI_MSIX_ENTRY_LOWER_ADDR	0x0  /* Message Address */
#define PCI_MSIX_ENTRY_UPPER_ADDR	0x4  /* Message Upper Address */
#define PCI_MSIX_ENTRY_DATA		0x8  /* Message Data */
#define PCI_MSIX_ENTRY_VECTOR_CTRL	0xc  /* Vector Control */
#define  PCI_MSIX_ENTRY_CTRL_MASKBIT	0x00000001

#define PCI_HEADER_TYPE_NORMAL  0
#define PCI_HEADER_TYPE_BRIDGE  1

#define PCI_DRIVER_INIT_TABLE __pci_driver_init_table
#define PCI_DRIVER_INIT_TABLE_END __pci_driver_init_table_end

enum {
	pci_mem_type_io = 0,
	pci_mem_type_mem,
	pci_mem_type_pref_mem,
};

enum {
	pci_addr_type_mem_32 = 0,
	pci_addr_type_mem_1M,
	pci_addr_type_mem_64,
};

struct bar {
	int mem_type;
	int addr_type;
	unsigned long base;
	unsigned int size;
};

struct pci_device {
	struct list_head list;
	unsigned int devfn;
	unsigned int vendor;
	unsigned int device;
	unsigned int class;
	struct bar bar[6];
	unsigned int msi_cap_pos;
	unsigned int msix_cap_pos;
	unsigned long msix_base;
};

struct resource {
	unsigned long base;
	unsigned int end;
};

struct pci_simple_root_bus {
	struct resource res;
	unsigned long pci_config_base;
	struct list_head devices;
};

typedef int (*pci_driver_init)(struct pci_device *dev, void *data);

struct pci_driver_init_entry {
	int vendor_id;
	int device_id;
	pci_driver_init init;
};

#define PCI_DRIVER_REGISTER(name, init_fn, vid, did)         \
	static const struct pci_driver_init_entry __attribute__((used))  \
		__pci_driver_init_##name                                 \
		__attribute__((section(".pci_driver_init_table"))) = {   \
			.vendor_id = vid,                          \
			.device_id = did,                          \
			.init = init_fn,                                 \
		}

unsigned int pci_simple_read_config_byte(int devfn, int reg);
unsigned int pci_simple_read_config_word(int devfn, int reg);
unsigned int pci_simple_read_config_dword(int devfn, int reg);
int pci_simple_write_config_byte(int devfn, int reg, unsigned int val);
int pci_simple_write_config_word(int devfn, int reg, unsigned int val);
int pci_simple_write_config_dword(int devfn, int reg, unsigned int val);
unsigned int pci_simple_find_capability(int devfn, int cap);
int pci_simple_get_resource(struct pci_device *dev, int bar, struct resource *res);
int pci_simple_probe_root_bus(void);
int pci_simple_register_root_bus(unsigned long ecam_addr, unsigned long mmio, unsigned int size);
int pci_simple_msix_enable(struct pci_device *dev, int *irqs);

#endif
