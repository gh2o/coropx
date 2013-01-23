#pragma once

#include <apr_pools.h>

typedef struct _Fiber Fiber;
typedef struct _FiberManager FiberManager;

typedef void (*FiberCallback)(void *);

apr_pool_t *fiber_get_pool (Fiber *fiber);

FiberManager *fiber_manager_new ();
void fiber_manager_free ();
Fiber *fiber_manager_spawn (FiberManager *fm, size_t stacksize,
	FiberCallback callback, void *data);
void fiber_manager_run (FiberManager *fm);
