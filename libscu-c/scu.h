#ifndef _SCU_H_
#define _SCU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Configuration parameters */

#define _SCU_MAX_TESTS 4096
#define _SCU_MAX_TAGS 128
#define _SCU_MAX_FAILURES 1024
#define _SCU_FAILURE_MESSAGE_LENGTH 2048

/* Test module functions */

void _scu_setup(void);
void _scu_teardown(void);
void _scu_before_each(void);
void _scu_after_each(void);

#define SCU_MODULE(name) \
	const char *_scu_module_name = name

#define SCU_SETUP() \
	void _scu_setup(void)

#define SCU_TEARDOWN() \
	void _scu_teardown(void)

#define SCU_BEFORE_EACH() \
	void _scu_before_each(void)

#define SCU_AFTER_EACH() \
	void _scu_after_each(void)

/* Test module summary variable pointers */

typedef struct {
	const char *file;
	int line;
	const char *assert_method;
	char msg[_SCU_FAILURE_MESSAGE_LENGTH];
	char lhs[256];
	char rhs[256];
	char lhs_value[256];
	char rhs_value[256];
} _scu_failure;

extern size_t _scu_num_asserts;

/* Test case definition */

typedef struct {
	void (*func)(void);
	int line;
	const char *name;
	const char *desc;
	const char *tags[_SCU_MAX_TAGS];
} _scu_testcase;

void _scu_register_testcase(_scu_testcase *);
void _scu_handle_fatal_assert(void) __attribute__((noreturn));
void _scu_fatal_assert_allowed(void);
_scu_failure *_scu_report_failure(const char *file, int line, const char *assert_method);
void _scu_cescape_str(char *dest, size_t siz, const char *src);

static void
_scu_strlcpy(char *dst, size_t siz, const char *src)
{
	size_t pos = 0;
	for (;;) {
		if (pos == siz - 1) {
			dst[pos] = 0;
			break;
		}
		dst[pos] = src[pos];
		if (!src[pos])
			break;
		pos++;
	}
}

static inline void
_scu_account_assert(bool is_fatal)
{
	if (is_fatal)
		_scu_fatal_assert_allowed();
	_scu_num_asserts++;
}

static inline void
_scu_handle_assert(const char *file, int line, bool cond, bool is_fatal, const char *assert_method, const char *msg, const char *actual, const char *expected, const char *actual_value, const char *expected_value)
{
	if (cond)
		return;

	_scu_failure *fail = _scu_report_failure(file, line, assert_method);
	if (fail) {
		_scu_strlcpy(fail->lhs, sizeof(fail->lhs), actual ?: "");
		_scu_strlcpy(fail->rhs, sizeof(fail->rhs), expected ?: "");
		_scu_strlcpy(fail->lhs_value, sizeof(fail->lhs_value), actual_value ?: "");
		_scu_strlcpy(fail->rhs_value, sizeof(fail->rhs_value), expected_value ?: "");
		_scu_strlcpy(fail->msg, sizeof(fail->msg), msg ?: "");
	}

	if (is_fatal)
		_scu_handle_fatal_assert();
}

static inline void
_scu_prettyprint_numeric_value(char *buf, size_t bufsiz, unsigned long long value)
{
	if (value & 1LLU << 63) {
		snprintf(buf, bufsiz, "%llu (0x%llx == %lld)", value, value, value);
	} else {
		snprintf(buf, bufsiz, "%llu (0x%llx)", value, value);
	}
}

static inline void
_scu_prettyprint_pointer_value(char *buf, size_t bufsiz, const void *value)
{
	if (value) {
		snprintf(buf, bufsiz, "%p", value);
	} else {
		snprintf(buf, bufsiz, "NULL");
	}
}

