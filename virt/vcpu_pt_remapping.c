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
#include "asm/type.h"
#include "asm/pgtable.h"
#include "iommu.h"
#include "virt.h"
#include "mm.h"
#include "print.h"
#include "machine.h"
#include "container_of.h"
#include "../dma-mapping/iova.h"
#include "device.h"
#include "list.h"
#include "pci.h"
#include "string.h"
#include "vcpu_pt_remapping.h"
#include "pci_device_driver.h"
#include "pci_generic_emulator.h"
#include "../drivers/irqchip/aia/imsic/imsic.h"

static int vcpu_create_dma_remapping(struct vcpu *vcpu, struct iommu_group *group)
{
	int i;
	struct iommu *iommu;
	unsigned long iova;

	for (i = 0; i < vcpu->iommu_nr; i++) {
		iommu = vcpu->iommu[i];
		if (iommu->ops->page_mapping) {
			iova = iova_alloc(&group->iova_cookie, vcpu->memory_size);
			if (iova == -1UL) {
				print("warning -- virt: iova_alloc failed\n");
				return -1;
			}
			iommu->ops->page_mapping(group->pgdp_gstage,
						 vcpu->guest_memory_pa,
						 (void *)vcpu->host_memory_pa,
						 vcpu->memory_size, NULL);
		}
	}

	return 0;
}

int vcpu_create_interrupt_remapping(struct vcpu *vcpu)
{
	int i;
	struct iommu *iommu;
	unsigned long msi_addr_gpa, msi_addr_hpa;
	unsigned int msi_addr_size;
	struct iommu_group *group = vcpu->iommu_group;

	msi_addr_gpa = get_machine_memmap_base(VIRT_IMSIC);
	msi_addr_size = get_machine_memmap_size(VIRT_IMSIC);
	msi_addr_hpa = imsic_get_interrupt_file_base(vcpu->cpu, vcpu->hgei);

	for (i = 0; i < vcpu->iommu_nr; i++) {
		iommu = vcpu->iommu[i];
		if (iommu->ops->map_msi_addr) {
			iommu->ops->map_msi_addr(group, iommu, msi_addr_hpa,
						 msi_addr_gpa, msi_addr_size);
		}
	}

	return 0;
}

void vcpu_create_pt_remapping(struct vcpu *vcpu)
{
	if (!vcpu->iommu_group)
		return;

	vcpu_create_dma_remapping(vcpu, vcpu->iommu_group);
}

int vcpu_attach_device_group(struct vcpu *vcpu, struct device **p_dev, int n)
{
	struct iommu_group *iommu_group;
	int i, j;

	for (i = 0; i < n; i++)
		iommu_device_dettach_group(p_dev[i]);

	iommu_group = iommu_alloc_group();
	if (!iommu_group)
		return -1;

	vcpu->pt_devices = (struct device **)mm_alloc(sizeof(unsigned long) * n);
	if (!vcpu->pt_devices) {
		print("warning -- virt: alloc pt_devices failed\n");
		return -1;
	}

	for (i = 0; i < n; i++) {
		if (!p_dev[i]->iommu) {
			print("warning -- virt: device(%s) no iommu\n", p_dev[i]->compatible);
			continue;
		}
		vcpu->pt_devices[vcpu->pt_device_nr++] = p_dev[i];
		iommu_device_attach_group(p_dev[i], iommu_group);
		iommu_attach_device(p_dev[i], p_dev[i]->iommu, 1);
		for (j = 0; j < vcpu->iommu_nr; j++) {
			if (vcpu->iommu[j] == p_dev[i]->iommu)
				continue;
		}
		if (vcpu->iommu_nr > 16) {
			print("warning -- virt: Support only 16 iommus in a vcpu\n");
			continue;
		}
		vcpu->iommu[vcpu->iommu_nr] = p_dev[i]->iommu;
		vcpu->iommu_nr++;
	}

	vcpu->iommu_group = iommu_group;

	return 0;
}

static unsigned long get_pci_bar_mask(unsigned int sz)
{
	int n = 0;
	unsigned long mask;

	if (sz == 0)
		return -1;

	while (!(sz & 0x1UL)) {
		sz = sz >> 1;
		n++;
	}

	mask = (~(1UL << n)) + 1;

	return mask;
}

