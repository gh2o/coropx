#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "stackmagic.h"

static void func (void *data)
{
	int i;
	printf ("%016lx\n", (uintptr_t) &i);
}

int main ()
{
	printf ("main starting ...\n");
	jmp_buf child;
	
	size_t stacksize = 65536;
	void *stackbase = malloc (stacksize);

	stack_setup (stackbase, stacksize, child, func, NULL);
	longjmp (child, 1);
}
