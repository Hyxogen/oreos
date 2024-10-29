#include <kernel/errno.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>
#include <kernel/printk.h>
#include <kernel/timer.h>

static struct process *_proc_list;
static struct process * _Atomic _proc_cur;
static atomic_int _pid;
static atomic_bool _preemption_enabled = false;

struct process *_sched_cur(void)
{
	return atomic_load(&_proc_cur);
}

static void sched_set_cur(struct process *proc)
{
	atomic_store(&_proc_cur, proc);
}

static bool sched_set_preemption(bool enabled)
{
	return atomic_exchange(&_preemption_enabled, enabled);
}

void proc_release(struct process *proc)
{
	if (atomic_fetch_sub(&proc->refcount, 1) == 0) {
		proc_free(proc);
	}
}

void proc_get(struct process *proc)
{
	atomic_fetch_add(&proc->refcount, 1);
}

static void _sched_del_proc(int pid)
{
	struct process *cur = _proc_list;
	struct process *prev = NULL;
	while (cur) {
		if (cur->pid == pid) {
			if (!prev)
				_proc_list = cur->next;
			else
				prev->next = cur->next;
			proc_release(cur);
			break;
		}

		prev = cur;
		cur = cur->next;
	}
}

static struct process *_sched_next(struct cpu_state *state)
{
	struct process *prev = _sched_cur();
	struct process *cur = NULL;
	if (prev) {
		prev->context = state;
		if (prev->status == RUNNING)
			prev->status = READY;

		cur = prev->next;
	}

	while (1) {
		if (!cur)
			cur = _proc_list;

		// TODO schedule idle process
		assert(cur && "nothing to schedule");

		if (cur->status == DEAD) {
			printk("%i exited with: %i\n", cur->pid, cur->exit_code);
			struct process *next = cur->next;
			_sched_del_proc(cur->pid);
			cur = next;
		} else {
			break;
		}
	}
	timer_sched_int(10);
	cur->status = RUNNING;

	sched_set_cur(cur);
	return cur;
}

__attribute__((noreturn))
static void _sched_yield(struct cpu_state *state)
{
	struct process *next = _sched_next(state);
	assert(next);

	proc_prepare_switch(next);

	sched_set_preemption(true);
	return_from_irq(next->context);
}

void sched_resume(struct cpu_state *state)
{
	bool saved = sched_set_preemption(false);

	struct process *proc = _sched_cur();
	if (is_from_userspace(state) && proc->status != RUNNING)
		_sched_yield(state);

	sched_set_preemption(saved);
	return_from_irq(state);
}

void sched_yield(struct cpu_state *state)
{
	assert(sched_set_preemption(false));
	assert(state || _sched_cur()->status == DEAD);
	_sched_yield(state);
}

static enum irq_result sched_on_tick(u8 irqn, struct cpu_state *state, void *dummy)
{
	(void)irqn;
	(void)dummy;
	(void)state;
	if (timer_poll() == 0) {
		if (sched_set_preemption(false))
			_sched_yield(state);
	}
	return IRQ_CONTINUE;
}

void init_sched(void)
{
	irq_register_handler(timer_get_irqn(), sched_on_tick, NULL);
}

int sched_kill(struct process *proc, int exit_code)
{
	/* TODO this will probably have to use atomics once we want SMP,
	 * otherwise races could exits for accessing the status and/or exit_code
	 */
	int res = -ESRCH;
	bool saved = sched_set_preemption(false);

	if (proc->status != DEAD) {
		proc->exit_code = exit_code;
		proc->status = DEAD;
		res = 0;
	}

	sched_set_preemption(saved);
	return res;
}

struct process *sched_get_current_proc(void)
{
	bool saved = sched_set_preemption(false);
	struct process *cur = _sched_cur();
	proc_get(cur);
	sched_set_preemption(saved);
	return cur;
}

int sched_schedule(struct process *proc)
{
	proc_get(proc);

	proc->pid = atomic_fetch_add(&_pid, 1);
	proc->status = READY;
	proc->next = NULL;

	bool saved = sched_set_preemption(false);

	if (!_proc_list) {
		_proc_list = proc;
	} else {
		struct process *last = _proc_list;

		while (last->next)
			last = last->next;
		last->next = proc;
	}

	sched_set_preemption(saved);
	return 0;
}

void sched_signal(struct process *proc, int signum)
{
	(void) signum;
	bool saved = sched_set_preemption(false);

	proc->status = DEAD;
	proc->exit_code = -1;

	sched_set_preemption(saved);
}

void sched_start(void)
{
	_sched_yield(NULL);
}
