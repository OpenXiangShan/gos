#ifndef __VCPU_AIA_H__
#define __VCPU_AIA_H__

#include "virt.h"

int vcpu_interrupt_file_update(struct vcpu *vcpu);
int vcpu_aia_init(void);

#endif
