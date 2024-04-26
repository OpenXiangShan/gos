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
