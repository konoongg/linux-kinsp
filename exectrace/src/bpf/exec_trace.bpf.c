#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>

#include "events/event.h"

struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
	__uint(key_size, sizeof(__u32));
	__uint(value_size, sizeof(__u32));
} events SEC(".maps");

struct qstr {
	const unsigned char *name;
} __attribute__((preserve_access_index));

struct dentry {
	struct qstr d_name;
} __attribute__((preserve_access_index));

struct path {
	struct dentry *dentry;
} __attribute__((preserve_access_index));

struct file {
	struct path f_path;
} __attribute__((preserve_access_index));

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

static __always_inline lk_error_code
sys_ret_to_status(long rc)
{
	if (!rc)
		return LK_MATCH;

	switch (-rc) {
	case 8:   /* ENOEXEC - binary format does not match */
		return LK_UNMATCH;
	case 12:  /* ENOMEM */
		return LK_MEM_ALLOC_FAIL;
	case 22:  /* EINVAL */
		return LK_INVALID_ARGUMENT;
	case 1:   /* EPERM */
	case 2:   /* ENOENT */
	case 13:  /* EACCES */
	case 14:  /* EFAULT */
	case 20:  /* ENOTDIR */
		return LK_OPEN_FAIL;
	case 5:   /* EIO */
	case 6:   /* ENXIO */
		return LK_READ_FAIL;
	case 4:   /* EINTR */
	case 7:   /* E2BIG */
	case 28:  /* ENOSPC */
		return LK_WRITE_FAIL;
	default:
		return LK_UNDEFINED_ERROR;
	}
}

static __always_inline void
capture_file_name(struct pt_regs *ctx, et_event *event)
{
	const struct file *file = (const struct file *)ctx->di;
	const struct dentry *dentry = NULL;
	const unsigned char *name = NULL;

	if (bpf_core_read(&dentry, sizeof(dentry), &file->f_path.dentry))
		return;
	if (!dentry)
		return;
	if (bpf_core_read(&name, sizeof(name), &dentry->d_name.name))
		return;

	if (name)
		bpf_probe_read_kernel_str(event->filename,
					  sizeof(event->filename), name);
}

static __always_inline int
trace_binfmt_enter(struct pt_regs *ctx, et_event_type type)
{
	et_event event = {};

	event.pid = bpf_get_current_pid_tgid() >> 32;
	event.event_type = type;

	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			      &event, sizeof(event));
	return 0;
}

static __always_inline int
trace_binfmt_ret(struct pt_regs *ctx, et_event_type type)
{
	et_event event = {};

	event.pid = bpf_get_current_pid_tgid() >> 32;
	event.event_type = type;
	event.status = sys_ret_to_status(ctx->ax);

	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			      &event, sizeof(event));
	return 0;
}

static __always_inline int trace_exec_enter(struct pt_regs *ctx,
					    unsigned long filename_off)
{
	et_event event = {};

	event.pid = bpf_get_current_pid_tgid() >> 32;
	event.event_type = ET_TYPE_START_EXEC;

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

SEC("kretprobe/alloc_bprm")
int handle_alloc_bprm_ret(struct pt_regs *ctx)
{
	et_event event = {};

	event.pid = bpf_get_current_pid_tgid() >> 32;
	event.event_type = ET_TYPE_BIN_PROG_CREATE;
	event.status = ctx->ax ? LK_OK : LK_MEM_ALLOC_FAIL;

	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			      &event, sizeof(event));
	return 0;
}

SEC("kprobe/kernel_read")
int handle_kernel_read(struct pt_regs *ctx)
{
	et_event event = {};

	event.pid = bpf_get_current_pid_tgid() >> 32;
	event.event_type = ET_TYPE_FILE_READ;
	event.status = LK_OK;
	event.count = ctx->dx;
	if (ctx->cx)
		bpf_probe_read_kernel(&event.offset, sizeof(event.offset),
				      (const void *)ctx->cx);

	capture_file_name(ctx, &event);

	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			      &event, sizeof(event));
	return 0;
}

SEC("kprobe.multi/load_elf_binary")
int handle_load_elf_binary(struct pt_regs *ctx)
{
	return trace_binfmt_enter(ctx, ET_TYPE_TRY_ELF);
}

SEC("kretprobe.multi/load_elf_binary")
int handle_load_elf_binary_ret(struct pt_regs *ctx)
{
	return trace_binfmt_ret(ctx, ET_TYPE_TRY_ELF_RESULT);
}

SEC("kprobe.multi/load_script")
int handle_load_script(struct pt_regs *ctx)
{
	return trace_binfmt_enter(ctx, ET_TYPE_TRY_SCRIPT);
}

SEC("kretprobe.multi/load_script")
int handle_load_script_ret(struct pt_regs *ctx)
{
	return trace_binfmt_ret(ctx, ET_TYPE_TRY_SCRIPT_RESULT);
}

char _license[] SEC("license") = "GPL";