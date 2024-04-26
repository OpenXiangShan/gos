#ifndef __VCPU_TIMER_H
#define __VCPU_TIMER_H

#include "virt.h"

int vcpu_timer_init(struct vcpu *vcpu);
void vcpu_time_init(struct vcpu *vcpu);
unsigned long vcpu_get_system_time(struct vcpu *vcpu);

#endif
