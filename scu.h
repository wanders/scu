#ifndef _SCU_H_
#define _SCU_H_

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Configuration parameters */

#define _SCU_MAX_FAILURES 1024
#define _SCU_FAILURE_MESSAGE_LENGTH 1024

/* Helper macros */

#define STRINGIFY_NOEXPAND(x) #x
#define STRINGIFY(x) STRINGIFY_NOEXPAND(x)

/* Test module functions */

void _scu_setup (void);
void _scu_teardown (void);
void _scu_before_each (void);
void _scu_after_each (void);

#define SCU_MODULE(name) \
	const char *_scu_module_name = name

#define SCU_SETUP() \
	void _scu_setup (void)

#define SCU_TEARDOWN() \
	void _scu_teardown (void)

#define SCU_BEFORE_EACH() \
	void _scu_before_each (void)

#define SCU_AFTER_EACH() \
	void _scu_after_each (void)

/* Test module summary variable pointers */

typedef struct {
	const char *file;
	int line;
	char msg[_SCU_FAILURE_MESSAGE_LENGTH];
} _scu_failure;

static bool *_scu_success __attribute__ ((used));
static size_t *_scu_asserts __attribute__ ((used));
static size_t *_scu_num_failures __attribute__ ((used));
static _scu_failure *_scu_failures __attribute__ ((used));

/* Test case definition */

typedef struct {
	void (*func) (bool *, size_t *, size_t *, _scu_failure *);
	const char *desc;
} _scu_testcase;

#define SCU_TEST(name, desc) \
static void _scu_test_func_##name (void); \
static void _scu_test_func_##name##_wrapper (bool *success, size_t *asserts, size_t *num_failures, \
                                             _scu_failure *failures) \
{ \
	_scu_success = success; \
	_scu_asserts = asserts; \
	_scu_num_failures = num_failures; \
	_scu_failures = failures; \
	_scu_test_func_##name (); \
} \
static _scu_testcase _scu_testcase_##name __attribute__ ((used)) \
    __attribute__ ((section (".scu.testcases"))) = {_scu_test_func_##name##_wrapper, desc}; \
static void _scu_test_func_##name (void)

/* Test case addresses */

extern _scu_testcase _scu_testcases_start;
extern _scu_testcase _scu_testcases_end;

/* Assertion functions */

#define SCU_FAIL(message) \
	do { \
		*_scu_success = false; \
		if (*_scu_num_failures < _SCU_MAX_FAILURES) { \
			_scu_failures[*_scu_num_failures].file = __FILE__; \
			_scu_failures[*_scu_num_failures].line = __LINE__; \
			strncpy (_scu_failures[*_scu_num_failures].msg, (message), _SCU_FAILURE_MESSAGE_LENGTH - 1); \
			_scu_failures[*_scu_num_failures].msg[_SCU_FAILURE_MESSAGE_LENGTH - 1] = 0; \
			(*_scu_num_failures)++; \
		} \
	} while (0)

#define SCU_ASSERT(test) \
	do { \
		(*_scu_asserts)++; \
		if (!(test)) { \
			SCU_FAIL ("assertion failure: " STRINGIFY (test)); \
		} \
	} while (0)

#define SCU_ASSERT_NSTRING_EQUAL(a, b, size) \
	SCU_ASSERT (strncmp ((const char *)a, (const char *)b, size) == 0)

#define SCU_ASSERT_MEM_EQUAL(a, b, size) \
	SCU_ASSERT (memcmp (a, b, size) == 0)

#endif
