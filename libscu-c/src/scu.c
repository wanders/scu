#include <argp.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "json.h"
#include "scu.h"

#define SCU_DOCUMENTATION "SCU test module\vExamples:\n  ./test --list\n  ./test --run 0 1 2"

#define SCU_OUTPUT_FILENAME_TEMPLATE "/tmp/scu.XXXXXX"
#define SCU_OUTPUT_FILENAME_TEMPLATE_SIZE (strlen(SCU_OUTPUT_FILENAME_TEMPLATE) + 1)

/* Optional hook functions */

__attribute__((weak)) void
_scu_setup(void)
{
}

__attribute__((weak)) void
_scu_teardown(void)
{
}

__attribute__((weak)) void
_scu_before_each(void)
{
}

__attribute__((weak)) void
_scu_after_each(void)
{
}

/* Test module globals */

extern const char *_scu_module_name;
static _scu_testcase **_scu_module_tests;
static size_t _scu_module_num_tests;

/* Test protocol functions */

static int _scu_cmd_fd;

static void
_scu_flush_json()
{
	write(_scu_cmd_fd, "\n", 1);
}

static void
_scu_output_module_list(const char *modulename)
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "module_list");
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "name");
	json_string(_scu_cmd_fd, modulename);
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}

static void
_scu_output_test_list(int line, const char *name, const char *description, const char *tags[])
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "testcase_list");
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "line");
	json_integer(_scu_cmd_fd, line);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "name");
	json_string(_scu_cmd_fd, name);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "description");
	json_string(_scu_cmd_fd, description);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "tags");
	json_array_start(_scu_cmd_fd);
	if (*tags) {
		json_string(_scu_cmd_fd, tags[0]);
		for (size_t i = 1; tags[i] && i < _SCU_MAX_TAGS; i++) {
			json_separator(_scu_cmd_fd);
			json_string(_scu_cmd_fd, tags[i]);
		}
	}
	json_array_end(_scu_cmd_fd);
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}

static void
_scu_output_setup_start(const char *filename)
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "setup_start");
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "output");
	json_string(_scu_cmd_fd, filename);
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}
static void
_scu_output_setup_end()
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "setup_end");
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}

static void
_scu_output_teardown_start(const char *filename)
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "teardown_start");
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "output");
	json_string(_scu_cmd_fd, filename);
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}
static void
_scu_output_teardown_end()
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "teardown_end");
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}

static void
_scu_output_test_start(int idx, const char *name, const char *filename)
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "testcase_start");
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "index");
	json_integer(_scu_cmd_fd, idx);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "name");
	json_string(_scu_cmd_fd, name);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "output");
	json_string(_scu_cmd_fd, filename);
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}

static void
_scu_output_test_failure(_scu_failure *failure)
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "file");
	json_string(_scu_cmd_fd, failure->file);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "line");
	json_integer(_scu_cmd_fd, failure->line);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "message");
	json_string(_scu_cmd_fd, failure->msg);
	json_object_end(_scu_cmd_fd);
}

static void
_scu_output_test_failures(size_t num, _scu_failure *failures)
{
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "failures");
	json_array_start(_scu_cmd_fd);
	if (num > 0) {
		_scu_output_test_failure(&failures[0]);
		for (size_t i = 1; i < num; i++) {
			json_separator(_scu_cmd_fd);
			_scu_output_test_failure(&failures[i]);
		}
	}
	json_array_end(_scu_cmd_fd);
}

static void
_scu_output_test_end(int idx, bool success, size_t asserts,
                     double mono_time, double cpu_time,
                     size_t num_failures, _scu_failure *failures)
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "testcase_end");
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "index");
	json_integer(_scu_cmd_fd, idx);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "success");
	json_boolean(_scu_cmd_fd, success);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "asserts");
	json_integer(_scu_cmd_fd, asserts);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "duration");
	json_real(_scu_cmd_fd, mono_time);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "cpu_time");
	json_real(_scu_cmd_fd, cpu_time);
	_scu_output_test_failures(num_failures, failures);
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}

static void
_scu_output_test_error(const char *file, int line, const char *msg)
{
	json_object_start(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "event");
	json_string(_scu_cmd_fd, "testcase_error");
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "message");
	json_string(_scu_cmd_fd, msg);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "file");
	json_string(_scu_cmd_fd, file);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "line");
	json_integer(_scu_cmd_fd, line);
	json_separator(_scu_cmd_fd);
	json_object_key(_scu_cmd_fd, "crash");
	json_true(_scu_cmd_fd);
	json_object_end(_scu_cmd_fd);
	_scu_flush_json(_scu_cmd_fd);
}

