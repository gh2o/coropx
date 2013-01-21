#include <stdio.h>
#include <apr_general.h>
#include "globals.h"
#include "fibers.h"

static void callback (void *data)
{
	printf ("callback!\n");
}

int main (int argc, const char * const argv[], const char * const env[])
{
	apr_app_initialize (&argc, &argv, &env);
	apr_pool_create (&global_pool, NULL);

	FiberManager *manager = fiber_manager_new ();
	fiber_manager_spawn (manager, 16384, callback, NULL);
	fiber_manager_run (manager);

	return 0;
}
