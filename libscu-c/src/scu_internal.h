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
	int line;
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
void _scu_handle_fatal_assert(void) __attribute__((noreturn));
_scu_failure *_scu_report_failure(const char *file, int line, const char *assert_method);

/* Internal functions */

static inline void
_scu_handle_assert(const char *file, int line, bool cond, bool is_fatal, const char *assert_method, const char *msg, const char *actual, const char *expected, const char *actual_value, const char *expected_value)
{
	if (cond)
		return;

	_scu_failure *fail = _scu_report_failure(file, line, assert_method);
	if (fail) {
		_scu_strlcpy(fail->msg, msg ?: "", sizeof(fail->msg));
		_scu_strlcpy(fail->lhs, actual ?: "", sizeof(fail->lhs));
		_scu_strlcpy(fail->rhs, expected ?: "", sizeof(fail->rhs));
		_scu_strlcpy(fail->lhs_value, actual_value ?: "", sizeof(fail->lhs_value));
		_scu_strlcpy(fail->rhs_value, expected_value ?: "", sizeof(fail->rhs_value));
	}

	if (is_fatal)
		_scu_handle_fatal_assert();
}

static inline void __attribute__((used))
_scu_handle_assert_int(const char *file, int line, bool condition, unsigned long long actual, size_t actual_bitlen, unsigned long long expected, size_t expected_bitlen, bool is_fatal, bool inverse, const char *assert_method, const char *actual_str, const char *expected_str)
{
	if ((!condition) ^ inverse) {
		char actual_value[64];
		char expected_value[64];
		_scu_prettyprint_numeric_value(actual_value, sizeof(actual_value), actual, actual_bitlen);
		_scu_prettyprint_numeric_value(expected_value, sizeof(expected_value), expected, expected_bitlen);
		_scu_handle_assert(file, line, false, is_fatal, assert_method, NULL, actual_str, expected_str, actual_value, expected_value);
	}
}

static inline void __attribute__((used))
_scu_handle_assert_ptr(const char *file, int line, const void *actual, const void *expected, bool is_fatal, bool inverse, const char *assert_method, const char *actual_str, const char *expected_str)
{
	if ((actual != expected) ^ inverse) {
		char actual_value[20];
		char expected_value[20];
		_scu_prettyprint_pointer_value(actual_value, sizeof(actual_value), actual);
		if (expected_str) {
			if (inverse && expected == NULL) {
				_scu_strlcpy(expected_value, "<NOT NULL>", sizeof(expected_value));
			} else {
				_scu_prettyprint_pointer_value(expected_value, sizeof(expected_value), expected);
			}
		}
		_scu_handle_assert(file, line, false, is_fatal, assert_method, NULL, actual_str, expected_str, actual_value, expected_str ? expected_value : NULL);
	}
}

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
		_scu_handle_assert(file, line, false, is_fatal, assert_method, NULL, actual_str, expected_str, actual_value, expected_value);
	}
}

/* Internal assertion macros */

#define _SCU_HANDLE_ASSERT(cond, is_fatal, assert_method, msg, actual, expected, actual_value, expected_value) \
	_scu_handle_assert(__FILE__, __LINE__, cond, is_fatal, assert_method, msg, actual, expected, actual_value, expected_value)

#define _SCU_ASSERT_WITH_MESSAGE(cond, is_fatal, assert_method, message, ...) \
	do { \
		_scu_account_assert(is_fatal); \
		if (!(cond)) { \
			char _scu_assert_msg[1024]; \
			snprintf(_scu_assert_msg, sizeof(_scu_assert_msg), message, ##__VA_ARGS__); \
			_SCU_HANDLE_ASSERT(cond, is_fatal, assert_method, _scu_assert_msg, #cond, "TRUE", NULL, NULL); \
		} \
	} while (0)

#define _SCU_ASSERT(cond, is_fatal, assert_method) \
	do { \
		_scu_account_assert(is_fatal); \
		if (!(cond)) { \
			_SCU_HANDLE_ASSERT(cond, is_fatal, assert_method, "", #cond, "TRUE", NULL, NULL); \
		} \
	} while (0)

#endif