static void
_scu_redirect_output(char *filename, size_t len)
{
	strncpy(filename, SCU_OUTPUT_FILENAME_TEMPLATE, len);
	int out = mkstemp(filename);
	assert(out >= 0);
	dup2(out, STDOUT_FILENO);
	dup2(out, STDERR_FILENO);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

/* Module actions */

static void
list_tests(void)
{
	_scu_cmd_fd = STDOUT_FILENO;

	_scu_output_module_list(_scu_module_name);

	for (size_t i = 0; i < _scu_module_num_tests; i++) {
		_scu_testcase *test = _scu_module_tests[i];
		_scu_output_test_list(test->line, test->name, test->desc, test->tags);
	}
}

static double
_scu_get_time_diff(struct timespec startt, struct timespec endt)
{
	time_t dsec = endt.tv_sec - startt.tv_sec;
	long dnsec = endt.tv_nsec - startt.tv_nsec;

	if (dnsec < 0) {
		dnsec += 1000000000;
		dsec--;
	}

	return dsec + dnsec / 1e9;
}

static bool _scu_fatal_assert_jmpbuf_valid;
static pid_t _scu_fatal_assert_allowed_thread_id;
static jmp_buf _scu_fatal_assert_jmpbuf;

static pid_t
_scu_get_current_thread_id(void)
{
	return syscall(SYS_gettid);
}

void
_scu_fatal_assert_allowed(const char *file, int line)
{
	assert(_scu_fatal_assert_jmpbuf_valid);
	if (_scu_get_current_thread_id() != _scu_fatal_assert_allowed_thread_id) {
		_scu_output_test_error(file, line, "Attempt to use fatal assert outside main thread");
		abort();
	}
}

void
_scu_handle_fatal_assert(void)
{
	longjmp(_scu_fatal_assert_jmpbuf, 1);
}

static _scu_failure _failures[_SCU_MAX_FAILURES];

static void
_scu_run_test(int idx)
{
	_scu_testcase *test = _scu_module_tests[idx];

	char filename[SCU_OUTPUT_FILENAME_TEMPLATE_SIZE];
	_scu_redirect_output(filename, sizeof(filename));

	_scu_output_test_start(idx, test->name, filename);

	struct timespec start_mono_time, end_mono_time, start_cpu_time, end_cpu_time;

	_scu_before_each();

	clock_gettime(CLOCK_MONOTONIC, &start_mono_time);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_cpu_time);

	bool success = true;
	size_t asserts = 0, num_failures = 0;

	_scu_fatal_assert_jmpbuf_valid = true;
	_scu_fatal_assert_allowed_thread_id = _scu_get_current_thread_id();
	if (!setjmp(_scu_fatal_assert_jmpbuf)) {
		test->func(&success, &asserts, &num_failures, _failures);
	}
	_scu_fatal_assert_allowed_thread_id = 0;
	_scu_fatal_assert_jmpbuf_valid = false;

	clock_gettime(CLOCK_MONOTONIC, &end_mono_time);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu_time);

	_scu_after_each();

	_scu_output_test_end(idx, success, asserts,
	                     _scu_get_time_diff(start_mono_time, end_mono_time),
	                     _scu_get_time_diff(start_cpu_time, end_cpu_time),
	                     num_failures, _failures);
}

static void
run_tests(size_t num_tests, long int test_indices[])
{
	_scu_cmd_fd = dup(STDOUT_FILENO);

	char filename[SCU_OUTPUT_FILENAME_TEMPLATE_SIZE];

	_scu_redirect_output(filename, sizeof(filename));

	_scu_output_setup_start(filename);

	_scu_setup();

	_scu_output_setup_end();

	for (size_t i = 0; i < num_tests; i++) {
		_scu_run_test(test_indices[i]);
	}

	_scu_redirect_output(filename, sizeof(filename));

	_scu_output_teardown_start(filename);

	_scu_teardown();

	_scu_output_teardown_end();
}

/* Argument parsing */

typedef struct {
	bool list;
	bool run;
	size_t num_tests;
	long int test_indices[_SCU_MAX_TESTS];
} _scu_arguments;

static struct argp_option options[] = {
    {"list", 'l', 0, 0, "list available test cases"},
    {"run", 'r', 0, 0, "run the test cases identified by the supplied indices"},
    {0}};

static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	_scu_arguments *parsed_args = state->input;

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
			if (endptr == arg || errno != 0 || idx < 0 || idx >= _scu_module_num_tests)
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

/* Main function */

static int
_scu_line_comparator(const void *a, const void *b)
{
	const _scu_testcase *const *ta = a;
	const _scu_testcase *const *tb = b;
	return (*ta)->line - (*tb)->line;
}

void
_scu_register_testcase(_scu_testcase *tc)
{
	if (((_scu_module_num_tests + 1) & _scu_module_num_tests) == 0) {
		_scu_module_tests = realloc(_scu_module_tests, sizeof(_scu_testcase *) * (_scu_module_num_tests + 1) * 2);
	}
	_scu_module_tests[_scu_module_num_tests] = tc;
	_scu_module_num_tests++;
}

int
main(int argc, char *argv[])
{
	_scu_arguments args = {0};

	argp_parse(&argp, argc, argv, 0, 0, &args);

	qsort(_scu_module_tests, _scu_module_num_tests, sizeof(_scu_testcase *), _scu_line_comparator);

	if (args.list) {
		list_tests();
	} else if (args.run) {
		run_tests(args.num_tests, args.test_indices);
	}

	return 0;
}
