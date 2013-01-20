#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "stackmagic.h"

#ifdef STACK_MAGIC_DEBUG
#include <stdio.h>
#define STACK_MAGIC_ABORT(reason) \
	do { fprintf (stderr, reason " aborting ... \n"); abort (); } while (0);
#else
#define STACK_MAGIC_ABORT(reason) do { abort (); } while (0);
#endif

#ifndef STACK_SAFETY_MARGIN
#define STACK_SAFETY_MARGIN (16)
#endif

void __chkstk_ms () { return; }

static int keep_variable (void *base, size_t size)
{
	jmp_buf env;

	uint8_t *bbase = base;
	uint8_t *benv = (uint8_t *) env;

	benv[0] = bbase[0];
	benv[1] = bbase[size - 1];

	return setjmp (env);
}

enum stack_direction
{
	STACK_DIRECTION_UNSET,
	STACK_DIRECTION_UNKNOWN,
	STACK_DIRECTION_DOWN,
	STACK_DIRECTION_UP,
};

static enum stack_direction stack_direction_cached = STACK_DIRECTION_UNSET;

static enum stack_direction detect_stack_direction (void *before)
{
	if (stack_direction_cached == STACK_DIRECTION_UNSET)
	{
		int tmp;
		void *after = &tmp;

		if (after < before)
			stack_direction_cached = STACK_DIRECTION_DOWN;
		else if (after > before)
			stack_direction_cached = STACK_DIRECTION_UP;
		else
			stack_direction_cached =  STACK_DIRECTION_UNKNOWN;
	}

	return stack_direction_cached;
}

static enum stack_direction detect_stack_direction_wrong (void *before)
{
	return STACK_DIRECTION_UNKNOWN;
}

static void stack_wrapper (void (*callback)(void *), void *data, size_t dummysize,
		jmp_buf parent, jmp_buf child)
{
	char dummy[dummysize];
	keep_variable (dummy, dummysize);

	if (setjmp (child) == STACK_SJLJ_INITIAL)
		longjmp (parent, STACK_SJLJ_STARTING);

	callback (data);

	STACK_MAGIC_ABORT ("stack callback returned!");
}

void stack_setup (void *stackbase, size_t stacksize,
	jmp_buf env, void (*callback)(void *), void *data)
{
	uintptr_t target = (uintptr_t) stackbase;
	uintptr_t source = (uintptr_t) &target;

	size_t dummysize;
	enum stack_direction (*dsdfunc)(void *) = keep_variable (env, sizeof (jmp_buf)) ?
		detect_stack_direction_wrong : detect_stack_direction;

	switch (dsdfunc (&dummysize))
	{
		case STACK_DIRECTION_DOWN:
			dummysize = source - (target + stacksize) - STACK_SAFETY_MARGIN;
			break;
		case STACK_DIRECTION_UP:
			dummysize = target - source + STACK_SAFETY_MARGIN;
			break;
		default:
			STACK_MAGIC_ABORT ("stack direction unknown!");
			break;
	}

	jmp_buf tmp;
	if (setjmp (tmp) == STACK_SJLJ_INITIAL)
		stack_wrapper (callback, data, dummysize, tmp, env);
}
