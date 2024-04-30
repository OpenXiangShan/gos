#ifndef __VCPU_AIA_H__
#define __VCPU_AIA_H__

#include "virt.h"

int vcpu_interrupt_file_upadte(struct vcpu *vcpu);
int vcpu_aia_init(struct vcpu *vcpu);

#endif
