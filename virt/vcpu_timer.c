#include "vcpu_timer.h"
#include "virt.h"
#include "timer.h"
#include "asm/csr.h"
#include "clock.h"

unsigned long vcpu_get_system_time(struct vcpu *vcpu)
{
	long delta = (long)vcpu->time_delta;
	long now = (long)get_system_tick();

	return now + delta;
}

static void vcpu_timer_handler(void *data)
{
	struct vcpu *vcpu = (struct vcpu *)data;

	vcpu_set_interrupt(vcpu, IRQ_VS_TIMER);
}

static void vcpu_timer_next_event(unsigned long next, void *data)
{
	struct vcpu *vcpu = (struct vcpu *)data;
	struct vcpu_timer *t = &vcpu->timer;
	unsigned long now, delta;
	unsigned long ms;

	vcpu_clear_interrupt(vcpu, IRQ_VS_TIMER);
	now = vcpu_get_system_time(vcpu);
	if (now >= next)
		delta = 0;
	else
		delta = next - now;

	ms = cycles_to_ms(delta, get_clock_source_freq());
	set_timer(ms, t->timer_handler, vcpu);
}

void vcpu_time_init(struct vcpu *vcpu)
{
	vcpu->time_delta = -get_system_time();
}

int vcpu_timer_init(struct vcpu *vcpu)
{
	struct vcpu_timer *t = &vcpu->timer;

	t->timer_handler = vcpu_timer_handler;
	t->next_event = vcpu_timer_next_event;
	t->data = (void *)vcpu;

	return 0;
}
