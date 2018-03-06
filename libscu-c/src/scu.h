#ifndef _SCU_H_
#define _SCU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "scu_internal.h"

/* Test module macros */

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

/* Assertion macros */

#define SCU_FAIL(message) \
	_SCU_HANDLE_ASSERT(false, false, "SCU_FAIL", message, NULL, NULL, NULL, NULL);
#define SCU_FATAL(message) \
	_SCU_HANDLE_ASSERT(false, true, "SCU_FATAL", message, NULL, NULL, NULL, NULL);

#define SCU_ASSERT_WITH_MESSAGE(cond, message, ...) _SCU_ASSERT_WITH_MESSAGE(cond, false, "SCU_ASSERT_WITH_MESSAGE", message, ##__VA_ARGS__)
#define SCU_ASSERT_WITH_MESSAGE_FATAL(cond, message, ...) _SCU_ASSERT_WITH_MESSAGE(cond, true, "SCU_ASSERT_WITH_MESSAGE_FATAL", message, ##__VA_ARGS__)

#define SCU_ASSERT(cond) _SCU_ASSERT(cond, false, "SCU_ASSERT")
#define SCU_ASSERT_FATAL(cond) _SCU_ASSERT(cond, true, "SCU_ASSERT_FATAL")

#define SCU_ASSERT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_SCU_HANDLE_ASSERT((actual) == (expected), false, "SCU_ASSERT_EQUAL", NULL, #actual, #expected, NULL, NULL); \
	} while (0)

#define SCU_ASSERT_INT_EQUAL(actual, expected) \
	do { \
		/* store to temporaries to ensure arguments only are evaluated once */ \
		typeof(actual) _scu_temp_actual = (actual); \
		typeof(expected) _scu_temp_expected = (expected); \
		_scu_account_assert(false); \
		_scu_handle_assert_int(__FILE__, __LINE__, _scu_temp_actual == _scu_temp_expected, _scu_temp_actual, sizeof (_scu_temp_actual), _scu_temp_expected, sizeof(_scu_temp_expected), false, false, "SCU_ASSERT_INT_EQUAL", #actual, #expected); \
	} while (0)

#define SCU_ASSERT_INT_NOT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_scu_handle_assert_int(__FILE__, __LINE__, (actual), (expected), false, true, "SCU_ASSERT_INT_NOT_EQUAL", #actual, #expected); \
	} while (0)

#define SCU_ASSERT_NOT_EQUAL(actual, expected) \
	do { \
		_scu_account_assert(false); \
		_SCU_HANDLE_ASSERT((actual) != (expected), false, "SCU_ASSERT_NOT_EQUAL", NULL, #actual, #expected, NULL, NULL); \
	} while (0)

#define SCU_ASSERT_MEM_EQUAL(actual, expected, size) \
	do { \
		_scu_account_assert(false); \
		_SCU_HANDLE_ASSERT((memcmp(actual, expected, size) == 0), false, "SCU_ASSERT_MEM_EQUAL", NULL, #actual, #expected, NULL, NULL); \
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

/* TODO: implement these properly */
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
