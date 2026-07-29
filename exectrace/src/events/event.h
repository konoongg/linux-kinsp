#ifndef ET_EVENT_H
#define ET_EVENT_H

#include <linux/types.h>

#include "object/lk_obj.h"

typedef union lk_status lk_status;

#define ET_FILENAME_LEN 256

typedef enum {
	ET_EVENT_TYPE_START_EXEC = 0,
} et_event_type;

typedef struct et_event {
	__u32 pid;
	et_event_type event_type;
	char  filename[ET_FILENAME_LEN];
} et_event;

typedef struct et_ev_handler {
	lk_obj obj;
} et_ev_handler;

extern lk_status et_ev_handler_new(et_ev_handler **out);

extern lk_status et_ev_handle(et_ev_handler *handler, const et_event *ev);

#endif
