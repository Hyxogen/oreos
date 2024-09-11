#include <stdbool.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/malloc/malloc.h>
#include <kernel/libc/assert.h>
#include <kernel/timer.h>

static struct process *_proc_list;
static struct process *_proc_cur;
static int _pid;

static bool del_proc(int pid)
{
	struct process *cur = _proc_list;
	struct process *prev = NULL;
	while (cur) {
		if (cur->pid == pid) {
			if (!prev)
				_proc_list = cur->next;
			else
				prev->next = cur->next;

			kfree(cur);
			return true;
		}

		prev = cur;
		cur = cur->next;
	}
	return false;
}

int sched_proc(struct cpu_state *ctx)
{
	struct process *proc = kmalloc(sizeof(*proc));
	if (!proc)
		return -1;

	proc->pid = _pid++;
	proc->status = READY;
	proc->next = NULL;
	proc->context = ctx;

	if (!_proc_list) {
		_proc_list = proc;
	} else {
		struct process *last = _proc_list;

		while (last->next)
			last = last->next;
		last->next = proc;
	}
	return 0;
}

struct cpu_state *sched_schedule(struct cpu_state *state)
{
	if (_proc_cur) {
		struct process *prev = _proc_cur;
		prev->status = READY;
		prev->context = state;

		_proc_cur = prev->next;
	}

	while (1) {
		if (!_proc_cur)
			_proc_cur = _proc_list;

		/* TODO schedule idle process */
		assert(_proc_cur && "nothing to schedule");

		if (_proc_cur->status == DEAD) {
			struct process *next = _proc_cur->next;
			del_proc(_proc_cur->pid);
			_proc_cur = next;
		} else {
			break;
		}

	}
	timer_sched_int(10);
	_proc_cur->status = RUNNING;
	return _proc_cur->context;
}

bool sched_should_preempt(void)
{
	return timer_poll() == 0;
}

void init_sched(void) {}

void sched_start(void)
{
	__asm__ volatile("int 0xab");
	panic("sched_start should not return!");
	//timer_sched_int(0);
}
