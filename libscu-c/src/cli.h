#ifndef _CLI_H_
#define _CLI_H_

#include <stdbool.h>
#include <stdlib.h>

#include "config.h"

typedef struct {
	bool list;
	bool run;
	size_t num_tests;
	long int test_indices[_SCU_MAX_TESTS];
} _scu_arguments;

_scu_arguments get_arguments(int argc, char *argv[], size_t module_num_tests);

#endif