static unsigned int pci_find_cap_ptr(struct pci_device_function_emulator *func, int start, int len)
{
	unsigned long cap_ptr = 0x40;
	unsigned char *in_used = func->inused_bitmap;
	int i;

	if ((!start) && (start >= 0x40) && (start <= 0xff)) {
		cap_ptr = start;
		for (i = cap_ptr; i < len; i++) {
			if (in_used[i] != 0)
				return 0;
		}
		for (i = cap_ptr; i < len; i++) {
			in_used[i] = 1;
		}
	} else {
		while (cap_ptr < 0xff - len) {
			for (i = cap_ptr; i < len; i++) {
				if (in_used[i] != 0)
					goto next;
			}
			for (i = cap_ptr; i < len; i++) {
				in_used[i] = 1;
			}
			return cap_ptr;

next:
			cap_ptr += i - cap_ptr + 1;
		}
	}

	return 0;
}

static int pci_add_cap(struct pci_device_function_emulator *func, int cap_id, unsigned int cap_ptr)
{
	struct type0_cfg *cfg = &func->cfg_space.type0_cfg;
	char *ptr;
	char cap = cfg->cap_ptr;
	unsigned char next_pos;

	ptr = (char *)((char *)cfg + cap);
	next_pos = *(ptr + 1);
	while (next_pos) {
		ptr = (char *)((unsigned long)next_pos + 1);
		if (*ptr == cap_id)
			return -1;
		next_pos = *(ptr + 1);
	}

	*(ptr + 1) = cap_ptr;

	return 0;
}

static void pcie_msix_init(struct pci_device_function_emulator *func,
			   unsigned int cap_ptr, int nr_irq,
			   int table_bar, unsigned int table_offset,
			   int pba_bar, unsigned int pba_offset)
{
	struct type0_cfg *cfg = &func->cfg_space.type0_cfg;
	unsigned int msix_cap;
	struct msix_cap *ptr;
	unsigned short msg_ctrl = 0;
	int table_size;

	msix_cap = pci_find_cap_ptr(func, cap_ptr, MSIX_CAP_LENGTH);
	if (!msix_cap) {
		print("warning -- pcie-msix-init fail\n");
		return;
	}
	ptr = (struct msix_cap *)((char *)cfg + msix_cap);
	memset((char *)ptr, 0, MSIX_CAP_LENGTH);

	table_size = nr_irq - 1;
	msg_ctrl = table_size;

	ptr->cap_id = PCI_CAP_ID_MSIX;
	ptr->next_cap = 0;
	ptr->msg_ctrl = msg_ctrl;
	ptr->table_offset = (table_offset & PCI_MSIX_TABLE_OFFSET) |
			    (table_bar & PCI_MSIX_TABLE_BIR);
	ptr->pba_offset = (pba_offset & PCI_MSIX_PBA_OFFSET) |
			  (pba_bar & PCI_MSIX_PBA_BIR);

	pci_add_cap(func, PCI_CAP_ID_MSIX, msix_cap);
}

static void pcie_pt_device_msix_init(struct pci_device *pdev,
				     struct pci_device_function_emulator *func)
{
	int msi_vec_count;
	unsigned int table_offset, pba_offset;
	int table_bar, pba_bar;

	msi_vec_count = pci_msix_get_vec_count(pdev);
	table_offset = pci_read_config_dword(pdev->bus, pdev->devfn,
					     pdev->msix_cap_pos + PCI_MSIX_TABLE);
	table_bar = table_offset & PCI_MSIX_TABLE_BIR;
	table_offset &= PCI_MSIX_TABLE_OFFSET;

	pba_offset = pci_read_config_dword(pdev->bus, pdev->devfn,
					   pdev->msix_cap_pos + PCI_MSIX_PBA);
	pba_bar = pba_offset & PCI_MSIX_PBA_BIR;
	pba_offset &= PCI_MSIX_PBA_OFFSET;

	pcie_msix_init(func, 0x40, msi_vec_count,
		       table_bar, table_offset,
		       pba_bar, pba_offset);
}

