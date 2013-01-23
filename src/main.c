#include <inttypes.h>
#include <stdio.h>
#include <apr_general.h>
#include "globals.h"
#include "fibers.h"
#include "fio.h"

static void callback (void *data)
{
	printf ("%" PRIuPTR "\n", (uintptr_t) data);
}

int main (int argc, const char * const argv[], const char * const env[])
{
	apr_app_initialize (&argc, &argv, &env);
	apr_pool_create (&global_pool, NULL);

	FiberManager *manager = fiber_manager_new ();
	for (uintptr_t j = 0; j < 1024; j++)
		fiber_manager_spawn (manager, 16384, callback, (void *) j);
	fiber_manager_run (manager);
	fiber_manager_free (manager);

	apr_pool_destroy (global_pool);

	return 0;
}
