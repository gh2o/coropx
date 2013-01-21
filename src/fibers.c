#include <stdio.h>
#include <stdlib.h>
#include <apr_pools.h>
#include <apr_ring.h>
#include "globals.h"
#include "stackmagic.h"
#include "fibers.h"

enum FiberSJLJ
{
	FIBER_SJLJ_RETURNED = STACK_SJLJ_LENGTH + 1,
	FIBER_SJLJ_RUNNING,
};

APR_RING_HEAD (_FiberRingHead, _Fiber);
typedef struct _FiberRingHead FiberRingHead;

struct _Fiber
{
	APR_RING_ENTRY(_Fiber) link;

	FiberManager *manager;
	apr_pool_t *pool;

	void *stack_base;
	size_t stack_size;
	jmp_buf env;

	FiberCallback callback;
	void *callback_data;
};

struct _FiberManager
{
	FiberRingHead fibers;
	jmp_buf env;
};

static Fiber *fiber_new (FiberManager *manager, size_t stacksize,
	FiberCallback callback, void *data)
{
	apr_pool_t *pool;
	apr_pool_create (&pool, global_pool);

	Fiber *fiber = apr_palloc (pool, sizeof (Fiber));

	fiber->manager = manager;
	fiber->pool = pool;

	fiber->stack_base = apr_palloc (pool, stacksize);
	fiber->stack_size = stacksize;

	fiber->callback = callback;
	fiber->callback_data = data;

	return fiber;
}

static void fiber_free (Fiber *fiber)
{
	apr_pool_destroy (fiber->pool);
}

static void fiber_handler (void *vfiber, jmp_buf parent, jmp_buf child)
{
	Fiber *fiber = vfiber;
	fiber->callback (fiber->callback_data);
	stack_magic_switch (fiber->env, fiber->manager->env, FIBER_SJLJ_RETURNED);
	abort ();
}

static void fiber_prepare (Fiber *fiber)
{
	stack_magic_setup (
		fiber->stack_base,
		fiber->stack_size,
		fiber->manager->env,
		fiber->env,
		fiber_handler,
		fiber
	);
}

FiberManager *fiber_manager_new ()
{
	FiberManager *fm = malloc (sizeof (FiberManager));
	APR_RING_INIT (&(fm->fibers), _Fiber, link);
	return fm;
}

void fiber_manager_free (FiberManager *fm)
{
	Fiber *fiber;
	APR_RING_FOREACH (fiber, &(fm->fibers), _Fiber, link)
		fiber_free (fiber);
	free (fm);
}

Fiber *fiber_manager_spawn (FiberManager *fm, size_t stacksize,
	FiberCallback callback, void *data)
{
	Fiber *fiber = fiber_new (fm, stacksize, callback, data);
	fiber_prepare (fiber);
	APR_RING_INSERT_TAIL (&(fm->fibers), fiber, _Fiber, link);
	return fiber;
}

int fiber_manager_iterate (FiberManager *fm)
{
	if (APR_RING_EMPTY (&(fm->fibers), _Fiber, link))
		return 1;

	Fiber *fiber = APR_RING_FIRST (&(fm->fibers));
	APR_RING_REMOVE (fiber, link);

	int res = stack_magic_switch (fm->env, fiber->env, FIBER_SJLJ_RUNNING);
	switch (res)
	{
		case FIBER_SJLJ_RUNNING:
			APR_RING_INSERT_TAIL (&(fm->fibers), fiber, _Fiber, link);
			break;
		case FIBER_SJLJ_RETURNED:
			break;
		default:
			fprintf (stderr, "unknown fiber sjlj %d!\n", res);
			break;
	}

	return 0;
}

void fiber_manager_run (FiberManager *fm)
{
	while (1)
		if (fiber_manager_iterate (fm))
			break;
}
