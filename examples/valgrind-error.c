/*
 * This file intentionally contains valgrind errors.
 *
 * Unfortunately they are also undefined behaviour in C, so compiler
 * might do something funny. But hopefully it still triggers valgrind
 * error so they can serve as an example of how output looks when
 * there is valgrind error.
 *
 */

#include <stdlib.h>

#include "scu.h"

SCU_MODULE("valgrind_error");

SCU_SETUP()
{
}

SCU_TEARDOWN()
{
}

SCU_TEST(uninitialized, "Access of uninitialized memory")
{
	char *b = malloc(10);

	if (b[8] == 17) {
		SCU_ASSERT(b[8] == 17);
	} else {
		SCU_ASSERT(b[8] != 17);
	}

	free(b);
}

SCU_TEST(outofbounds, "Out of bounds access")
{
	char *b = malloc(10);

	b[10] = 17;
	SCU_ASSERT(b[10] == 17);

	free(b);
}

SCU_TEST(useafterfree, "Use after free")
{
	char *b = malloc(10);

	free(b);
	b[1] = 17;
	SCU_ASSERT(b[1] == 17);
}

SCU_TEST(memory_leak, "Memory leak")
{
	char *b = malloc(10);

	b[1] = 17;
	SCU_ASSERT(b[1] == 17);
}
