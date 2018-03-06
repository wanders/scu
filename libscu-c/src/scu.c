#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "cli.h"
#include "protocol.h"
#include "scu_internal.h"
#include "util.h"

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
static size_t _scu_module_num_tests;
static _scu_testcase **_scu_module_tests;

static pid_t _scu_fatal_assert_allowed_thread_id;
static bool _scu_fatal_assert_jmpbuf_valid;
static jmp_buf _scu_fatal_assert_jmpbuf;

static bool _scu_success;
static size_t _scu_num_asserts;
static size_t _scu_num_failures;
static _scu_failure _scu_failures[_SCU_MAX_FAILURES];

/* Utility functions */

static void
redirect_output(char *filename, size_t len)
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
	int cmd = STDOUT_FILENO;

	_scu_output_module_list(cmd, _scu_module_name);

	for (size_t i = 0; i < _scu_module_num_tests; i++) {
		_scu_testcase *test = _scu_module_tests[i];
		_scu_output_test_list(cmd, test->line, test->name, test->desc, test->tags);
	}
}

static void
_scu_run_test(int fd, int idx)
{
	_scu_testcase *test = _scu_module_tests[idx];

	char filename[SCU_OUTPUT_FILENAME_TEMPLATE_SIZE];
	redirect_output(filename, sizeof(filename));

	_scu_output_test_start(fd, idx, test->name, filename);

	struct timespec start_mono_time, end_mono_time, start_cpu_time, end_cpu_time;

	_scu_before_each();

	clock_gettime(CLOCK_MONOTONIC, &start_mono_time);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_cpu_time);

	_scu_success = true;
	_scu_num_asserts = 0;
	_scu_num_failures = 0;

	_scu_fatal_assert_jmpbuf_valid = true;
	_scu_fatal_assert_allowed_thread_id = get_current_thread_id();
	if (!setjmp(_scu_fatal_assert_jmpbuf)) {
		test->func();
	}
	_scu_fatal_assert_allowed_thread_id = 0;
	_scu_fatal_assert_jmpbuf_valid = false;

	clock_gettime(CLOCK_MONOTONIC, &end_mono_time);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu_time);

	_scu_after_each();

	_scu_output_test_end(fd, idx, _scu_success, _scu_num_asserts,
	                     get_time_diff(start_mono_time, end_mono_time),
	                     get_time_diff(start_cpu_time, end_cpu_time),
	                     _scu_num_failures, _scu_failures);
}

static void
_scu_sigcont_handler(int sig)
{
}

static void
run_tests(size_t num_tests, long int test_indices[])
{
	int cmd = dup(STDOUT_FILENO);

	if (getenv("SCU_WAIT_FOR_DEBUGGER")) {
		/* We use an empty signal handler to wait for SIGCONT with pause() */
		signal(SIGCONT, _scu_sigcont_handler);
		pause();
	}

	_scu_output_module_start(cmd, _scu_module_name);

	char filename[SCU_OUTPUT_FILENAME_TEMPLATE_SIZE];

	redirect_output(filename, sizeof(filename));

	_scu_output_setup_start(cmd, filename);

	_scu_setup();

	_scu_output_setup_end(cmd);

	for (size_t i = 0; i < num_tests; i++) {
		_scu_run_test(cmd, test_indices[i]);
	}

	redirect_output(filename, sizeof(filename));

	_scu_output_teardown_start(cmd, filename);

	_scu_teardown();

	_scu_output_teardown_end(cmd);

	_scu_output_module_end(cmd);
}

/* Internal functions */

void
_scu_account_assert(bool is_fatal)
{
	if (is_fatal) {
		assert(_scu_fatal_assert_jmpbuf_valid);
		/* FATAL errors can only be used from same thread */
		assert(get_current_thread_id() == _scu_fatal_assert_allowed_thread_id);
	}
	_scu_num_asserts++;
}

void
_scu_handle_fatal_assert(void)
{
	longjmp(_scu_fatal_assert_jmpbuf, 1);
}

_scu_failure *
_scu_report_failure(const char *file, int line, const char *assert_method)
{
	_scu_success = false;

	if (_scu_num_failures >= _SCU_MAX_FAILURES)
		return NULL;

	_scu_failure *res = &_scu_failures[_scu_num_failures++];

	*res = (_scu_failure){
	    .file = file,
	    .line = line,
	    .assert_method = assert_method};

	return res;
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

/* Main function */

static int
_scu_line_comparator(const void *a, const void *b)
{
	const _scu_testcase *const *ta = a;
	const _scu_testcase *const *tb = b;
	return (*ta)->line - (*tb)->line;
}

int
main(int argc, char *argv[])
{
	_scu_arguments args = get_arguments(argc, argv, _scu_module_num_tests);

	qsort(_scu_module_tests, _scu_module_num_tests, sizeof(_scu_testcase *), _scu_line_comparator);

	if (args.list) {
		list_tests();
	} else if (args.run) {
		run_tests(args.num_tests, args.test_indices);
	}

	return 0;
}
