#include <stddef.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/errno.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/arch/i386/platform.h>

static i32 syscall_stub(void)
{
	return -ENOSYS;
}

static i32 (*_syscall_table[])() = {
    syscall_stub, /* restart_syscall */
    syscall_exit, /* exit */
    syscall_fork, /* fork */
    syscall_read, /* read */
    syscall_write, /* write */
    syscall_stub, /* open */
    syscall_close, /* close */
    syscall_waitpid, /* waitpid */
    syscall_stub, /* creat */
    syscall_stub, /* link */
    syscall_stub, /* unlink */
    syscall_stub, /* execve */
    syscall_stub, /* chdir */
    syscall_stub, /* time */
    syscall_stub, /* mknod */
    syscall_stub, /* chmod */
    syscall_stub, /* lchown */
    syscall_stub, /* break */
    syscall_stub, /* oldstat */
    syscall_stub, /* lseek */
    syscall_getpid, /* getpid */
    syscall_stub, /* mount */
    syscall_stub, /* umount */
    syscall_stub, /* setuid */
    syscall_getuid, /* getuid */
    syscall_stub, /* stime */
    syscall_stub, /* ptrace */
    syscall_alarm, /* alarm */
    syscall_stub, /* oldfstat */
    syscall_pause, /* pause */
    syscall_stub, /* utime */
    syscall_stub, /* stty */
    syscall_stub, /* gtty */
    syscall_stub, /* access */
    syscall_stub, /* nice */
    syscall_stub, /* ftime */
    syscall_stub, /* sync */
    syscall_kill, /* kill */
    syscall_stub, /* rename */
    syscall_stub, /* mkdir */
    syscall_stub, /* rmdir */
    syscall_stub, /* dup */
    syscall_stub, /* pipe */
    syscall_stub, /* times */
    syscall_stub, /* prof */
    syscall_stub, /* brk */
    syscall_stub, /* setgid */
    syscall_stub, /* getgid */
    syscall_signal, /* signal */
    syscall_stub, /* geteuid */
    syscall_stub, /* getegid */
    syscall_stub, /* acct */
    syscall_stub, /* umount2 */
    syscall_stub, /* lock */
    syscall_stub, /* ioctl */
    syscall_stub, /* fcntl */
    syscall_stub, /* mpx */
    syscall_stub, /* setpgid */
    syscall_stub, /* ulimit */
    syscall_stub, /* oldolduname */
    syscall_stub, /* umask */
    syscall_stub, /* chroot */
    syscall_stub, /* ustat */
    syscall_stub, /* dup2 */
    syscall_stub, /* getppid */
    syscall_stub, /* getpgrp */
    syscall_stub, /* setsid */
    syscall_stub, /* sigaction */
    syscall_stub, /* sgetmask */
    syscall_stub, /* ssetmask */
    syscall_stub, /* setreuid */
    syscall_stub, /* setregid */
    syscall_stub, /* sigsuspend */
    syscall_stub, /* sigpending */
    syscall_stub, /* sethostname */
    syscall_stub, /* setrlimit */
    syscall_stub, /* getrlimit */
    syscall_stub, /* getrusage */
    syscall_stub, /* gettimeofday */
    syscall_stub, /* settimeofday */
    syscall_stub, /* getgroups */
    syscall_stub, /* setgroups */
    syscall_stub, /* select */
    syscall_stub, /* symlink */
    syscall_stub, /* oldlstat */
    syscall_stub, /* readlink */
    syscall_stub, /* uselib */
    syscall_stub, /* swapon */
    syscall_stub, /* reboot */
    syscall_stub, /* readdir */
    syscall_mmap, /* mmap */
    syscall_stub, /* munmap */
    syscall_stub, /* truncate */
    syscall_stub, /* ftruncate */
    syscall_stub, /* fchmod */
    syscall_stub, /* fchown */
    syscall_stub, /* getpriority */
    syscall_stub, /* setpriority */
    syscall_stub, /* profil */
    syscall_stub, /* statfs */
    syscall_stub, /* fstatfs */
    syscall_stub, /* ioperm */
    syscall_stub, /* socketcall */
    syscall_stub, /* syslog */
    syscall_stub, /* setitimer */
    syscall_stub, /* getitimer */
    syscall_stub, /* stat */
    syscall_stub, /* lstat */
    syscall_stub, /* fstat */
    syscall_stub, /* olduname */
    syscall_stub, /* iopl */
    syscall_stub, /* vhangup */
    syscall_stub, /* idle */
    syscall_stub, /* vm86old */
    syscall_stub, /* wait4 */
    syscall_stub, /* swapoff */
    syscall_stub, /* sysinfo */
    syscall_stub, /* ipc */
    syscall_stub, /* fsync */
    syscall_sigreturn, /* sigreturn */
    syscall_stub, /* clone */
    syscall_stub, /* setdomainname */
    syscall_stub, /* uname */
    syscall_stub, /* modify_ldt */
    syscall_stub, /* adjtimex */
    syscall_stub, /* mprotect */
    syscall_stub, /* sigprocmask */
    syscall_stub, /* create_module */
    syscall_stub, /* init_module */
    syscall_stub, /* delete_module */
    syscall_stub, /* get_kernel_syms */
    syscall_stub, /* quotactl */
    syscall_stub, /* getpgid */
    syscall_stub, /* fchdir */
    syscall_stub, /* bdflush */
    syscall_stub, /* sysfs */
    syscall_stub, /* personality */
    syscall_stub, /* afs_syscall */
    syscall_stub, /* setfsuid */
    syscall_stub, /* setfsgid */
    syscall_stub, /* _llseek */
    syscall_stub, /* getdents */
    syscall_stub, /* _newselect */
    syscall_stub, /* flock */
    syscall_stub, /* msync */
    syscall_stub, /* readv */
    syscall_stub, /* writev */
    syscall_stub, /* getsid */
    syscall_stub, /* fdatasync */
    syscall_stub, /* _sysctl */
    syscall_stub, /* mlock */
    syscall_stub, /* munlock */
    syscall_stub, /* mlockall */
    syscall_stub, /* munlockall */
    syscall_stub, /* sched_setparam */
    syscall_stub, /* sched_getparam */
    syscall_stub, /* sched_setscheduler */
    syscall_stub, /* sched_getscheduler */
    syscall_sched_yield, /* sched_yield */
    syscall_stub, /* sched_get_priority_max */
    syscall_stub, /* sched_get_priority_min */
    syscall_stub, /* sched_rr_get_interval */
    syscall_stub, /* nanosleep */
    syscall_stub, /* mremap */
    syscall_stub, /* setresuid */
    syscall_stub, /* getresuid */
    syscall_stub, /* vm86 */
    syscall_stub, /* query_module */
    syscall_stub, /* poll */
    syscall_stub, /* nfsservctl */
    syscall_stub, /* setresgid */
    syscall_stub, /* getresgid */
    syscall_stub, /* prctl */
    syscall_stub, /* rt_sigreturn */
    syscall_stub, /* rt_sigaction */
    syscall_stub, /* rt_sigprocmask */
    syscall_stub, /* rt_sigpending */
    syscall_stub, /* rt_sigtimedwait */
    syscall_stub, /* rt_sigqueueinfo */
    syscall_stub, /* rt_sigsuspend */
    syscall_stub, /* pread64 */
    syscall_stub, /* pwrite64 */
    syscall_stub, /* chown */
    syscall_stub, /* getcwd */
    syscall_stub, /* capget */
    syscall_stub, /* capset */
    syscall_stub, /* sigaltstack */
    syscall_stub, /* sendfile */
    syscall_stub, /* getpmsg */
    syscall_stub, /* putpmsg */
    syscall_stub, /* vfork */
    syscall_stub, /* ugetrlimit */
    syscall_stub, /* mmap2 */
    syscall_stub, /* truncate64 */
    syscall_stub, /* ftruncate64 */
    syscall_stub, /* stat64 */
    syscall_stub, /* lstat64 */
    syscall_stub, /* fstat64 */
    syscall_stub, /* lchown32 */
    syscall_stub, /* getuid32 */
    syscall_stub, /* getgid32 */
    syscall_stub, /* geteuid32 */
    syscall_stub, /* getegid32 */
    syscall_stub, /* setreuid32 */
    syscall_stub, /* setregid32 */
    syscall_stub, /* getgroups32 */
    syscall_stub, /* setgroups32 */
    syscall_stub, /* fchown32 */
    syscall_stub, /* setresuid32 */
    syscall_stub, /* getresuid32 */
    syscall_stub, /* setresgid32 */
    syscall_stub, /* getresgid32 */
    syscall_stub, /* chown32 */
    syscall_stub, /* setuid32 */
    syscall_stub, /* setgid32 */
    syscall_stub, /* setfsuid32 */
    syscall_stub, /* setfsgid32 */
    syscall_stub, /* pivot_root */
    syscall_stub, /* mincore */
    syscall_stub, /* madvise */
    syscall_stub, /* getdents64 */
    syscall_stub, /* fcntl64 */
    syscall_stub, /* not */
    syscall_stub, /* not */
    syscall_stub, /* gettid */
    syscall_stub, /* readahead */
    syscall_stub, /* setxattr */
    syscall_stub, /* lsetxattr */
    syscall_stub, /* fsetxattr */
    syscall_stub, /* getxattr */
    syscall_stub, /* lgetxattr */
    syscall_stub, /* fgetxattr */
    syscall_stub, /* listxattr */
    syscall_stub, /* llistxattr */
    syscall_stub, /* flistxattr */
    syscall_stub, /* removexattr */
    syscall_stub, /* lremovexattr */
    syscall_stub, /* fremovexattr */
    syscall_stub, /* tkill */
    syscall_stub, /* sendfile64 */
    syscall_stub, /* futex */
    syscall_stub, /* sched_setaffinity */
    syscall_stub, /* sched_getaffinity */
    syscall_stub, /* set_thread_area */
    syscall_stub, /* get_thread_area */
    syscall_stub, /* io_setup */
    syscall_stub, /* io_destroy */
    syscall_stub, /* io_getevents */
    syscall_stub, /* io_submit */
    syscall_stub, /* io_cancel */
    syscall_stub, /* fadvise64 */
    syscall_stub, /* not */
    syscall_stub, /* exit_group */
    syscall_stub, /* lookup_dcookie */
    syscall_stub, /* epoll_create */
    syscall_stub, /* epoll_ctl */
    syscall_stub, /* epoll_wait */
    syscall_stub, /* remap_file_pages */
    syscall_stub, /* set_tid_address */
    syscall_stub, /* timer_create */
    syscall_stub, /* timer_settime */
    syscall_stub, /* timer_gettime */
    syscall_stub, /* timer_getoverrun */
    syscall_stub, /* timer_delete */
    syscall_stub, /* clock_settime */
    syscall_stub, /* clock_gettime */
    syscall_stub, /* clock_getres */
    syscall_stub, /* clock_nanosleep */
    syscall_stub, /* statfs64 */
    syscall_stub, /* fstatfs64 */
    syscall_stub, /* tgkill */
    syscall_stub, /* utimes */
    syscall_stub, /* fadvise64_64 */
    syscall_stub, /* vserver */
    syscall_stub, /* mbind */
    syscall_stub, /* get_mempolicy */
    syscall_stub, /* set_mempolicy */
    syscall_stub, /* mq_open */
    syscall_stub, /* mq_unlink */
    syscall_stub, /* mq_timedsend */
    syscall_stub, /* mq_timedreceive */
    syscall_stub, /* mq_notify */
    syscall_stub, /* mq_getsetattr */
    syscall_stub, /* kexec_load */
    syscall_stub, /* waitid */
    syscall_stub, /* not */
    syscall_stub, /* add_key */
    syscall_stub, /* request_key */
    syscall_stub, /* keyctl */
    syscall_stub, /* ioprio_set */
    syscall_stub, /* ioprio_get */
    syscall_stub, /* inotify_init */
    syscall_stub, /* inotify_add_watch */
    syscall_stub, /* inotify_rm_watch */
    syscall_stub, /* migrate_pages */
    syscall_stub, /* openat */
    syscall_stub, /* mkdirat */
    syscall_stub, /* mknodat */
    syscall_stub, /* fchownat */
    syscall_stub, /* futimesat */
    syscall_stub, /* fstatat64 */
    syscall_stub, /* unlinkat */
    syscall_stub, /* renameat */
    syscall_stub, /* linkat */
    syscall_stub, /* symlinkat */
    syscall_stub, /* readlinkat */
    syscall_stub, /* fchmodat */
    syscall_stub, /* faccessat */
    syscall_stub, /* pselect6 */
    syscall_stub, /* ppoll */
    syscall_stub, /* unshare */
    syscall_stub, /* set_robust_list */
    syscall_stub, /* get_robust_list */
    syscall_stub, /* splice */
    syscall_stub, /* sync_file_range */
    syscall_stub, /* tee */
    syscall_stub, /* vmsplice */
    syscall_stub, /* move_pages */
    syscall_stub, /* getcpu */
    syscall_stub, /* epoll_pwait */
    syscall_stub, /* utimensat */
    syscall_stub, /* signalfd */
    syscall_stub, /* timerfd_create */
    syscall_stub, /* eventfd */
    syscall_stub, /* fallocate */
    syscall_stub, /* timerfd_settime */
    syscall_stub, /* timerfd_gettime */
    syscall_stub, /* signalfd4 */
    syscall_stub, /* eventfd2 */
    syscall_stub, /* epoll_create1 */
    syscall_stub, /* dup3 */
    syscall_stub, /* pipe2 */
    syscall_stub, /* inotify_init1 */
    syscall_stub, /* preadv */
    syscall_stub, /* pwritev */
    syscall_stub, /* rt_tgsigqueueinfo */
    syscall_stub, /* perf_event_open */
    syscall_stub, /* recvmmsg */
    syscall_stub, /* fanotify_init */
    syscall_stub, /* fanotify_mark */
    syscall_stub, /* prlimit64 */
    syscall_stub, /* name_to_handle_at */
    syscall_stub, /* open_by_handle_at */
    syscall_stub, /* clock_adjtime */
    syscall_stub, /* syncfs */
    syscall_stub, /* sendmmsg */
    syscall_stub, /* setns */
    syscall_stub, /* process_vm_readv */
    syscall_stub, /* process_vm_writev */
    syscall_stub, /* kcmp */
    syscall_stub, /* finit_module */
    syscall_stub, /* sched_setattr */
    syscall_stub, /* sched_getattr */
    syscall_stub, /* renameat2 */
    syscall_stub, /* seccomp */
    syscall_stub, /* getrandom */
    syscall_stub, /* memfd_create */
    syscall_stub, /* bpf */
    syscall_stub, /* execveat */
    syscall_stub, /* socket */
    syscall_socketpair, /* socketpair */
    syscall_stub, /* bind */
    syscall_stub, /* connect */
    syscall_stub, /* listen */
    syscall_stub, /* accept4 */
    syscall_stub, /* getsockopt */
    syscall_stub, /* setsockopt */
    syscall_stub, /* getsockname */
    syscall_stub, /* getpeername */
    syscall_stub, /* sendto */
    syscall_stub, /* sendmsg */
    syscall_stub, /* recvfrom */
    syscall_stub, /* recvmsg */
    syscall_stub, /* shutdown */
    syscall_stub, /* userfaultfd */
    syscall_stub, /* membarrier */
    syscall_stub, /* mlock2 */
    syscall_stub, /* copy_file_range */
    syscall_stub, /* preadv2 */
    syscall_stub, /* pwritev2 */
    syscall_stub, /* pkey_mprotect */
    syscall_stub, /* pkey_alloc */
    syscall_stub, /* pkey_free */
    syscall_stub, /* statx */
    syscall_stub, /* arch_prctl */
};

int do_syscall(struct cpu_state *state)
{
	size_t idx = state->eax;

	if (!idx || idx > ARRAY_SIZE(_syscall_table))
		return -1;

	// syscall argument order: eax, ebx, ecx, edx, esi, edi, and ebp
	u32 res = _syscall_table[idx](state, state->ebx, state->ecx, state->edx, state->esi, state->edi, state->ebp);
	proc_set_syscall_ret(state, res);
	return 0;
}
