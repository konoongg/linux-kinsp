#include "logger/lk_logger.h"
#include "object/lk_status.h"

#include "event.h"
#include "event_processor.h"
#include "filter.h"
#include "profiler.h"

extern int
process_events(et_profiler *profiler, const et_filter *filter,
	       et_ev_handler *handler)
{
	et_event ev;
	lk_status st;
	int err;

	while ((err = get_event(profiler, filter, &ev)) > 0) {
		st = et_ev_handle(handler, &ev);
		if (!lk_status_is_ok(st)) {
			lk_string msg = lk_status_msg(st);
			lk_log_error("Failed to handle event: %s", lk_str_raw(msg));
			lk_status_decref(st);
			return -1;
		}
	}

	return err;
}
