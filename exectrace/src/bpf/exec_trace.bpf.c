#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>

#include "events/event.h"

struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
	__uint(key_size, sizeof(__u32));
	__uint(value_size, sizeof(__u32));
} events SEC(".maps");

struct pt_regs {
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long bp;
	unsigned long bx;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long ax;
	unsigned long cx;
	unsigned long dx;
	unsigned long si;
	unsigned long di;
} __attribute__((preserve_access_index));

static __always_inline int trace_exec_enter(struct pt_regs *ctx,
					    unsigned long filename_off)
{
	et_event event = {};

	event.pid = bpf_get_current_pid_tgid() >> 32;
	event.event_type = ET_EVENT_TYPE_START_EXEC;

	const struct pt_regs *syscall_regs;
	syscall_regs = (const struct pt_regs *)ctx->di;

	const char *filename_ptr = NULL;
	bpf_probe_read_kernel(&filename_ptr, sizeof(filename_ptr),
			      (const char *)syscall_regs + filename_off);

	if (filename_ptr)
		bpf_probe_read_user_str(event.filename,
					sizeof(event.filename),
					filename_ptr);

	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			      &event, sizeof(event));
	return 0;
}

SEC("kprobe/__x64_sys_execve")
int handle_execve(struct pt_regs *ctx)
{
	return trace_exec_enter(ctx,
				offsetof(struct pt_regs, di));
}

SEC("kprobe/__x64_sys_execveat")
int handle_execveat(struct pt_regs *ctx)
{
	return trace_exec_enter(ctx,
				offsetof(struct pt_regs, si));
}

char _license[] SEC("license") = "GPL";
