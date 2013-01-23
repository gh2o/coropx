#include "fibers.h"
#include "fio.h"

struct _FIOState
{
	Fiber *fiber;
};

FIOState *fio_state_new (Fiber *fiber)
{
	FIOState *st = apr_palloc (fiber_get_pool (fiber), sizeof (FIOState));
	st->fiber = fiber;
	return st;
}

void fio_state_free (FIOState *st)
{
	// freed by fiber's pool
}

int fio_state_ready (FIOState *st)
{
	return 1;
}
