#ifndef ET_FILTER_H
#define ET_FILTER_H

#include <stdbool.h>

#include "object/lk_obj.h"
#include "object/lk_status.h"

typedef struct et_event et_event;

typedef struct et_filter {
	lk_obj obj;
} et_filter;

extern lk_status et_ft_new(et_filter **out);

extern bool et_ft_check(const et_filter *filter, const et_event *ev);

#endif