static void pci_func_bar_init(struct pci_device *pdev,
			      struct pci_device_function_emulator *func)
{
	int pos;
	unsigned int reg, l;
	struct pt_device_resource *res = func->res;
	struct resource *r;
	struct type0_cfg *cfg = &func->cfg_space.type0_cfg;
	unsigned int *bar0 = &cfg->bar0;
	unsigned long sz;
	unsigned int sz_mask;

	for (pos = 0; pos < 6; pos++) {
		reg = PCI_BASE_ADDRESS_0 + (pos << 2);
		l = pci_read_config_dword(pdev->bus, pdev->devfn, reg);
		*(bar0 + (pos << 2)) = l & 0xf;
		if ((l & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_64)
			pos++;
	}

	if (!res)
		return;

	for (pos = 0; pos < 6; pos++) {
		r = &res->_res.pci_dev_res[pos];
		sz = r->end - r->base + 1;
		if (sz == 1)
			continue;
		sz_mask = get_pci_bar_mask(sz);
		func->bar_mask[pos] = sz_mask;
		if (sz >= (1UL << 32)) {
			pos++;
			sz_mask = get_pci_bar_mask(sz >> 32);
			func->bar_mask[pos] = sz_mask;
		}
	}
}

static void pcie_cap_init(struct pci_device_function_emulator *func, unsigned int cap)
{
	struct type0_cfg *cfg = &func->cfg_space.type0_cfg;
	char *exp_cap;

	cfg->cap_ptr = cap;
	exp_cap = (char *)(cfg + cap);
	*exp_cap = PCI_CAP_ID_EXP;
	*(exp_cap + 1) = 0x0;
}

static int find_free_pci_slot_and_plugin(struct pci_generic_emulator *pci_emulator,
				         struct pci_device_emulator *pci_dev)
{
	int i;
	struct pci_device_emulator **slot;

	for (i = 0; i < 32; i++) {
		slot = &pci_emulator->slot[i];
		if (*slot == NULL)
			goto plugin;
	}

	return -1;
plugin:
	*slot = pci_dev;

	return 0;
}

static void pci_emulator_gstage_ioremap(void *pgdp, unsigned long gpa,
					unsigned long hpa, unsigned int size)
{
	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY |
		     _PAGE_USER);

	print("%s -- gpa: 0x%lx, hpa:0x%lx, size:0x%x\n", __FUNCTION__, gpa, hpa, size);
	mmu_gstage_page_mapping(pgdp, hpa, gpa, size, pgprot);
}

static void pci_process_type0_cfg_bar_write(struct pci_device_function_emulator *func,
					    unsigned int reg, unsigned long val, int len)
{
	unsigned long base = get_machine_memmap_base(VIRT_PCI_MMIO);
	unsigned int size = get_machine_memmap_size(VIRT_PCI_MMIO);
	int bar0_offset = offsetof(struct type0_cfg, bar0);
	int offset = (reg - bar0_offset) >> 2;
	struct type0_cfg *cfg = &func->cfg_space.type0_cfg;
	unsigned int *bar0 = &cfg->bar0;
	struct resource *res = &func->res->_res.pci_dev_res[offset];
	struct virt_machine *machine = (struct virt_machine *)func->data;

	*(bar0 + offset) = val & func->bar_mask[offset];

	if ((val >= base) && (val < base + size)) {
		if (machine)
			pci_emulator_gstage_ioremap((unsigned long *)machine->gstage_pgdp,
						    val, res->base, res->end - res->base + 1);
	}
}

static void pci_process_type0_cfg_write(struct pci_device_function_emulator *func,
					unsigned int reg, unsigned long val, int len)
{
	int bar0_offset = offsetof(struct type0_cfg, bar0);
	int bar5_offset = offsetof(struct type0_cfg, bar5);

	if ((reg >= bar0_offset) && (reg <= bar5_offset)) {
		pci_process_type0_cfg_bar_write(func, reg, val, len);
	}
}

static int pci_pt_device_emu_contained(struct pci_device_function_emulator *func,
				       unsigned long addr)
{
	return 0;
}

static int pci_pt_device_emu_cfg_write(struct pci_device_function_emulator *func,
				       unsigned int reg, unsigned long val, int len)
{
	if (func->cfg_space.type0_cfg.header_type == 0x0)
		pci_process_type0_cfg_write(func, reg, val, len);

	return 0;
}

