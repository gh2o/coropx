#include "fibers.h"

typedef struct _FIOState FIOState;

FIOState *fio_state_new (Fiber *fiber);
void fio_state_free (FIOState *fs);

int fio_state_ready (FIOState *fs);
