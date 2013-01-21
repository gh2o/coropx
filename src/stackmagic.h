#include <stddef.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

enum stack_sjlj
{
	STACK_SJLJ_INITIAL = 0,
	// 1 may be ambiguous
	STACK_SJLJ_RUNNING = 2,
	STACK_SJLJ_STARTING,
	STACK_SJLJ_STOPPING,
	STACK_SJLJ_LENGTH,
};

typedef void (*stack_magic_callback)(void *, jmp_buf, jmp_buf);

void stack_magic_setup (void *stackbase, size_t stacksize,
	jmp_buf parent, jmp_buf child,
	stack_magic_callback callback, void *data);
int stack_magic_switch (jmp_buf source, jmp_buf target, int num);

#ifdef __cplusplus
}
#endif
