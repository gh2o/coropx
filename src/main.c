#include <stdio.h>
#include <apr_general.h>
#include "globals.h"
#include "fibers.h"

static void callback (void *data)
{
	printf ("%d\n", * (int *) data);
}

int main (int argc, const char * const argv[], const char * const env[])
{
	int i = 157;

	apr_app_initialize (&argc, &argv, &env);
	apr_pool_create (&global_pool, NULL);

	FiberManager *manager = fiber_manager_new ();
	for (int j = 0; j < 1024; j++)
		fiber_manager_spawn (manager, 16384, callback, &i);
	fiber_manager_run (manager);
	fiber_manager_free (manager);

	return 0;
}
