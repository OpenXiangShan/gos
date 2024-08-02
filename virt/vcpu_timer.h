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

#ifndef __VCPU_TIMER_H
#define __VCPU_TIMER_H

#include "virt.h"

int vcpu_timer_init(struct vcpu *vcpu);
void vcpu_time_init(struct vcpu *vcpu);
unsigned long vcpu_get_system_time(struct vcpu *vcpu);
void vcpu_timer_save(struct vcpu *vcpu);
void vcpu_timer_restore(struct vcpu *vcpu);

#endif
