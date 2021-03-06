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

// override mingw's stack checking
void __chkstk_ms () { return; }

static int one = -1;

static void keep_variable (void *base)
{
	if (one < 0)
	{
		union {
			jmp_buf env;
			void *dummy;
		} un;
		un.dummy = base;
		one = setjmp (un.env) + 1;
	}
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

static void stack_wrapper (stack_magic_callback callback,
	void *data, size_t dummysize,
	jmp_buf parent, jmp_buf child)
{
	char dummy[dummysize];
	keep_variable (dummy);

	stack_magic_switch (child, parent, STACK_SJLJ_STARTING);

	callback (data, parent, child);

	// returning will kill us all
	while (1)
		stack_magic_switch (child, parent, STACK_SJLJ_STOPPING);
}

static void stack_wrapper_wrong (stack_magic_callback callback,
	void *data, size_t dummysize,
	jmp_buf parent, jmp_buf child)
{
}

void stack_magic_setup (void *stackbase, size_t stacksize,
	jmp_buf parent, jmp_buf child,
	stack_magic_callback callback, void *data)
{
	keep_variable (detect_stack_direction);
	keep_variable (detect_stack_direction_wrong);
	keep_variable (stack_wrapper);
	keep_variable (stack_wrapper_wrong);

	enum stack_direction (*dsdfunc)(void *) = one ?
		detect_stack_direction : detect_stack_direction_wrong;
	void (*swfunc)(stack_magic_callback, void *, size_t, jmp_buf, jmp_buf) = one ?
		stack_wrapper : stack_wrapper_wrong;

	uintptr_t target = (uintptr_t) stackbase;
	uintptr_t source = (uintptr_t) &target;

	size_t dummysize;
	switch (dsdfunc (&dummysize))
	{
		case STACK_DIRECTION_DOWN:
			dummysize = source - (target + stacksize) + STACK_SAFETY_MARGIN;
			break;
		case STACK_DIRECTION_UP:
			dummysize = target - source + STACK_SAFETY_MARGIN;
			break;
		default:
			STACK_MAGIC_ABORT ("stack direction unknown!");
			break;
	}

	if (setjmp (parent) == STACK_SJLJ_INITIAL)
		swfunc (callback, data, dummysize, parent, child);
}

int stack_magic_switch (jmp_buf source, jmp_buf target, int num)
{
	int res = setjmp (source);
	if (res == STACK_SJLJ_INITIAL)
		longjmp (target, num);
	return res;
}
