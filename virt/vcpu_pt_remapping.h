#ifndef __VCPU_PT_REMAPPING_H__
#define __VCPU_PT_REMAPPING_H__

#include "virt.h"
#include "device.h"

struct pt_device_resource {
	struct list_head list;
	struct device *dev;
	union res {
		struct resource dev_res;
		struct resource pci_dev_res[6];
	} _res;
};

void vcpu_create_pt_remapping(struct vcpu *vcpu);
void vcpu_create_pt_device(struct vcpu *vcpu);
int vcpu_attach_device_group(struct vcpu *vcpu, struct device **p_dev, int n);

#endif
