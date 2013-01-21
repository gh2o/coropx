#include <apr_general.h>
#include "globals.h"
#include "fibers.h"

int main (int argc, const char * const argv[], const char * const env[])
{
	apr_app_initialize (&argc, &argv, &env);
	apr_pool_create (&global_pool, NULL);

	FiberManager *manager = fiber_manager_new ();
}
