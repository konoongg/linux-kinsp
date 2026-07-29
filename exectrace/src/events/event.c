#include "allocator/lk_allocator.h"
#include "logger/lk_logger.h"
#include "object/lk_obj.h"
#include "object/lk_status.h"

#include "event.h"

typedef struct et_ev_handler_impl et_ev_handler_impl;

struct et_ev_handler_impl {
	et_ev_handler base;
};

static void
et_ev_handler_destructor(void *self)
{
	lk_free(self);
}

static const char *
event_type_str(et_event_type type)
{
	switch (type) {
	case ET_EVENT_TYPE_START_EXEC: return "START_EXEC";
	default:                       return "UNKNOWN";
	}
}

extern lk_status
et_ev_handler_new(et_ev_handler **out)
{
	et_ev_handler_impl *handler = lk_malloc(sizeof(*handler));
	if (!handler)
		return lk_status_create(LK_MEM_ALLOC_FAIL);

	LK_OBJ_INIT(&handler->base, et_ev_handler_destructor);

	*out = &handler->base;
	return lk_status_create(LK_OK);
}

extern lk_status
et_ev_handle(et_ev_handler *handler, const et_event *ev)
{
	(void)handler;

	lk_log_info("event=%-12s pid=%-7u filename=%s",
		    event_type_str(ev->event_type), ev->pid, ev->filename);

	return lk_status_create(LK_OK);
}
