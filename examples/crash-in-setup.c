#include <stdio.h>
#include <stdlib.h>

#include "scu.h"

SCU_MODULE("Crash in setup");

SCU_SETUP()
{
	printf("Buckle up, about to crash in setup\n");
	fflush(NULL);
	abort();
}

SCU_TEST(wont_be_run, "Wont be run")
{
}
