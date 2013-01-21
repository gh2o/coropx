#include <apr_ring.h>
#include "globals.h"
#include "fibers.h"

APR_RING_HEAD (_FiberRingHead, _Fiber);
typedef struct _FiberRingHead FiberRingHead;

struct _Fiber
{
	APR_RING_ENTRY(_Fiber) link;
};

struct _FiberManager
{
	FiberRingHead fibers;
};

FiberManager *fiber_manager_new ()
{
	FiberManager *fm = apr_palloc (global_pool, sizeof (FiberManager));
	APR_RING_INIT (&(fm->fibers), _Fiber, link);
	return fm;
}
