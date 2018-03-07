#ifndef _SCU_INTERNAL_H_
#define _SCU_INTERNAL_H_

#include <stdbool.h>
#include <string.h>

#include "config.h"
#include "pretty.h"
#include "util.h"

/* Internal structures */

typedef struct {
	void (*func)(void);
	size_t index;
	const char *name;
	const char *desc;
	const char *tags[_SCU_MAX_TAGS];
} _scu_testcase;

typedef struct {
	const char *file;
	int line;
	const char *assert_method;
	char msg[_SCU_FAILURE_MESSAGE_LENGTH];
	char lhs[_SCU_FAILURE_VALUE_LENGTH];
	char rhs[_SCU_FAILURE_VALUE_LENGTH];
	char lhs_value[_SCU_FAILURE_VALUE_LENGTH];
	char rhs_value[_SCU_FAILURE_VALUE_LENGTH];
} _scu_failure;

/* Test module forward declarations */

void _scu_setup(void);
void _scu_teardown(void);
void _scu_before_each(void);
void _scu_after_each(void);

void _scu_register_testcase(_scu_testcase *tc);
void _scu_account_assert(bool is_fatal);
void _scu_handle_failure(const char *file, int line, const char *assert_method, const char *msg, const char *actual, const char *expected, const char *actual_value, const char *expected_value, bool is_fatal);

/* Internal assertion functions */

static inline void __attribute__((used))
_scu_assert_equal_int(const char *file, int line, const char *assert_method, const char *actual_str, const char *expected_str, unsigned long long actual, unsigned long long expected, size_t actual_size, size_t expected_size, bool invert, bool is_fatal)
{
	if ((actual == expected) ^ invert)
		return;

	char actual_buf[64];
	char expected_buf[64];
	_scu_prettyprint_integer_value(actual_buf, sizeof(actual_buf), actual, actual_size);
	_scu_prettyprint_integer_value(expected_buf, sizeof(expected_buf), expected, expected_size);
	_scu_handle_failure(file, line, assert_method, NULL, actual_str, expected_str, actual_buf, expected_buf, is_fatal);
}

static inline void __attribute__((used))
_scu_assert_equal_ptr(const char *file, int line, const char *assert_method, const char *actual_str, const char *expected_str, const void *actual, const void *expected, bool invert, bool is_fatal)
{
	if ((actual == expected) ^ invert)
		return;

	char actual_buf[20];
	char expected_buf[20];
	_scu_prettyprint_pointer_value(actual_buf, sizeof(actual_buf), actual);
	if (!expected && invert) {
		_scu_strlcpy(expected_buf, "not null", sizeof(expected_buf));
	} else {
		_scu_prettyprint_pointer_value(expected_buf, sizeof(expected_buf), expected);
	}
	_scu_handle_failure(file, line, assert_method, NULL, actual_str, expected_str, actual_buf, expected_str ? expected_buf : NULL, is_fatal);
}





// TODO: rewrite me
static inline void __attribute__((used))
_scu_handle_assert_nstr(const char *file, int line, const char *actual, const char *expected, int size, bool is_fatal, bool inverse, const char *assert_method, const char *actual_str, const char *expected_str)
{
	int res;
	if (size == -1) {
		res = strcmp(actual, expected);
	} else {
		res = strncmp(actual, expected, size);
	}
	if ((res != 0) ^ inverse) {
		char actual_value[256];
		char expected_value[256];

		/* TODO: For overlong strings find the (first) section where they differ */
		_scu_cescape_str(actual_value, actual, sizeof(actual_value));
		_scu_cescape_str(expected_value, expected, sizeof(expected_value));
		_scu_handle_failure(file, line, assert_method, NULL, actual_str, expected_str, actual_value, expected_value, is_fatal);
	}
}





static inline void __attribute__((used))
_scu_assert_equal_memory(const char *file, int line, const char *assert_method, const char *actual_str, const char *expected_str, const void *actual, const void *expected, size_t size, bool invert, bool is_fatal)
{
	if ((memcmp(actual, expected, size) == 0) ^ invert)
		return;

	char actual_buf[256];
	char expected_buf[256];
	/* TODO: For overlong strings find the (first) section where they differ */
	_scu_prettyprint_bytes_value(actual_buf, sizeof(actual_buf), actual, size);
	_scu_prettyprint_bytes_value(expected_buf, sizeof(expected_buf), expected, size);
	_scu_handle_failure(file, line, assert_method, NULL, actual_str, expected_str, actual_buf, expected_buf, is_fatal);
}

