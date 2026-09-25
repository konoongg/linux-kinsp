#ifndef ET_EVENT_H
#define ET_EVENT_H

#include <linux/types.h>

#include "object/lk_status.h"

#define ET_FILENAME_LEN 256

typedef enum {
	ET_TYPE_START_EXEC = 0,
	ET_TYPE_BIN_PROG_CREATE,
	ET_TYPE_TRY_ELF,
	ET_TYPE_TRY_ELF_RESULT,
	ET_TYPE_TRY_SCRIPT,
	ET_TYPE_TRY_SCRIPT_RESULT,
	ET_TYPE_FILE_READ,
} et_event_type;

typedef struct et_event {
	__u32 pid;
	et_event_type event_type;
	lk_error_code status;
	__u64 count;
	__u64 offset;
	char  filename[ET_FILENAME_LEN];
} et_event;

typedef struct et_ev_handler {
	lk_obj obj;
} et_ev_handler;

extern lk_status et_ev_handler_new(et_ev_handler **out);

extern lk_status et_ev_handle(et_ev_handler *handler, const et_event *ev);

#endif