static inline void
_scu_handle_assert_int(const char *file, int line, unsigned long long actual, unsigned long long expected, bool is_fatal, bool inverse, const char *assert_method, const char *actual_str, const char *expected_str)
{
	if ((actual != expected) ^ inverse) {
		char actual_value[64];
		char expected_value[64];
		_scu_prettyprint_numeric_value(actual_value, sizeof(actual_value), actual);
		_scu_prettyprint_numeric_value(expected_value, sizeof(expected_value), expected);
		_scu_handle_assert(file, line, false, is_fatal, assert_method, NULL, actual_str, expected_str, actual_value, expected_value);
	}
}

static inline void
_scu_handle_assert_ptr(const char *file, int line, const void *actual, const void *expected, bool is_fatal, bool inverse, const char *assert_method, const char *actual_str, const char *expected_str)
{
	if ((actual != expected) ^ inverse) {
		char actual_value[20];
		char expected_value[20];
		_scu_prettyprint_pointer_value(actual_value, sizeof(actual_value), actual);
		if (expected_str) {
			if (inverse && expected == NULL) {
				_scu_strlcpy(expected_value, sizeof(expected_value), "<NOT NULL>");
			} else {
				_scu_prettyprint_pointer_value(expected_value, sizeof(expected_value), expected);
			}
		}
		_scu_handle_assert(file, line, false, is_fatal, assert_method, NULL, actual_str, expected_str, actual_value, expected_str ? expected_value : NULL);
	}
}

static inline void
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
		_scu_cescape_str(actual_value, sizeof(actual_value), actual);
		_scu_cescape_str(expected_value, sizeof(expected_value), expected);
		_scu_handle_assert(file, line, false, is_fatal, assert_method, NULL, actual_str, expected_str, actual_value, expected_value);
	}
}

#define SCU_TAGS(...) \
	.tags = {__VA_ARGS__}

#define SCU_TEST(name, desc, ...) \
	static void name(void); \
	static void __attribute__((constructor)) _scu_register_##name(void) \
	{ \
		_Pragma("GCC diagnostic push"); \
		_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\""); \
		static _scu_testcase tc = {name, __LINE__, #name, (desc), ##__VA_ARGS__}; \
		_Pragma("GCC diagnostic pop"); \
		_scu_register_testcase(&tc); \
	} \
	static void name(void)

	/* Assertion functions */

#define SCU_FAIL(message) \
	_scu_handle_assert(__FILE__, __LINE__, false, false, "SCU_FAIL", message, NULL, NULL, NULL, NULL);
#define SCU_FATAL(message) \
	_scu_handle_assert(__FILE__, __LINE__, true, false, "SCU_FATAL", message, NULL, NULL, NULL, NULL);

#define _SCU_ASSERT_WITH_MESSAGE(test, is_fatal, assert_method, message, ...) \
	do { \
		_scu_account_assert(is_fatal); \
		if (!(test)) { \
			char _scu_assert_msg[1024]; \
			snprintf(_scu_assert_msg, sizeof(_scu_assert_msg), message, ##__VA_ARGS__); \
			_scu_handle_assert(__FILE__, __LINE__, is_fatal, false, assert_method, _scu_assert_msg, #test, "TRUE", NULL, NULL); \
		} \
	} while (0)

#define SCU_ASSERT_WITH_MESSAGE(test, message, ...) _SCU_ASSERT_WITH_MESSAGE(test, true, "SCU_ASSERT_WITH_MESSAGE", message, ##__VA_ARGS__)
#define SCU_ASSERT_WITH_MESSAGE_FATAL(test, message, ...) _SCU_ASSERT_WITH_MESSAGE(test, true, "SCU_ASSERT_WITH_MESSAGE_FATAL", message, ##__VA_ARGS__)

#define _SCU_ASSERT(test, is_fatal, assert_method) \
	do { \
		_scu_account_assert(is_fatal); \
		if (!(test)) { \
			_scu_handle_assert(__FILE__, __LINE__, is_fatal, false, assert_method, "", #test, "TRUE", NULL, NULL); \
		} \
	} while (0)
#define SCU_ASSERT(test) _SCU_ASSERT(test, false, "SCU_ASSERT")
#define SCU_ASSERT_FATAL(test) _SCU_ASSERT(test, true, "SCU_ASSERT_FATAL")

#define SCU_ASSERT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert(__FILE__, __LINE__, (actual) == (expected), false, "SCU_ASSERT_EQUAL", NULL, #actual, #expected, NULL, NULL); \
	} while (0)

#define SCU_ASSERT_INT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_int(__FILE__, __LINE__, (actual), (expected), false, false, "SCU_ASSERT_INT_EQUAL", #actual, #expected); \
	} while (0)

#define SCU_ASSERT_INT_NOT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_int(__FILE__, __LINE__, (actual), (expected), false, true, "SCU_ASSERT_INT_NOT_EQUAL", #actual, #expected); \
	} while (0)

