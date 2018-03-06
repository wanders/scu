#include <stdbool.h>

#include "config.h"
#include "json.h"
#include "scu_internal.h"

static inline void
flush_json(int fd)
{
	write(fd, "\n", 1);
}

void
_scu_output_module_list(int fd, const char *modulename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "module_list");
	json_separator(fd);
	json_object_key(fd, "name");
	json_string(fd, modulename);
	json_object_end(fd);
	flush_json(fd);
}

void
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
	flush_json(fd);
}

void
_scu_output_module_start(int fd, const char *modulename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "module_start");
	json_separator(fd);
	json_object_key(fd, "name");
	json_string(fd, modulename);
	json_object_end(fd);
	flush_json(fd);
}

void
_scu_output_module_end(int fd)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "module_end");
	json_object_end(fd);
	flush_json(fd);
}

void
_scu_output_setup_start(int fd, const char *filename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "setup_start");
	json_separator(fd);
	json_object_key(fd, "output");
	json_string(fd, filename);
	json_object_end(fd);
	flush_json(fd);
}
void
_scu_output_setup_end(int fd)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "setup_end");
	json_object_end(fd);
	flush_json(fd);
}

void
_scu_output_teardown_start(int fd, const char *filename)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "teardown_start");
	json_separator(fd);
	json_object_key(fd, "output");
	json_string(fd, filename);
	json_object_end(fd);
	flush_json(fd);
}
void
_scu_output_teardown_end(int fd)
{
	json_object_start(fd);
	json_object_key(fd, "event");
	json_string(fd, "teardown_end");
	json_object_end(fd);
	flush_json(fd);
}

void
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
	flush_json(fd);
}

void
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

void
_scu_output_test_failures(int fd, size_t num, _scu_failure failures[])
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

void
_scu_output_test_end(int fd, int idx, bool success, size_t asserts,
                     double mono_time, double cpu_time,
                     size_t num_failures, _scu_failure failures[])
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
	flush_json(fd);
}
