#include <stdio.h>

#include "allocator/lk_allocator.h"
#include "logger/lk_logger.h"
#include "object/lk_obj.h"
#include "object/lk_status.h"

#include "configure.h"
#include "events/event.h"
#include "events/event_processor.h"
#include "events/filter.h"
#include "events/profiler.h"

static lk_status
init_project(int argc, char **argv)
{
	lk_status st;

	st = lk_default_allocator();
	if (!lk_status_is_ok(st))
		return st;

	st = et_cfg_init(argc, argv);
	if (!lk_status_is_ok(st)) {
		lk_allocator_deinit();
		return st;
	}

	st = lk_determ_logger(et_cfg_output_path(), et_cfg_log_level());
	if (!lk_status_is_ok(st)) {
		et_cfg_deinit();
		lk_allocator_deinit();
		return st;
	}

	return lk_status_create(LK_OK);
}

static void
deinit_project(void)
{
	lk_logger_global_deinit();
	et_cfg_deinit();
	lk_allocator_deinit();
}

int main(int argc, char **argv)
{
	et_filter *filter = NULL;
	et_profiler *profiler = NULL;
	et_ev_handler *handler = NULL;
	lk_status st;
	int err;

	st = init_project(argc, argv);
	if (!lk_status_is_ok(st)) {
		lk_string msg = lk_status_msg(st);
		fprintf(stderr, "Failed to init project: %s\n", lk_str_raw(msg));
		lk_status_decref(st);
		return 1;
	}

	if (et_cfg_mode() == ET_MODE_HELP) {
		et_cfg_usage_help(argv[0]);
		deinit_project();
		return 0;
	}

	st = et_ev_handler_new(&handler);
	if (!lk_status_is_ok(st)) {
		lk_string msg = lk_status_msg(st);
		lk_log_error("Failed to create event handler: %s", lk_str_raw(msg));
		lk_status_decref(st);
		goto fail1;
	}

	st = et_ft_new(&filter);
	if (!lk_status_is_ok(st)) {
		lk_string msg = lk_status_msg(st);
		lk_log_error("Failed to create filter: %s", lk_str_raw(msg));
		lk_status_decref(st);
		goto fail2;
	}

	st = profiler_new(&profiler);
	if (!lk_status_is_ok(st)) {
		lk_string msg = lk_status_msg(st);
		lk_log_error("Failed to create profiler: %s", lk_str_raw(msg));
		lk_status_decref(st);
		goto fail3;
	}

	if (et_cfg_target_pid())
		lk_log_info("Tracing exec calls for PID %d...", et_cfg_target_pid());
	else
		lk_log_info("Tracing exec calls...");

	lk_log_info("Logging to %s. Press Ctrl+C to stop.", et_cfg_output_path());

	err = process_events(profiler, filter, handler);

	lk_log_info("Stopping...");

	lk_obj_decref(profiler);
	lk_obj_decref(filter);
	lk_obj_decref(handler);
	deinit_project();
	return err < 0 ? 1 : 0;

fail3:
	lk_obj_decref(filter);
fail2:
	lk_obj_decref(handler);
fail1:
	deinit_project();
	return 1;
}
