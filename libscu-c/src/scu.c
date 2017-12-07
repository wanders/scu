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

static void
_scu_flush_json(int fd)
{
	write(fd, "\n", 1);
}

static void
_scu_output_module_list(int fd, const char *modulename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "module_list");
	json_separator(fd);
	json_object_key(fd, "name");
	json_string(fd, modulename);
	json_object_end(fd);
	_scu_flush_json(fd);
}

static void
_scu_output_test_list(int fd, int line, const char *name, const char *description, const char *tags[])
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "testcase_list");
	json_separator(fd);
	json_object_key(fd, "line");
	json_integer(fd, line);
	json_separator(fd);
	json_object_key(fd, "name");
	json_string(fd, name);
	json_separator(fd);
	json_object_key(fd, "description");
	json_string(fd, description);
	json_separator(fd);
	json_object_key(fd, "tags");
	json_array_start(fd);
	if (*tags) {
		json_string(fd, tags[0]);
		for (size_t i = 1; tags[i] && i < _SCU_MAX_TAGS; i++) {
			json_separator(fd);
			json_string(fd, tags[i]);
		}
	}
	json_array_end(fd);
	json_object_end(fd);
	_scu_flush_json(fd);
}

static void
_scu_output_module_start(int fd, const char *modulename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "module_start");
	json_separator(fd);
	json_object_key(fd, "name");
	json_string(fd, modulename);
	json_object_end(fd);
	_scu_flush_json(fd);
}

static void
_scu_output_module_end(int fd)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "module_end");
	json_object_end(fd);
	_scu_flush_json(fd);
}

static void
_scu_output_setup_start(int fd, const char *filename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "setup_start");
	json_separator(fd);
	json_object_key(fd, "output");
	json_string(fd, filename);
	json_object_end(fd);
	_scu_flush_json(fd);
}
static void
_scu_output_setup_end(int fd)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "setup_end");
	json_object_end(fd);
	_scu_flush_json(fd);
}

static void
_scu_output_teardown_start(int fd, const char *filename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "teardown_start");
	json_separator(fd);
	json_object_key(fd, "output");
	json_string(fd, filename);
	json_object_end(fd);
	_scu_flush_json(fd);
}
static void
_scu_output_teardown_end(int fd)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "teardown_end");
	json_object_end(fd);
	_scu_flush_json(fd);
}

static void
_scu_output_test_start(int fd, int idx, const char *name, const char *filename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "testcase_start");
	json_separator(fd);
	json_object_key(fd, "index");
	json_integer(fd, idx);
	json_separator(fd);
	json_object_key(fd, "name");
	json_string(fd, name);
	json_separator(fd);
	json_object_key(fd, "output");
	json_string(fd, filename);
	json_object_end(fd);
	_scu_flush_json(fd);
}

static void
_scu_output_test_failure(int fd, _scu_failure *failure)
{
	json_object_start(fd);
	json_object_key(fd, "file");
	json_string(fd, failure->file);
	json_separator(fd);
	json_object_key(fd, "line");
	json_integer(fd, failure->line);
	json_separator(fd);
	json_object_key(fd, "message");
	json_string(fd, failure->msg);
	json_separator(fd);
	json_object_key(fd, "assert_method");
	json_string(fd, failure->assert_method);
	json_separator(fd);
	json_object_key(fd, "assert_method_values");
	json_array_start(fd);
	if (failure->lhs[0]) {
		json_object_start(fd);
		json_object_key(fd, "name");
		json_string(fd, failure->lhs);
		if (failure->lhs_value[0]) {
			json_separator(fd);
			json_object_key(fd, "value");
			json_string(fd, failure->lhs_value);
		}
		json_object_end(fd);
		if (failure->rhs[0]) {
			json_separator(fd);
			json_object_start(fd);
			json_object_key(fd, "name");
			json_string(fd, failure->rhs);
			if (failure->rhs_value[0]) {
				json_separator(fd);
				json_object_key(fd, "value");
				json_string(fd, failure->rhs_value);
			}
			json_object_end(fd);
		}
	}
	json_array_end(fd);
	json_object_end(fd);
}

static void
_scu_output_test_failures(int fd, size_t num, _scu_failure *failures)
{
	json_separator(fd);
	json_object_key(fd, "failures");
	json_array_start(fd);
	if (num > 0) {
		_scu_output_test_failure(fd, &failures[0]);
		for (size_t i = 1; i < num; i++) {
			json_separator(fd);
			_scu_output_test_failure(fd, &failures[i]);
		}
	}
	json_array_end(fd);
}

