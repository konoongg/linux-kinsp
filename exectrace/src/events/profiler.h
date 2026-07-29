#ifndef ET_PROFILER_H
#define ET_PROFILER_H

#include "object/lk_obj.h"
#include "object/lk_status.h"

#include "event.h"

typedef struct et_filter et_filter;

typedef struct et_profiler {
	lk_obj obj;
} et_profiler;

extern lk_status profiler_new(et_profiler **out);
extern int get_event(et_profiler *profiler, const et_filter *filter, et_event *ev);

#endif
