#ifndef ET_EVENT_PROCESSOR_H
#define ET_EVENT_PROCESSOR_H

typedef struct et_filter et_filter;
typedef struct et_profiler et_profiler;
typedef struct et_ev_handler et_ev_handler;

extern int process_events(et_profiler *profiler, const et_filter *filter,
			  et_ev_handler *handler);

#endif
