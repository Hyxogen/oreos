#include <kernel/errno.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <kernel/kernel.h>

static struct process *_proc_list;
static struct process * _Atomic _proc_cur;
static struct process *_idle_proc;
static atomic_int _pid;
static atomic_int _preemption_disabled_count = 1; /* we default to 1 until the scheduler is started */

struct process *_sched_cur(void)
{
	return atomic_load(&_proc_cur);
}

static void sched_set_cur(struct process *proc)
{
	atomic_store(&_proc_cur, proc);
}

bool sched_disable_preemption(void)
{
	/* TODO check if seq_cst is really needed */
	/* In theory this is just a recursive spinlock */
	int i = atomic_fetch_add_explicit(&_preemption_disabled_count, 1, memory_order_seq_cst);
	assert(i >= 0);
	return i == 0;
}

bool sched_enable_preemption(void)
{
	int i = atomic_fetch_sub_explicit(&_preemption_disabled_count, 1, memory_order_seq_cst);
	assert(i >= 0);
	return i == 1;
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
	assert(atomic_load(&_preemption_disabled_count) != 0);
	struct process *prev = _sched_cur();
	struct process *cur = NULL;
	if (prev) {
		if (state)
			prev->context = state;
		if (prev->status == RUNNING)
			prev->status = READY;

		cur = prev->next;
	}

	while (1) {
		if (!cur)
			cur = _proc_list;

		if (!cur) {
			cur = _idle_proc;
			break;
		}

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
	struct process *prev = _sched_cur();
	struct process *next = _sched_next(state);
	assert(next);

	if (prev != next) {
		disable_irqs();
		mmu_invalidate_user(); /* do irqs have to be disabled? */
		/* proc_prepare_switch enables irqs */
		proc_prepare_switch(next);
	}

	bool can_preempt = sched_enable_preemption();
	assert(can_preempt);
	return_from_irq(next->context);
}

void sched_resume(struct cpu_state *state)
{
	sched_disable_preemption();

	struct process *proc = _sched_cur();
	/* TODO make sure that you remove the saved check when just resuming for
	 * signals (SEE TODO BELOW) */
	if (is_from_userspace(state) && proc->pending_signals) {
		if (proc_do_signal(proc, state)) {
			proc->status = DEAD;
			proc->exit_code = -2; /* TODO set proper exit code */
			_sched_yield(state);
		}
	}

	assert(!is_from_userspace(state) || proc->status == RUNNING);

	sched_enable_preemption();
	return_from_irq(state);
}

void sched_yield(struct cpu_state *state)
{
	bool can_preempt = sched_disable_preemption();
	assert(can_preempt);
	_sched_yield(state);
}

static enum irq_result sched_on_tick(u8 irqn, struct cpu_state *state, void *dummy)
{
	(void)irqn;
	(void)dummy;
	(void)state;
	if (timer_poll() == 0) {
		bool can_preempt = sched_disable_preemption();
		if (can_preempt)
			_sched_yield(state);
		else
			sched_enable_preemption();
	}
	return IRQ_CONTINUE;
}

__attribute__((noreturn))
static void sched_idle(void)
{
	while (1)
		halt();
}

void init_sched(void)
{
	irq_register_handler(timer_get_irqn(), sched_on_tick, NULL);

	atomic_init(&_pid, 1);

	_idle_proc = proc_create(sched_idle, PROC_FLAG_RING0);
	assert(_idle_proc);
	_idle_proc->pid = 0;
}

int sched_kill(struct process *proc, int exit_code)
{
	/* TODO this will probably have to use atomics once we want SMP,
	 * otherwise races could exits for accessing the status and/or exit_code
	 */
	int res = -ESRCH;
	sched_disable_preemption();

	if (proc->status != DEAD) {
		proc->exit_code = exit_code;
		proc->status = DEAD;
		res = 0;
	}

	sched_enable_preemption();
	return res;
}

struct process *sched_get_current_proc(void)
{
	sched_disable_preemption();
	struct process *cur = _sched_cur();
	proc_get(cur);
	sched_enable_preemption();
	return cur;
}

struct process *sched_get(int pid)
{
	sched_disable_preemption();

	struct process *proc = _proc_list;

	while (proc) {
		if (proc->pid == pid) {
			proc_get(proc);
			break;
		}
		proc = proc->next;
	}

	sched_enable_preemption();
	return proc;
}

int sched_schedule(struct process *proc)
{
	proc_get(proc);

	proc->pid = atomic_fetch_add(&_pid, 1);
	proc->status = READY;
	proc->next = NULL;

	sched_disable_preemption();

	if (!_proc_list) {
		_proc_list = proc;
	} else {
		struct process *last = _proc_list;

		while (last->next)
			last = last->next;
		last->next = proc;
	}

	sched_enable_preemption();
	return 0;
}

void sched_signal(struct process *proc, int signum)
{
	(void) signum;
	sched_disable_preemption();

	proc->pending_signals |= 1 << signum;

	sched_enable_preemption();
}

void sched_save(struct cpu_state *state)
{
	sched_disable_preemption();

	struct process *proc = _sched_cur();
	assert(proc);
	proc->context = state;

	sched_enable_preemption();
}

void sched_start(void)
{
	_sched_yield(NULL);
}
