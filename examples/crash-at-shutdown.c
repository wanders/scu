#include <stdlib.h>

#include "scu.h"

static void
_boom(void)
{
	abort();
}

SCU_MODULE("Crash at shutdown");

SCU_TEST(set_up_bomb, "Set us up the bomb")
{
	atexit(_boom);
}
