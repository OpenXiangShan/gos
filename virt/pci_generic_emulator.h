#ifndef __PCI_GENERIC_EMULATOR_H__
#define __PCI_GENERIC_EMULATOR_H__

union pci_config_addr {
	struct {
		unsigned reg_number    : 12;
		unsigned fun_number    : 3;
		unsigned device_number : 5;
		unsigned bus_number    : 8;
		unsigned reserved      : 4;
	};
	unsigned int addr;
};

struct pci_device_function_emulator;
struct pci_devfn_emu_ops {
	int (*contained_pci_address)(struct pci_device_function_emulator *func, unsigned long addr);
	int (*config_write)(struct pci_device_function_emulator *func, unsigned int reg, unsigned long val, int len);
	unsigned long (*config_read)(struct pci_device_function_emulator *func, unsigned int reg, int len);
};

struct type0_cfg {
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short command;
	unsigned short status;
	unsigned char revision_id;
	unsigned char class_code[3];
	unsigned char cache_line_size;
	unsigned char latency_timer;
	unsigned char header_type;
	unsigned char bist;
	unsigned int bar0;
	unsigned int bar1;
	unsigned int bar2;
	unsigned int bar3;
	unsigned int bar4;
	unsigned int bar5;
	unsigned int cardbus_cis_ptr;
	unsigned short subsys_vendor_id;
	unsigned short subsys_id;
	unsigned int rom_bar;
	unsigned char cap_ptr;
	unsigned char reserved[7];
	unsigned char interrupt_line;
	unsigned char interrupt_pin;
	unsigned char min_gnt;
	unsigned char max_lat;
	unsigned char cap[192];
};

struct type1_cfg{
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short command;
	unsigned short status;
	unsigned char revision_id;
	unsigned char class_code[3];
	unsigned char cache_line_size;
	unsigned char latency_timer;
	unsigned char header_type;
	unsigned char bist;
	unsigned int bar0;
	unsigned int bar1;
	unsigned char primary_bus_num;
	unsigned char secondary_bus_num;
	unsigned char subordinate_bus_num;
	unsigned char secondary_latency_timer;
	unsigned char io_base;
	unsigned char io_limit;
	unsigned short secondary_status;
	unsigned short memory_base;
	unsigned short memory_limit;
	unsigned short prefetch_mem_base;
	unsigned short prefetch_mem_limit;
	unsigned int prefetch_base_upper;
	unsigned int prefetch_limit_upper;
	unsigned short io_base_upper;
	unsigned short io_limit_upper;
	unsigned char cap_ptr;
	unsigned char reserved[3];
	unsigned int expansion_rom_bar;
	unsigned char interrupt_line;
	unsigned char interrupt_pin;
	unsigned short bridge_ctrl;
};

union pci_cfg_space{
	struct type0_cfg type0_cfg;
	struct type1_cfg type1_cfg;
	unsigned char data[256];
};

struct pci_device_function_emulator {
	int function_num;
	unsigned int bar_mask[6];
	unsigned long offset;
	struct pci_devfn_emu_ops *ops;
	struct pt_device_resource *res;
	union pci_cfg_space cfg_space;
	void *data;
};

struct pci_device_emulator {
	int bus_num;
	int device_num;
	int type;
	struct pci_device_function_emulator *function[8];
};

struct pci_generic_emulator {
	unsigned long pci_addr;
	unsigned long cpu_addr;
	unsigned int pci_mmio_size;
	struct pci_device_emulator *slot[32];
};

int create_pci_generic_device(struct virt_machine *machine);
int create_pci_generic_priv_data(struct virt_machine *machine, void *ptr);

#endif
