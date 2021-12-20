#include <stdio.h>
#include <stdlib.h>

#include "scu.h"

SCU_MODULE("Crash in teardown");

SCU_TEARDOWN()
{
	printf("Buckle up, about to crash in teardown\n");
	fflush(NULL);
	abort();
}

SCU_TEST(uninteresting, "Not a very interesting test")
{
}
