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
	case ET_TYPE_START_EXEC:        return "START_EXEC";
	case ET_TYPE_BIN_PROG_CREATE:   return "BIN_PROG_CREATE";
	case ET_TYPE_TRY_ELF:           return "TRY_ELF";
	case ET_TYPE_TRY_ELF_RESULT:    return "TRY_ELF_RESULT";
	case ET_TYPE_TRY_SCRIPT:        return "TRY_SCRIPT";
	case ET_TYPE_TRY_SCRIPT_RESULT: return "TRY_SCRIPT_RESULT";
	case ET_TYPE_FILE_READ:         return "FILE_READ";
	default:                        return "UNKNOWN";
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

	if (ev->event_type == ET_TYPE_FILE_READ)
		lk_log_info("event=%-16s pid=%-7u filename=%s count=%-10llu offset=%lld",
			    event_type_str(ev->event_type), ev->pid,
			    ev->filename[0] ? ev->filename : "?", ev->count,
			    (long long)ev->offset);
	else {
		lk_status st = lk_status_create((lk_error_code)ev->status);
		lk_string msg = lk_status_msg(st);

		if (ev->filename[0])
			lk_log_info("event=%-16s pid=%-7u status=%-26s filename=%s",
				    event_type_str(ev->event_type), ev->pid,
				    lk_str_raw(msg), ev->filename);
		else
			lk_log_info("event=%-16s pid=%-7u status=%s",
				    event_type_str(ev->event_type), ev->pid,
				    lk_str_raw(msg));

		lk_status_decref(st);
	}
	return lk_status_create(LK_OK);
}