/* Internal assertion macros */

#define _SCU_HANDLE_FAILURE(assert_method, msg, actual, expected, actual_value, expected_value, is_fatal) \
	_scu_handle_failure(__FILE__, __LINE__, assert_method, msg, actual, expected, actual_value, expected_value, is_fatal)

#define _SCU_ASSERT_WITH_MESSAGE(cond, assert_method, actual, is_fatal, message, ...) \
	do { \
		_scu_account_assert(is_fatal); \
		if (!(cond)) { \
			char _scu_assert_msg[1024]; \
			snprintf(_scu_assert_msg, sizeof(_scu_assert_msg), message, ##__VA_ARGS__); \
			_SCU_HANDLE_FAILURE(assert_method, _scu_assert_msg, actual, NULL, NULL, NULL, is_fatal); \
		} \
	} while (0)

#define _SCU_ASSERT(cond, assert_method, actual, is_fatal) \
	do { \
		_scu_account_assert(is_fatal); \
		if (!(cond)) { \
			_SCU_HANDLE_FAILURE(assert_method, NULL, actual, NULL, NULL, NULL, is_fatal); \
		} \
	} while (0)

#define _SCU_ASSERT_EQUAL(assert_method, actual, expected, invert, is_fatal) \
	_Generic (_scu_temp_expected, \
		int: _SCU_ASSERT_EQUAL_INT(assert_method, actual, expected, invert, is_fatal), \
		unsigned int: _SCU_ASSERT_EQUAL_INT(assert_method, actual, expected, invert, is_fatal), \
		float: _SCU_ASSERT_EQUAL_FLOAT(assert_method, actual, expected, invert, is_fatal), \
		double: _SCU_ASSERT_EQUAL_FLOAT(assert_method, actual, expected, invert, is_fatal), \
		void *: _SCU_ASSERT_EQUAL_POINTER(assert_method, actual, expected, invert, is_fatal), \
		char *: _SCU_ASSERT_EQUAL_STRING(assert_method, actual, expected, -1, invert, is_fatal), \
		default: _SCU_ASSERT((actual) == (expected), is_fatal, assert_method) \
	) \

#define _SCU_ASSERT_EQUAL_INT(assert_method, actual, expected, invert, is_fatal) \
	do { \
		typeof(actual) _scu_temp_actual = (actual); \
		typeof(expected) _scu_temp_expected = (expected); \
		_scu_account_assert(is_fatal); \
		_scu_assert_equal_int(__FILE__, __LINE__, assert_method, #actual, #expected, _scu_temp_actual, _scu_temp_expected, sizeof(_scu_temp_actual), sizeof(_scu_temp_expected), invert, is_fatal); \
	} while (0)

#define _SCU_ASSERT_EQUAL_POINTER(assert_method, actual, expected, invert, is_fatal) \
	do { \
		_scu_account_assert(is_fatal); \
		_scu_assert_equal_ptr(__FILE__, __LINE__, assert_method, #actual, #expected, actual, expected, invert, is_fatal); \
	} while (0)

#define _SCU_ASSERT_EQUAL_FLOAT(assert_method, actual, expected, invert, is_fatal) \
	_scu_assert_equal_float(__FILE__, __LINE__, assert_method, #actual, #expected, actual, expected, invert, is_fatal)

#define _SCU_ASSERT_EQUAL_STRING(assert_method, actual, expected, size, invert, is_fatal) \
	_scu_assert_equal_str(__FILE__, __LINE__, assert_method, #actual, #expected, actual, expected, size, invert, is_fatal)

#define _SCU_ASSERT_EQUAL_MEMORY(assert_method, actual, expected, size, invert, is_fatal) \
	do { \
		_scu_account_assert(is_fatal); \
		_scu_assert_equal_memory(__FILE__, __LINE__, assert_method, #actual, #expected, actual, expected, size, invert, is_fatal); \
	} while (0)

#endif
