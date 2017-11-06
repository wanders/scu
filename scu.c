#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "scu.h"
#include "json.h"

/* Optional hook functions */

__attribute__ ((weak)) void
_scu_setup (void)
{
}

__attribute__ ((weak)) void
_scu_teardown (void)
{
}

__attribute__ ((weak)) void
_scu_before_each (void)
{
}

__attribute__ ((weak)) void
_scu_after_each (void)
{
}

/* Test module name */

extern const char *_scu_module_name;

/* Test protocol functions */

static double
_scu_get_time_diff (struct timespec startt, struct timespec endt)
{
	time_t dsec = endt.tv_sec - startt.tv_sec;
	long dnsec = endt.tv_nsec - startt.tv_nsec;

	if (dnsec < 0) {
		dnsec += 1000000000;
		dsec--;
	}

	return dsec + dnsec / 1e9;
}

static void
_scu_flush_json (int fd)
{
	write (fd, "\n", 1);
}

static void
_scu_output_module_start (int fd, const char *modulename)
{
	json_object_start (fd);
	json_object_key (fd, "name");
	json_string (fd, modulename);
	json_separator (fd);
	json_object_key (fd, "event");
	json_string (fd, "module_start");
	json_object_end (fd);
	_scu_flush_json (fd);
}


static void
_scu_output_module_end (int fd, const char *modulename)
{
	json_object_start (fd);
	json_object_key (fd, "name");
	json_string (fd, modulename);
	json_separator (fd);
	json_object_key (fd, "event");
	json_string (fd, "module_end");
	json_object_end (fd);
	_scu_flush_json (fd);
}

static void
_scu_output_test_start (int fd, const char *filename, const char *description)
{
	json_object_start (fd);
	json_object_key (fd, "event");
	json_string (fd, "testcase_start");
	json_separator (fd);
	json_object_key (fd, "output");
	json_string (fd, filename);
	json_separator (fd);
	json_object_key (fd, "description");
	json_string (fd, description);
	json_object_end (fd);
	_scu_flush_json (fd);
}

static void
_scu_output_test_failure (int fd, _scu_failure *failure)
{
	json_object_start (fd);
	json_object_key (fd, "file");
	json_string (fd, failure->file);
	json_separator (fd);
	json_object_key (fd, "line");
	json_integer (fd, failure->line);
	json_separator (fd);
	json_object_key (fd, "message");
	json_string (fd, failure->msg);
	json_object_end (fd);
}

static void
_scu_output_test_failures (int fd, size_t num, _scu_failure *failures)
{
	json_separator (fd);
	json_object_key (fd, "failures");
	json_array_start (fd);

	if (num > 0) {
		_scu_output_test_failure (fd, &failures[0]);
		for (size_t i = 1; i < num; i++) {
			json_separator (fd);
			_scu_output_test_failure (fd, &failures[i]);
		}
	}

	json_array_end (fd);
}

static void
_scu_output_test_end (int fd, bool success, size_t asserts,
                      double mono_time, double cpu_time,
                      size_t num_failures, _scu_failure *failures)
{
	json_object_start (fd);
	json_object_key (fd, "event");
	json_string (fd, "testcase_end");
	json_separator (fd);
	json_object_key (fd, "success");
	json_boolean (fd, success);
	json_separator (fd);
	json_object_key (fd, "asserts");
	json_integer (fd, asserts);
	json_separator (fd);
	json_object_key (fd, "duration");
	json_real (fd, mono_time);
	json_separator (fd);
	json_object_key (fd, "cpu_time");
	json_real (fd, cpu_time);

	_scu_output_test_failures (fd, num_failures, failures);

	json_object_end (fd);
	_scu_flush_json (fd);
}

static void
_scu_run_test (int fd, _scu_testcase *test)
{
	char temp_filename[] = "/tmp/scu.XXXXXX";
	int out = mkstemp (temp_filename);
	assert (out > 0);
	dup2 (out, STDOUT_FILENO);
	dup2 (out, STDERR_FILENO);
	setvbuf (stdout, NULL, _IONBF, 0);
	setvbuf (stderr, NULL, _IONBF, 0);

	_scu_output_test_start (fd, temp_filename, test->desc);

	struct timespec start_mono_time, end_mono_time, start_cpu_time, end_cpu_time;

	_scu_before_each ();

	clock_gettime (CLOCK_MONOTONIC, &start_mono_time);
	clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &start_cpu_time);

	_scu_failure failures[_SCU_MAX_FAILURES];
	bool success = true;
	size_t asserts = 0, num_failures = 0;
	test->func (&success, &asserts, &num_failures, failures);

	clock_gettime (CLOCK_MONOTONIC, &end_mono_time);
	clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &end_cpu_time);

	_scu_after_each ();

	_scu_output_test_end (fd, success, asserts,
	                      _scu_get_time_diff (start_mono_time, end_mono_time),
	                      _scu_get_time_diff (start_cpu_time, end_cpu_time),
	                      num_failures, failures);
}

static int
_scu_line_comparator (const void *a, const void *b)
{
	const _scu_testcase * const *ta = a;
	const _scu_testcase * const *tb = b;
	return (*ta)->line - (*tb)->line;
}

static void _scu_sigcont_handler (int sig) {}

int
main (void)
{
	int cmd = dup (STDOUT_FILENO);

	_scu_testcase **tests = &_scu_testcases_start;
	size_t num_tests = ((void*)&_scu_testcases_end - (void*)&_scu_testcases_start) / sizeof (_scu_testcase*);

	qsort (tests, num_tests, sizeof (_scu_testcase*), _scu_line_comparator);

	if (getenv ("SCU_WAIT_FOR_DEBUGGER")) {
		/* We use an empty signal handler to wait for SIGCONT with pause() */
		signal (SIGCONT, _scu_sigcont_handler);
		pause ();
	}

	_scu_output_module_start (cmd, _scu_module_name);

	_scu_setup ();

	for (size_t i = 0; i < num_tests; i++) {
		_scu_run_test (cmd, tests[i]);
	}

	_scu_teardown ();

	_scu_output_module_end (cmd, _scu_module_name);

	return 0;
}
