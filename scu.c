#include <assert.h>
#include <jansson.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "scu.h"

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
_scu_capture_test_failures (json_t *json_test, size_t num, _scu_failure_t *failures)
{
	json_t *json_failures = json_array ();
	json_object_set (json_test, "failures", json_failures);

	for (size_t i = 0; i < num; i++) {
		json_t *json_failure = json_object ();
		json_object_set (json_failure, "file", json_string (failures[i].file));
		json_object_set (json_failure, "line", json_integer (failures[i].line));
		json_object_set (json_failure, "message", json_string (failures[i].msg));
		json_array_append (json_failures, json_failure);
	}
}

static void
_scu_capture_test_result (json_t *json_test, bool success, size_t asserts,
                          double mono_time, double cpu_time)
{
	json_object_set (json_test, "success", json_boolean (success));
	json_object_set (json_test, "asserts", json_integer (asserts));
	json_object_set (json_test, "duration", json_real (mono_time));
	json_object_set (json_test, "cpu_time", json_real (cpu_time));
}

json_t *
_scu_run_test (_scu_testcase_t *test)
{
	char temp_filename[] = "/tmp/scu.XXXXXX";
	int out = mkstemp (temp_filename);
	assert (out > 0);
	dup2 (out, STDOUT_FILENO);
	dup2 (out, STDERR_FILENO);
	setvbuf (stdout, NULL, _IONBF, 0);
	setvbuf (stderr, NULL, _IONBF, 0);

	json_t *json_test = json_object ();
	json_object_set (json_test, "event", json_string ("testcase"));
	json_object_set (json_test, "output", json_string (temp_filename));
	json_object_set (json_test, "description", json_string (test->desc));

	struct timespec start_mono_time, end_mono_time, start_cpu_time, end_cpu_time;

	_scu_before_each ();

	clock_gettime (CLOCK_MONOTONIC, &start_mono_time);
	clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &start_cpu_time);

	_scu_failure_t failures[_SCU_MAX_FAILURES];
	bool success = true;
	size_t asserts = 0, num_failures = 0;
	test->func (&success, &asserts, &num_failures, failures);

	clock_gettime (CLOCK_MONOTONIC, &end_mono_time);
	clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &end_cpu_time);

	_scu_after_each ();

	_scu_capture_test_failures (json_test, num_failures, failures);

	_scu_capture_test_result (json_test, success, asserts,
	                          _scu_get_time_diff (start_mono_time, end_mono_time),
													  _scu_get_time_diff (start_cpu_time, end_cpu_time));

	return json_test;
}

void
_scu_write_json (int fd, json_t *json)
{
	char *msg = json_dumps (json, 0);
	write (fd, msg, strlen (msg));
	free (msg);
	write (fd, "\n", 1);
}

int
main (int argc, char **argv)
{
	int cmd = dup (STDOUT_FILENO);

	json_t *json_suite_start = json_object ();
	json_object_set (json_suite_start, "name", json_string (_scu_suite_name));
	json_object_set (json_suite_start, "event", json_string ("suite_start"));
	_scu_write_json (cmd, json_suite_start);

	_scu_setup ();

	for (_scu_testcase_t *test = &_scu_testcases_start; test < &_scu_testcases_end; test++) {
		json_t *json_test = _scu_run_test (test);
		_scu_write_json (cmd, json_test);
	}

	_scu_teardown ();

	json_t *json_suite_end = json_object ();
	json_object_set (json_suite_end, "name", json_string (_scu_suite_name));
	json_object_set (json_suite_end, "event", json_string ("suite_end"));
	_scu_write_json (cmd, json_suite_end);

	return 0;
}
