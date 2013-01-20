#include <setjmp.h>

enum stack_sjlj
{
	STACK_SJLJ_INITIAL = 0,
	STACK_SJLJ_STARTING,
	STACK_SJLJ_RUNNING,
	STACK_SJLJ_STOPPING,
};

void stack_setup (void *stackbase, size_t stacksize,
	jmp_buf env, void (*callback)(void *), void *data);