static void
_scu_output_test_end(int fd, int idx, bool success, size_t asserts,
                     double mono_time, double cpu_time,
                     size_t num_failures, _scu_failure *failures)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "testcase_end");
	json_separator(fd);
	json_object_key(fd, "index");
	json_integer(fd, idx);
	json_separator(fd);
	json_object_key(fd, "success");
	json_boolean(fd, success);
	json_separator(fd);
	json_object_key(fd, "asserts");
	json_integer(fd, asserts);
	json_separator(fd);
	json_object_key(fd, "duration");
	json_real(fd, mono_time);
	json_separator(fd);
	json_object_key(fd, "cpu_time");
	json_real(fd, cpu_time);
	_scu_output_test_failures(fd, num_failures, failures);
	json_object_end(fd);
	_scu_flush_json(fd);
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
	int cmd = STDOUT_FILENO;

	_scu_output_module_list(cmd, _scu_module_name);

	for (size_t i = 0; i < _scu_module_num_tests; i++) {
		_scu_testcase *test = _scu_module_tests[i];
		_scu_output_test_list(cmd, test->line, test->name, test->desc, test->tags);
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

static pid_t _scu_fatal_assert_allowed_thread_id;
static bool _scu_fatal_assert_jmpbuf_valid;
static jmp_buf _scu_fatal_assert_jmpbuf;

static pid_t
_scu_get_current_thread_id(void)
{
	return syscall(SYS_gettid);
}

void
_scu_fatal_assert_allowed(void)
{
	assert(_scu_fatal_assert_jmpbuf_valid);
	/* FATAL errors can only be used from same thread */
	assert(_scu_get_current_thread_id() == _scu_fatal_assert_allowed_thread_id);
}

void
_scu_handle_fatal_assert(void)
{
	longjmp(_scu_fatal_assert_jmpbuf, 1);
}

static bool _scu_success;
static size_t _scu_num_failures;
static _scu_failure _scu_failures[_SCU_MAX_FAILURES];
;
size_t _scu_num_asserts;

_scu_failure *
_scu_report_failure(const char *file, int line, const char *assert_method)
{
	_scu_failure *res;

	_scu_success = false;

	if (_scu_num_failures >= _SCU_MAX_FAILURES)
		return NULL;

	res = &_scu_failures[_scu_num_failures];
	_scu_num_failures++;

	*res = (_scu_failure){
	    .file = file,
	    .line = line,
	    .assert_method = assert_method};

	return res;
}

void
_scu_cescape_str(char *dest, size_t siz, const char *src)
{
	int o = 0;

	assert(siz >= 3);

	dest[o++] = '"';

	for (int i = 0; src[i]; i++) {
		unsigned char c = src[i];
		if (c == '\n') {
			if (o + 2 >= siz - 2)
				break;
			dest[o++] = '\\';
			dest[o++] = 'n';
		} else if (c == '\t') {
			if (o + 2 >= siz - 2)
				break;
			dest[o++] = '\\';
			dest[o++] = 't';
		} else if (c < 32 || c >= 127) {
			if (o + 4 >= siz - 2)
				break;
			dest[o++] = '\\';
			dest[o++] = 'x';
			dest[o++] = "0123456789abcdef"[c >> 4];
			dest[o++] = "0123456789abcdef"[c & 15];
		} else {
			if (o + 1 >= siz - 2)
				break;
			dest[o++] = c;
		}
	}
	dest[o++] = '"';
	dest[o++] = 0;
}

static void
_scu_run_test(int fd, int idx)
{
	_scu_testcase *test = _scu_module_tests[idx];

	char filename[SCU_OUTPUT_FILENAME_TEMPLATE_SIZE];
	_scu_redirect_output(filename, sizeof(filename));

	_scu_output_test_start(fd, idx, test->name, filename);

	struct timespec start_mono_time, end_mono_time, start_cpu_time, end_cpu_time;

	_scu_before_each();

	clock_gettime(CLOCK_MONOTONIC, &start_mono_time);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_cpu_time);

	_scu_success = true;
	_scu_num_asserts = 0;
	_scu_num_failures = 0;

	_scu_fatal_assert_jmpbuf_valid = true;
	_scu_fatal_assert_allowed_thread_id = _scu_get_current_thread_id();
	if (!setjmp(_scu_fatal_assert_jmpbuf)) {
		test->func();
	}
	_scu_fatal_assert_allowed_thread_id = 0;
	_scu_fatal_assert_jmpbuf_valid = false;

	clock_gettime(CLOCK_MONOTONIC, &end_mono_time);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu_time);

	_scu_after_each();

	_scu_output_test_end(fd, idx, _scu_success, _scu_num_asserts,
	                     _scu_get_time_diff(start_mono_time, end_mono_time),
	                     _scu_get_time_diff(start_cpu_time, end_cpu_time),
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

	_scu_redirect_output(filename, sizeof(filename));

	_scu_output_setup_start(cmd, filename);

	_scu_setup();

	_scu_output_setup_end(cmd);

	for (size_t i = 0; i < num_tests; i++) {
		_scu_run_test(cmd, test_indices[i]);
	}

	_scu_redirect_output(filename, sizeof(filename));

	_scu_output_teardown_start(cmd, filename);

	_scu_teardown();

	_scu_output_teardown_end(cmd);

	_scu_output_module_end(cmd);
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
