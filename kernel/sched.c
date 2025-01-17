#include <kernel/errno.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <kernel/kernel.h>
#include <kernel/signal.h>

static struct process *_proc_list;
static struct process * _Atomic _proc_cur;
static struct process *_idle_proc;
static atomic_int _pid;
static atomic_int _preemption_disabled_count = 1; /* we default to 1 until the scheduler is started */
static u8 _yield_irqn;
static atomic_uint _clock = 0;

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
	/* TODO In theory this is just a recursive spinlock, perhaphs just use one */
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
		struct process *next = cur->next;

		if (cur->status == DEAD) {
			printk("%i exited with: %i\n", cur->pid, cur->exit_code);
			_sched_del_proc(cur->pid);
		} else {
			if (cur->alarm && cur->alarm < sched_get_time()) {
				cur->alarm = 0;
				sched_signal(cur->pid, SIGALRM);
			}

			if (cur->status == READY) {
				break;
			}
		}

		cur = next;
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
	/* TODO we should probably disable irqs here, or we might get preempted
	 * here */
	assert(can_preempt);
	return_from_irq(next->context);
}

void sched_resume(struct cpu_state *state)
{
	sched_disable_preemption();

	struct process *proc = _sched_cur();
	if (is_from_userspace(state) && proc->pending_signals) {
		int res = proc_do_signal(proc, state);
		if (res) {
			sched_enable_preemption();
			sched_do_kill(res);
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
	atomic_fetch_add_explicit(&_clock, 1, memory_order_relaxed);

	if (timer_poll() == 0) {
		bool can_preempt = sched_disable_preemption();
		if (can_preempt)
			_sched_yield(state);
		else
			sched_enable_preemption();
	}
	return IRQ_CONTINUE;
}

static enum irq_result sched_on_yield(u8 irqn, struct cpu_state *state, void *dummy)
{
	(void)irqn;
	(void)dummy;
	(void)state;
	bool can_preempt = sched_disable_preemption();
	if (can_preempt)
		_sched_yield(state);
	else
		oops("could not yield because preemption was disabled");
	sched_enable_preemption();
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

	i16 irqn = irq_get_free_irq();
	assert(irqn > 0);
	_yield_irqn = (u8) irqn;

	irq_register_handler(_yield_irqn, sched_on_yield, NULL);
}

static struct process *_sched_get(int pid)
{
	sched_disable_preemption();

	struct process *proc = _proc_list;

	while (proc) {
		if (proc->pid == pid) {
			break;
		}
		proc = proc->next;
	}

	sched_enable_preemption();
	return proc;
}

static struct process *_sched_get_parent(void)
{
	struct process *parent = _sched_get(atomic_load_explicit(&_sched_cur()->parent_pid, memory_order_relaxed));
	if (!parent)
		parent = sched_get_init();
	assert(parent);
	return parent;
}

void sched_do_kill(int exit_code)
{
	sched_disable_preemption();

	struct process *proc = _sched_cur();

	if (proc->status != DEAD) {
		proc->exit_code = exit_code;
		proc->status = DEAD;

		struct process *parent = _sched_get_parent();
		monitor_signal(&parent->child_exited_cond);
	}

	_sched_yield(NULL);
}

struct process *sched_get_current_proc(void)
{
	sched_disable_preemption();
	struct process *cur = _sched_cur();
	sched_enable_preemption();
	return cur;
}

struct process *sched_get_init(void)
{
	return _sched_get(1);
}

int sched_schedule(struct process *proc)
{
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

void sched_yield_here(void) {
	do_irq(_yield_irqn);
}

void sched_prepare_goto_sleep(void)
{
	sched_disable_preemption();

	struct process *proc = _sched_cur();

	assert(proc->status != DEAD);
	proc->status = SLEEPING;

	sched_enable_preemption();
}

void sched_goto_sleep(void)
{
	sched_prepare_goto_sleep();
	sched_yield_here();
}

void sched_wakeup(struct process *proc)
{
	sched_disable_preemption();

	enum proc_status sleeping = SLEEPING;

	atomic_compare_exchange_strong(&proc->status, &sleeping, READY);

	sched_enable_preemption();
}

unsigned sched_set_alarm(struct process *proc, unsigned timepoint)
{
	sched_disable_preemption();

	unsigned prev = proc->alarm;
	proc->alarm = timepoint;

	sched_enable_preemption();

	return prev;
}

int sched_signal(int pid, int signum)
{
	(void) signum;
	sched_disable_preemption();

	int res = -ENOENT;
	struct process *proc = _sched_get(pid);

	if (proc) {
		proc->pending_signals |= 1 << signum;
		sched_wakeup(proc);
		res = 0;
	}

	sched_enable_preemption();
	return res;
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

bool sched_has_pending_signals()
{
	sched_disable_preemption();
	bool pending = _sched_cur()->pending_signals;
	sched_enable_preemption();
	return pending;
}

unsigned sched_get_time(void)
{
	return atomic_load_explicit(&_clock, memory_order_relaxed);
}
