#include <assert.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "scu.h"
#include "json.h"

/* Optional hook functions */

void _scu_setup (void);
void _scu_teardown (void);
void _scu_before_each (void);
void _scu_after_each (void);

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

/* Test suite name */

extern const char *_scu_suite_name;

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
_scu_output_suite_start (int fd, const char *suitename)
{
	json_object_start (fd);
	json_object_key (fd, "name");
	json_string (fd, suitename);
	json_separator (fd);
	json_object_key (fd, "event");
	json_string (fd, "suite_start");
	json_object_end (fd);
	_scu_flush_json (fd);
}


static void
_scu_output_suite_end (int fd, const char *suitename)
{
	json_object_start (fd);
	json_object_key (fd, "name");
	json_string (fd, suitename);
	json_separator (fd);
	json_object_key (fd, "event");
	json_string (fd, "suite_end");
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

int
main (int argc, char **argv)
{
	int cmd = dup (STDOUT_FILENO);

	_scu_output_suite_start (cmd, _scu_suite_name);

	_scu_setup ();

	for (_scu_testcase *test = &_scu_testcases_start; test < &_scu_testcases_end; test++) {
		_scu_run_test (cmd, test);
	}

	_scu_teardown ();

	_scu_output_suite_end (cmd, _scu_suite_name);

	return 0;
}
