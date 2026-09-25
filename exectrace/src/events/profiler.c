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
typedef struct event_message event_message;

struct event_message {
	et_event event;
	event_message *next;
};

struct profiler_impl {
	et_profiler base;
	struct exec_trace_bpf *skel;
	struct perf_buffer *pb;
	const et_filter *filter;
	event_message *head;
	event_message *tail;
	int queue_error;
	volatile sig_atomic_t exiting;
};

static profiler_impl *active_profiler = NULL;

static void
sig_handler(int sig)
{
	if (active_profiler)
		active_profiler->exiting = 1;
}

static void
event_queue_push(profiler_impl *p, const et_event *event)
{
	event_message *message = lk_malloc(sizeof(*message));

	if (!message) {
		p->queue_error = -ENOMEM;
		return;
	}

	message->event = *event;
	message->next = NULL;

	if (p->tail)
		p->tail->next = message;
	else
		p->head = message;
	p->tail = message;
}

static event_message *
event_queue_pop(profiler_impl *p)
{
	event_message *message = p->head;

	if (!message)
		return NULL;

	p->head = message->next;
	if (!p->head)
		p->tail = NULL;
	return message;
}

static void
event_queue_clear(profiler_impl *p)
{
	event_message *message;

	while ((message = event_queue_pop(p)))
		lk_free(message);
}

static void
sample_cb(void *ctx, int cpu, void *data, __u32 size)
{
	profiler_impl *p = (profiler_impl *)ctx;
	const et_event *event = data;

	(void)cpu;

	if (size < sizeof(*event)) {
		lk_log_error("Invalid event size: %u (expected %lu)",
			     (unsigned int)size, (unsigned long)sizeof(*event));
		return;
	}

	if (!et_ft_check(p->filter, event))
		return;

	event_queue_push(p, event);
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

	event_queue_clear(p);
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

	p->skel = NULL;
	p->pb = NULL;
	p->filter = NULL;
	p->head = NULL;
	p->tail = NULL;
	p->queue_error = 0;
	p->exiting = 0;
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
	event_message *message;
	int cnt;

	p->filter = filter;

	while (!p->head && !p->exiting && !p->queue_error) {
		cnt = perf_buffer__poll(p->pb, 100);
		if (cnt < 0 && cnt != -EINTR) {
			lk_log_error("Error polling perf buffer: %d", cnt);
			return cnt;
		}
	}

	message = event_queue_pop(p);
	if (message) {
		*ev = message->event;
		lk_free(message);
		return 1;
	}

	if (p->queue_error) {
		lk_log_error("Failed to queue profiler event: %d",
			     p->queue_error);
		return p->queue_error;
	}
	return 0;
}
