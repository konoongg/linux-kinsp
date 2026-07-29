#include "allocator/lk_allocator.h"
#include "object/lk_obj.h"
#include "object/lk_status.h"

#include "configure.h"
#include "event.h"
#include "filter.h"

typedef struct filter_impl filter_impl;

struct filter_impl {
	et_filter base;
	int target_pid;
};

static void
filter_destructor(void *self)
{
	lk_free(self);
}

extern lk_status
et_ft_new(et_filter **out)
{
	filter_impl *filter = lk_malloc(sizeof(*filter));
	if (!filter)
		return lk_status_create(LK_MEM_ALLOC_FAIL);

	LK_OBJ_INIT(&filter->base, filter_destructor);
	filter->target_pid = et_cfg_target_pid();

	*out = &filter->base;
	return lk_status_create(LK_OK);
}

extern bool
et_ft_check(const et_filter *filter, const et_event *ev)
{
	const filter_impl *impl = (const filter_impl *)filter;

	if (impl->target_pid && (int)ev->pid != impl->target_pid)
		return false;
	return true;
}
