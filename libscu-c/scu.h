#ifndef _SCU_H_
#define _SCU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Configuration parameters */

#define _SCU_MAX_TESTS 4096
#define _SCU_MAX_TAGS 128
#define _SCU_MAX_FAILURES 1024
#define _SCU_FAILURE_MESSAGE_LENGTH 2048

/* Helper macros */

#define STRINGIFY_NOEXPAND(x) #x
#define STRINGIFY(x) STRINGIFY_NOEXPAND(x)

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
	char msg[_SCU_FAILURE_MESSAGE_LENGTH];
} _scu_failure;

static bool *_scu_success __attribute__((used));
static size_t *_scu_asserts __attribute__((used));
static size_t *_scu_num_failures __attribute__((used));
static _scu_failure *_scu_failures __attribute__((used));

/* Test case definition */

typedef struct {
	void (*func)(bool *, size_t *, size_t *, _scu_failure *);
	int line;
	const char *name;
	const char *desc;
	const char *tags[_SCU_MAX_TAGS];
} _scu_testcase;

void _scu_register_testcase(_scu_testcase *);

#define SCU_TAGS(...) \
	.tags = {__VA_ARGS__}

#define SCU_TEST(name, desc, ...)                                                                             \
	static void name(void);                                                                               \
	static void _scu_test_wrapper_##name(bool *success, size_t *asserts, size_t *num_failures,            \
	                                     _scu_failure *failures)                                          \
	{                                                                                                     \
		_scu_success = success;                                                                       \
		_scu_asserts = asserts;                                                                       \
		_scu_num_failures = num_failures;                                                             \
		_scu_failures = failures;                                                                     \
		name();                                                                                       \
	}                                                                                                     \
	static void __attribute__((constructor)) _scu_register_##name(void)                                   \
	{                                                                                                     \
		_Pragma("GCC diagnostic push");                                                               \
		_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"");                           \
		static _scu_testcase tc = {_scu_test_wrapper_##name, __LINE__, #name, (desc), ##__VA_ARGS__}; \
		_Pragma("GCC diagnostic pop");                                                                \
		_scu_register_testcase(&tc);                                                                  \
	}                                                                                                     \
	static void name(void)

/* Test case addresses */

/* Assertion functions */

void _scu_handle_fatal_assert(void) __attribute__((noreturn));
void _scu_fatal_assert_allowed(void);

#define SCU_FAIL(message)                                                                                           \
	do {                                                                                                        \
		*_scu_success = false;                                                                              \
		if (*_scu_num_failures < _SCU_MAX_FAILURES) {                                                       \
			_scu_failures[*_scu_num_failures].file = __FILE__;                                          \
			_scu_failures[*_scu_num_failures].line = __LINE__;                                          \
			strncpy(_scu_failures[*_scu_num_failures].msg, (message), _SCU_FAILURE_MESSAGE_LENGTH - 1); \
			_scu_failures[*_scu_num_failures].msg[_SCU_FAILURE_MESSAGE_LENGTH - 1] = 0;                 \
			(*_scu_num_failures)++;                                                                     \
		}                                                                                                   \
	} while (0)

#define _SCU_ASSERT_WITH_MESSAGE(test, is_fatal, message, ...)                            \
	do {                                                                              \
		(*_scu_asserts)++;                                                        \
		if (!(test)) {                                                            \
			char _scu_fmsg[_SCU_FAILURE_MESSAGE_LENGTH];                      \
			snprintf(_scu_fmsg, sizeof(_scu_fmsg), (message), ##__VA_ARGS__); \
			SCU_FAIL(_scu_fmsg);                                              \
			if (is_fatal)                                                     \
				_scu_handle_fatal_assert();                               \
		}                                                                         \
	} while (0)

#define SCU_ASSERT_WITH_MESSAGE(test, message, ...) _SCU_ASSERT_WITH_MESSAGE(test, false, ##__VA_ARGS__)
#define SCU_ASSERT_WITH_MESSAGE_FATAL(test, message, ...) _SCU_ASSERT_WITH_MESSAGE(test, true, ##__VA_ARGS__)

#define _SCU_ASSERT(test, is_fatal)                                                                   \
	do {                                                                                          \
		if (is_fatal)                                                                         \
			_scu_fatal_assert_allowed();                                                  \
		_SCU_ASSERT_WITH_MESSAGE((test), is_fatal, "assertion failure: %s", STRINGIFY(test)); \
	} while (0)

#define SCU_ASSERT(test) _SCU_ASSERT(test, false)
#define SCU_ASSERT_FATAL(test) _SCU_ASSERT(test, true)

/* Convenience assertion macros */

#define SCU_ASSERT_TRUE(val) \
	SCU_ASSERT(val)

#define SCU_ASSERT_FALSE(val) \
	SCU_ASSERT(!(val))

#define SCU_ASSERT_EQUAL(actual, expected) \
	SCU_ASSERT((actual) == (expected))

#define SCU_ASSERT_NOT_EQUAL(actual, expected) \
	SCU_ASSERT((actual) != (expected))

#define SCU_ASSERT_MEM_EQUAL(actual, expected, size) \
	SCU_ASSERT(memcmp((actual), (expected), (size)) == 0)

#define SCU_ASSERT_PTR_NULL(ptr) \
	SCU_ASSERT((ptr) == NULL)

#define SCU_ASSERT_PTR_NOT_NULL(ptr) \
	SCU_ASSERT((ptr) != NULL)

#define SCU_ASSERT_PTR_EQUAL(actual, expected) \
	SCU_ASSERT((actual) == (expected))

#define SCU_ASSERT_PTR_NOT_EQUAL(actual, expected) \
	SCU_ASSERT((actual) != (expected))

#define SCU_ASSERT_STRING_EQUAL(actual, expected) \
	SCU_ASSERT(strcmp((actual), (expected)) == 0)

#define SCU_ASSERT_NSTRING_EQUAL(actual, expected, size) \
	SCU_ASSERT(strncmp((actual), (expected), (size)) == 0)

#define SCU_ASSERT_TRUE_FATAL(val) \
	SCU_ASSERT_FATAL(val)

#define SCU_ASSERT_FALSE_FATAL(val) \
	SCU_ASSERT_FATAL(!(val))

#define SCU_ASSERT_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL((actual) == (expected))

#define SCU_ASSERT_NOT_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL((actual) != (expected))

#define SCU_ASSERT_MEM_EQUAL_FATAL(actual, expected, size) \
	SCU_ASSERT_FATAL(memcmp((actual), (expected), (size)) == 0)

#define SCU_ASSERT_PTR_NULL_FATAL(ptr) \
	SCU_ASSERT_FATAL((ptr) == NULL)

#define SCU_ASSERT_PTR_NOT_NULL_FATAL(ptr) \
	SCU_ASSERT_FATAL((ptr) != NULL)

#define SCU_ASSERT_PTR_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL((actual) == (expected))

#define SCU_ASSERT_PTR_NOT_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL((actual) != (expected))

#define SCU_ASSERT_STRING_EQUAL_FATAL(actual, expected) \
	SCU_ASSERT_FATAL(strcmp((actual), (expected)) == 0)

#define SCU_ASSERT_NSTRING_EQUAL_FATAL(actual, expected, size) \
	SCU_ASSERT_FATAL(strncmp((actual), (expected), (size)) == 0)

#ifdef __cplusplus
}
#endif

#endif
