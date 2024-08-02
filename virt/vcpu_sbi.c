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

#include "virt.h"
#include "vcpu_timer.h"
#include "print.h"
#include "asm/sbi.h"

static void vcpu_sbi_set_timer(struct vcpu *vcpu, unsigned long next)
{
	struct vcpu_timer *t = &vcpu->timer;

	if (!t->next_event)
		return;

	t->next_event(next, t->data);
}

int vcpu_sbi_call(struct vcpu *vcpu)
{
	struct virt_cpu_context *guest_ctx;
	int sbi_id;

	guest_ctx = &vcpu->cpu_ctx.guest_context;
	sbi_id = guest_ctx->a7;

	switch (sbi_id) {
	case SBI_SET_TIMER:
		vcpu_sbi_set_timer(vcpu, guest_ctx->a0);
		break;
	default:
		print("%s -- Unsupport sbi id: %d\n", __FUNCTION__, sbi_id);
	}

	guest_ctx->sepc += 4;

	return 0;
}