static unsigned long pci_pt_device_emu_cfg_read(struct pci_device_function_emulator *func,
						unsigned int reg, int len)
{
	unsigned long data;

	if (len == 1)
		data = *(func->cfg_space.data + reg);
	else if (len == 2)
		data = *((unsigned short *)(func->cfg_space.data + reg));
	else if (len == 4)
		data = *((unsigned int *)(func->cfg_space.data + reg));
	else if (len == 8)
		data = *((unsigned long *)(func->cfg_space.data + reg));

	return data;
}

static struct pci_devfn_emu_ops pci_pt_device_emu_ops = {
	.contained_pci_address = pci_pt_device_emu_contained,
	.config_write = pci_pt_device_emu_cfg_write,
	.config_read = pci_pt_device_emu_cfg_read,
};

static int create_pci_pt_devices(struct virt_machine *machine,
				 struct pt_device_resource *res)
{
	struct pci_device *pdev;
	struct device *dev = res->dev;
	struct pci_device_emulator *dev_emu;
	struct pci_device_function_emulator *func;

	if (!dev)
		return -1;

	if (!is_pci_device(dev))
		return -1;

	pdev = to_pci_dev(dev);
	if (!pdev) {
		print("warning -- vcpu-pt-remapping: create pci pt device fail, pdev is NULL\n");
		return -1;
	}

	dev_emu = (struct pci_device_emulator *)mm_alloc(sizeof(struct pci_device_emulator));
	if (!dev_emu) {
		print("warning -- vcpu-pt-remapping: alloc pci_device_emulator fail\n");
		return -1;
	}

	func = (struct pci_device_function_emulator *)
		mm_alloc(sizeof(struct pci_device_function_emulator));
	if (!func) {
		print("warning -- vpu-pt-remapping: alloc pci func fail\n");
		return -1;
	}
	func->ops = &pci_pt_device_emu_ops;
	func->cfg_space.type0_cfg.vendor_id = pdev->vendor;
	func->cfg_space.type0_cfg.device_id = pdev->device;
	func->cfg_space.type0_cfg.header_type = 0x0;
	func->function_num = 0;
	func->res = res;
	func->offset = machine->pci_emu->cpu_addr - machine->pci_emu->pci_addr;
	func->data = (void *)machine;
	pci_func_bar_init(pdev, func);
	pcie_cap_init(func, 0xe0);
	pcie_pt_device_msix_init(pdev, func);

	dev_emu->function[0] = func;
	if (find_free_pci_slot_and_plugin(machine->pci_emu, dev_emu)) {
		print("warning -- vcpu-pt-remapping: pci device plugin faile\n");
		return -1;
	}

	return 0;
}

static void get_pt_device_resource(struct virt_machine *machine, struct device *dev)
{
	int i;
	struct pt_device_resource *res;

	res = (struct pt_device_resource *)mm_alloc(sizeof(struct pt_device_resource));
	if (!res) {
		print("warning -- virt machine: alloc pt_device_resource failed\n");
		return;
	}

	if (is_pci_device(dev)) {
		for (i = 0; i < 6; i++) {
			pci_get_resource(to_pci_dev(dev), i, &res->_res.pci_dev_res[i]);
		}
	}
	else {
		device_get_resource(dev, &res->_res.dev_res);
	}

	res->dev = dev;

	list_add_tail(&res->list, &machine->pt_devices_res);
}

void vcpu_create_pt_device(struct vcpu *vcpu)
{
	struct pt_device_resource *res;
	struct virt_machine *machine = &vcpu->machine;
	int i;
	struct device *dev;

	if (vcpu->pt_device_nr == 0)
		return;

	INIT_LIST_HEAD(&machine->pt_devices_res);

	for (i = 0; i < vcpu->pt_device_nr; i++) {
		dev = vcpu->pt_devices[i];
		get_pt_device_resource(machine, dev);
	}

	list_for_each_entry(res, &machine->pt_devices_res, list) {
		if (is_pci_device(res->dev))
			create_pci_pt_devices(machine, res);
	}
}
