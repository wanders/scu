#include <stdbool.h>
#include <stdlib.h>

#include "scu.h"

SCU_MODULE("Crash");

SCU_TEST(before_crash, "This test will get executed")
{
	SCU_ASSERT(true);
}

SCU_TEST(crash, "Test that should crash")
{
	printf("Calling `abort`\n");
	abort();
	SCU_ASSERT(true);
}

SCU_TEST(after_crash, "This test will not get executed")
{
	SCU_ASSERT(true);
}
