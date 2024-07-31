#include "vcpu_timer.h"
#include "virt.h"
#include "timer.h"
#include "asm/csr.h"
#include "clock.h"
#include "gos.h"

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

#if CONFIG_ENABLE_SSTC
static void vcpu_timer_sstc_next_event(unsigned long next, void *data)
{
	write_csr(CSR_VSTIMECMP, next);
}
#else
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
#endif

#if CONFIG_ENABLE_VS_SSTC
static void vs_sstc_init(void)
{
	unsigned long val;

	val = read_csr(hcounteren);
	val |= HCOUNTEREN_TM;
	write_csr(hcounteren, val);

	val = read_csr(henvcfg);
	val |= HENVCFG_STCE;
	write_csr(henvcfg, val);
}
#endif

void vcpu_timer_save(struct vcpu *vcpu)
{
	struct vcpu_timer *t = &vcpu->timer;

#if CONFIG_ENABLE_SSTC
	t->timecmp = read_csr(CSR_VSTIMECMP);
	t->htimedelta = read_csr(CSR_HTIMEDELTA);
#endif
}

void vcpu_timer_restore(struct vcpu *vcpu)
{
	struct vcpu_timer *t = &vcpu->timer;

#if CONFIG_ENABLE_SSTC
	write_csr(CSR_VSTIMECMP, t->timecmp);
	write_csr(CSR_HTIMEDELTA, t->htimedelta);
#endif
}

void vcpu_time_init(struct vcpu *vcpu)
{
	vcpu->time_delta = -get_system_time();
#if CONFIG_ENABLE_SSTC
	write_csr(htimedelta, vcpu->time_delta);
#endif
}

int vcpu_timer_init(struct vcpu *vcpu)
{
	struct vcpu_timer *t = &vcpu->timer;

	t->timer_handler = vcpu_timer_handler;
#if CONFIG_ENABLE_SSTC
	t->next_event = vcpu_timer_sstc_next_event;
#else
	t->next_event = vcpu_timer_next_event;
#endif
	t->data = (void *)vcpu;

#if CONFIG_ENABLE_VS_SSTC
	vs_sstc_init();
#endif

	return 0;
}
