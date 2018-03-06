#include <argp.h>

#include "cli.h"

#define SCU_DOCUMENTATION "SCU test module\vExamples:\n  ./test --list\n  ./test --run 0 1 2"

typedef struct {
  size_t module_num_tests;
	_scu_arguments args;
} argp_input;

static struct argp_option options[] = {
    {"list", 'l', 0, 0, "list available test cases"},
    {"run", 'r', 0, 0, "run the test cases identified by the supplied indices"},
    {0}};

static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
  argp_input *input = (argp_input *)state->input;
	_scu_arguments *parsed_args = &input->args;

	switch (key) {
		case 'l':
			parsed_args->list = true;
			break;
		case 'r':
			parsed_args->run = true;
			break;
		case ARGP_KEY_NO_ARGS:
			if (!parsed_args->list && !parsed_args->run)
				argp_usage(state);
			if (parsed_args->run)
				argp_error(state, "not enough arguments");
			break;
		case ARGP_KEY_ARG: {
			errno = 0;
			char *endptr = NULL;
			long int idx = strtol(arg, &endptr, 10);
			if (endptr == arg || errno != 0 || idx < 0 || idx >= input->module_num_tests)
				argp_error(state, "invalid index: %s", arg);
			parsed_args->test_indices[parsed_args->num_tests++] = idx;
			break;
		}
		case ARGP_KEY_END:
			if (!parsed_args->run && parsed_args->num_tests > 0)
				argp_error(state, "extraneous arguments");
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = {options, parse_opt, 0, SCU_DOCUMENTATION};

_scu_arguments
get_arguments(int argc, char *argv[], size_t module_num_tests)
{
	argp_input input = {.module_num_tests = module_num_tests};

	argp_parse(&argp, argc, argv, 0, 0, &input);

  return input.args;
}
