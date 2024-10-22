#include <stdbool.h>
#include <stdatomic.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/malloc/malloc.h>
#include <kernel/libc/assert.h>
#include <kernel/timer.h>
#include <kernel/irq.h>
#include <kernel/debug.h>

//TODO remove
#include <kernel/printk.h>

static struct process *_proc_list;
static struct process *_proc_cur;
static struct process *_idle_proc;
static atomic_int _pid = 0;
static atomic_bool _enable_preempt = false;

struct process *sched_cur(void)
{
	return _proc_cur;
}

bool sched_set_preemption(bool enabled)
{
	return atomic_exchange(&_enable_preempt, enabled);
}

static void del_proc(int pid)
{
	bool stored = sched_set_preemption(false);

	struct process *cur = _proc_list;
	struct process *prev = NULL;
	while (cur) {
		if (cur->pid == pid) {
			if (!prev)
				_proc_list = cur->next;
			else
				prev->next = cur->next;
			proc_free(cur);
			break;
		}

		prev = cur;
		cur = cur->next;
	}

	sched_set_preemption(stored);
}

static void sched_idle(void)
{
	while (1)
		halt();
}

int sched_proc(struct process *proc)
{
	proc->pid = atomic_fetch_add_explicit(&_pid, 1, memory_order_relaxed);
	proc->status = READY;
	proc->next = NULL;

	bool prev = sched_set_preemption(false);
	if (!_proc_list) {
		_proc_list = proc;
	} else {
		struct process *last = _proc_list;

		while (last->next)
			last = last->next;
		last->next = proc;
	}
	sched_set_preemption(prev);
	return 0;
}

struct process *sched_schedule(struct cpu_state *state)
{
	if (_proc_cur) {
		struct process *prev = _proc_cur;
		if (prev->status != DEAD)
			prev->status = READY;
		prev->context = state;

		_proc_cur = prev->next;
	}

	while (1) {
		if (!_proc_cur)
			_proc_cur = _proc_list;

		// TODO schedule idle process
		assert(_proc_cur && "nothing to schedule");

		if (_proc_cur->status == DEAD) {
			printk("%i exited with: %i\n", _proc_cur->pid, _proc_cur->exit_code);
			struct process *next = _proc_cur->next;
			del_proc(_proc_cur->pid);
			_proc_cur = next;
		} else {
			break;
		}

	}
	timer_sched_int(10);
	_proc_cur->status = RUNNING;
	return _proc_cur;
}

void sched_preempt(struct cpu_state *state)
{
	struct process *next_proc = sched_schedule(state);
	assert(next_proc);
	proc_prepare_switch(next_proc);

	//TODO what if an interrupt happends before IRET?
	sched_set_preemption(true);
	return_from_irq(next_proc->context);
}

static enum irq_result sched_on_tick(u8 irqn, struct cpu_state *state, void *dummy)
{
	(void)irqn;
	(void)dummy;
	bool expected = true;
	if (timer_poll() == 0 && atomic_compare_exchange_strong(
				     &_enable_preempt, &expected, false)) {
		sched_preempt(state);
		//should preempt
	}
	return IRQ_CONTINUE;
}

void init_sched(void)
{
	irq_register_handler(timer_get_irqn(), sched_on_tick, NULL);

	_idle_proc = NULL;
	_idle_proc = proc_create(sched_idle, PROC_FLAG_RING0);
	assert(_idle_proc);
}

void sched_start(void)
{
	sched_preempt(NULL);
}
