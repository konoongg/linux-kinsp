#include "typedef_libc.h"

#include <bpf/libbpf.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "allocator/lk_allocator.h"
#include "logger/lk_logger.h"
#include "object/lk_obj.h"
#include "object/lk_status.h"

#include "exec_trace.skel.h"
#include "filter.h"
#include "profiler.h"

#define ET_PERF_BUFFER_PAGES 64

typedef struct profiler_impl profiler_impl;

struct profiler_impl {
	et_profiler base;
	struct exec_trace_bpf *skel;
	struct perf_buffer *pb;
	const et_filter *filter;
	et_event current;
	volatile sig_atomic_t exiting;
	bool event_ready;
};

static profiler_impl *active_profiler = NULL;

static void
sig_handler(int sig)
{
	if (active_profiler)
		active_profiler->exiting = 1;
}

static void
sample_cb(void *ctx, int cpu, void *data, __u32 size)
{
	profiler_impl *p = (profiler_impl *)ctx;
	et_event *ev = (et_event *)data;

	(void)cpu;
	(void)size;

	if (!et_ft_check(p->filter, ev))
		return;

	p->current = *ev;
	p->event_ready = true;
}

static void
lost_cb(void *ctx, int cpu, __u64 cnt)
{
	(void)ctx;
	lk_log_error("Lost %llu events on CPU #%d", cnt, cpu);
}

static void
profiler_destructor(void *self)
{
	profiler_impl *p = (profiler_impl *)self;

	if (active_profiler == p)
		active_profiler = NULL;

	if (p->pb)
		perf_buffer__free(p->pb);

	if (p->skel)
		exec_trace_bpf__destroy(p->skel);

	lk_free(p);
}

extern lk_status
profiler_new(et_profiler **out)
{
	profiler_impl *p = lk_malloc(sizeof(*p));
	lk_sigaction sa = { .sa_handler = sig_handler };
	int err;

	if (!p)
		return lk_status_create(LK_MEM_ALLOC_FAIL);

	LK_OBJ_INIT(&p->base, profiler_destructor);

	p->skel = exec_trace_bpf__open();
	if (!p->skel) {
		lk_log_error("Failed to open BPF skeleton");
		lk_obj_decref(&p->base);
		return lk_status_create(LK_OPEN_FAIL);
	}

	err = exec_trace_bpf__load(p->skel);
	if (err) {
		lk_log_error("Failed to load BPF program: %d", err);
		lk_obj_decref(&p->base);
		return lk_status_create(LK_INVALID_ARGUMENT);
	}

	err = exec_trace_bpf__attach(p->skel);
	if (err) {
		lk_log_error("Failed to attach BPF program: %d", err);
		lk_obj_decref(&p->base);
		return lk_status_create(LK_INVALID_ARGUMENT);
	}

	p->pb = perf_buffer__new(bpf_map__fd(p->skel->maps.events),
				 ET_PERF_BUFFER_PAGES,
				 sample_cb, lost_cb,
				 p, NULL);

	if (!p->pb) {
		lk_log_error("Failed to create perf buffer: %s",
			     strerror(errno));
		lk_obj_decref(&p->base);
		return lk_status_create(LK_MEM_ALLOC_FAIL);
	}

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	active_profiler = p;

	*out = &p->base;
	return lk_status_create(LK_OK);
}

extern int
get_event(et_profiler *profiler, const et_filter *filter, et_event *ev)
{
	profiler_impl *p = (profiler_impl *)profiler;
	int cnt;

	p->filter = filter;
	p->event_ready = false;

	while (!p->event_ready && !p->exiting) {
		cnt = perf_buffer__poll(p->pb, 100);
		if (cnt < 0 && cnt != -EINTR) {
			lk_log_error("Error polling perf buffer: %d", cnt);
			return cnt;
		}
	}

	if (p->event_ready) {
		*ev = p->current;
		return 1;
	}
	return 0;
}