#define SCU_ASSERT_NOT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert(__FILE__, __LINE__, (actual) != (expected), false, "SCU_ASSERT_NOT_EQUAL", NULL, #actual, #expected, NULL, NULL); \
	} while (0)

#define SCU_ASSERT_MEM_EQUAL(actual, expected, size) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert(__FILE__, __LINE__, (memcmp(actual, expected, size) == 0), false, "SCU_ASSERT_MEM_EQUAL", NULL, #actual, #expected, NULL, NULL); \
	} while (0)

#define SCU_ASSERT_PTR_NULL(ptr) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_ptr(__FILE__, __LINE__, (ptr), NULL, false, false, "SCU_ASSERT_PTR_NULL", #ptr, NULL); \
	} while (0)

#define SCU_ASSERT_PTR_NOT_NULL(ptr) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_ptr(__FILE__, __LINE__, (ptr), NULL, false, true, "SCU_ASSERT_PTR_NOT_NULL", #ptr, NULL); \
	} while (0)

#define SCU_ASSERT_PTR_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_ptr(__FILE__, __LINE__, (actual), (expected), false, false, "SCU_ASSERT_PTR_EQUAL", #actual, #expected); \
	} while (0)

#define SCU_ASSERT_PTR_NOT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_ptr(__FILE__, __LINE__, (actual), (expected), false, true, "SCU_ASSERT_PTR_NOT_EQUAL", #actual, #expected); \
	} while (0)

#define SCU_ASSERT_STRING_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_nstr(__FILE__, __LINE__, (actual), (expected), -1, false, false, "SCU_ASSERT_STRING_EQUAL", #actual, #expected); \
	} while (0)

#define SCU_ASSERT_NSTRING_EQUAL(actual, expected, size) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_nstr(__FILE__, __LINE__, (actual), (expected), size, false, false, "SCU_ASSERT_NSTRING_EQUAL", #actual, #expected); \
	} while (0)

/* todo */
#define SCU_ASSERT_TRUE_FATAL(val) \
	SCU_ASSERT_FATAL(val)

#define SCU_ASSERT_FALSE_FATAL(val) \
	SCU_ASSERT_FATAL(!(val))

#define SCU_ASSERT_NOT_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL((actual) != (expected))

#define SCU_ASSERT_MEM_EQUAL_FATAL(actual, expected, size) \
	SCU_ASSERT_FATAL(memcmp(actual, expected, size) == 0)

#define SCU_ASSERT_PTR_NULL_FATAL(ptr) \
	SCU_ASSERT_FATAL((ptr) == NULL)

#define SCU_ASSERT_PTR_NOT_NULL_FATAL(ptr) \
	SCU_ASSERT_FATAL((ptr) != NULL)

#define SCU_ASSERT_PTR_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL((actual) == (expected))

#define SCU_ASSERT_PTR_NOT_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL((actual) != (expected))

#define SCU_ASSERT_STRING_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL(strcmp(actual, expected) == 0)

#define SCU_ASSERT_NSTRING_EQUAL_FATAL(actual, expected, size) \
	SCU_ASSERT_FATAL(strncmp(actual, expected, size) == 0)

#ifdef __cplusplus
}
#endif

#endif
