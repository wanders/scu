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
		static _scu_testcase tc = {name, 0, #name, (desc), ##__VA_ARGS__}; \
		_Pragma("GCC diagnostic pop"); \
		_scu_register_testcase(&tc); \
	} \
	static void name(void)

/* Assertion macros */

#define SCU_FAIL(msg) \
	_SCU_HANDLE_FAILURE("SCU_FAIL", msg, NULL, NULL, NULL, NULL, false);
#define SCU_FATAL(msg) \
	_SCU_HANDLE_FAILURE("SCU_FATAL", msg, NULL, NULL, NULL, NULL, true);

#define SCU_ASSERT_WITH_MESSAGE(cond, msg, ...) _SCU_ASSERT_WITH_MESSAGE(cond, "SCU_ASSERT_WITH_MESSAGE", #cond, false, msg, ##__VA_ARGS__)
#define SCU_ASSERT_WITH_MESSAGE_FATAL(cond, msg, ...) _SCU_ASSERT_WITH_MESSAGE(cond, "SCU_ASSERT_WITH_MESSAGE_FATAL", #cond, true, msg, ##__VA_ARGS__)

#define SCU_ASSERT(cond) _SCU_ASSERT(cond, "SCU_ASSERT", #cond, false)
#define SCU_ASSERT_FATAL(cond) _SCU_ASSERT(cond, "SCU_ASSERT_FATAL", #cond, true)

#define SCU_ASSERT_TRUE(cond) \
	_SCU_ASSERT(cond, "SCU_ASSERT_TRUE", #cond, false)
#define SCU_ASSERT_TRUE_FATAL(cond) \
	_SCU_ASSERT(cond, "SCU_ASSERT_TRUE_FATAL", #cond, true)
#define SCU_ASSERT_FALSE(cond) \
	_SCU_ASSERT(!(cond), "SCU_ASSERT_FALSE", #cond, false)
#define SCU_ASSERT_FALSE_FATAL(cond) \
	_SCU_ASSERT(!(cond), "SCU_ASSERT_FALSE_FATAL", #cond, true)

#define SCU_ASSERT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL("SCU_ASSERT_EQUAL", #actual, #expected, actual, expected, false, false)
#define SCU_ASSERT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL("SCU_ASSERT_EQUAL_FATAL", #actual, #expected, actual, expected, false, true)
#define SCU_ASSERT_NOT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL("SCU_ASSERT_NOT_EQUAL", #actual, #expected, actual, expected, true, false)
#define SCU_ASSERT_NOT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL("SCU_ASSERT_NOT_EQUAL_FATAL", #actual, #expected, actual, expected, true, true)

#define SCU_ASSERT_INT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_INT("SCU_ASSERT_INT_EQUAL", #actual, #expected, actual, expected, false, false)
#define SCU_ASSERT_INT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_INT("SCU_ASSERT_INT_EQUAL_FATAL", #actual, #expected, actual, expected, false, true)
#define SCU_ASSERT_INT_NOT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_INT("SCU_ASSERT_INT_NOT_EQUAL", #actual, #expected, actual, expected, true, false)
#define SCU_ASSERT_INT_NOT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_INT("SCU_ASSERT_INT_NOT_EQUAL_FATAL", #actual, #expected, actual, expected, true, true)

#define SCU_ASSERT_FLOAT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_FLOAT("SCU_ASSERT_FLOAT_EQUAL", #actual, #expected, actual, expected, false, false)
#define SCU_ASSERT_FLOAT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_FLOAT("SCU_ASSERT_FLOAT_EQUAL_FATAL", #actual, #expected, actual, expected, false, true)
#define SCU_ASSERT_FLOAT_NOT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_FLOAT("SCU_ASSERT_FLOAT_NOT_EQUAL", #actual, #expected, actual, expected, true, false)
#define SCU_ASSERT_FLOAT_NOT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_FLOAT("SCU_ASSERT_FLOAT_NOT_EQUAL_FATAL", #actual, #expected, actual, expected, true, true)

#define SCU_ASSERT_PTR_NULL(ptr) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_NULL", #ptr, NULL, ptr, NULL, false, false)
#define SCU_ASSERT_PTR_NULL_FATAL(ptr) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_NULL_FATAL", #ptr, NULL, ptr, NULL, false, true)
#define SCU_ASSERT_PTR_NOT_NULL(ptr) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_NOT_NULL", #ptr, NULL, ptr, NULL, true, false)
#define SCU_ASSERT_PTR_NOT_NULL_FATAL(ptr) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_NOT_NULL_FATAL", #ptr, NULL, ptr, NULL, true, true)
#define SCU_ASSERT_PTR_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_EQUAL", #actual, #expected, actual, expected, false, false)
#define SCU_ASSERT_PTR_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_EQUAL_FATAL", #actual, #expected, actual, expected, false, true)
#define SCU_ASSERT_PTR_NOT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_NOT_EQUAL", #actual, #expected, actual, expected, true, false)
#define SCU_ASSERT_PTR_NOT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_POINTER("SCU_ASSERT_PTR_NOT_EQUAL_FATAL", #actual, #expected, actual, expected, true, true)

#define SCU_ASSERT_STRING_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_STRING_EQUAL", #actual, #expected, actual, expected, -1, false, false)
#define SCU_ASSERT_STRING_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_STRING_EQUAL_FATAL", #actual, #expected, actual, expected, -1, false, true)
#define SCU_ASSERT_STRING_NOT_EQUAL(actual, expected) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_STRING_NOT_EQUAL", #actual, #expected, actual, expected, -1, true, false)
#define SCU_ASSERT_STRING_NOT_EQUAL_FATAL(actual, expected) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_STRING_NOT_EQUAL_FATAL", #actual, #expected, actual, expected, -1, true, true)
#define SCU_ASSERT_NSTRING_EQUAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_NSTRING_EQUAL", #actual, #expected, actual, expected, size, false, false)
#define SCU_ASSERT_NSTRING_EQUAL_FATAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_NSTRING_EQUAL_FATAL", #actual, #expected, actual, expected, size, false, true)
#define SCU_ASSERT_NSTRING_NOT_EQUAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_NSTRING_NOT_EQUAL", #actual, #expected, actual, expected, size, true, false)
#define SCU_ASSERT_NSTRING_NOT_EQUAL_FATAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_STRING("SCU_ASSERT_NSTRING_NOT_EQUAL_FATAL", #actual, #expected, actual, expected, size, true, true)

#define SCU_ASSERT_MEM_EQUAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_MEMORY("SCU_ASSERT_MEM_EQUAL", #actual, #expected, actual, expected, size, false, false)
#define SCU_ASSERT_MEM_EQUAL_FATAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_MEMORY("SCU_ASSERT_MEM_EQUAL_FATAL", #actual, #expected, actual, expected, size, false, true)
#define SCU_ASSERT_MEM_NOT_EQUAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_MEMORY("SCU_ASSERT_MEM_NOT_EQUAL", #actual, #expected, actual, expected, size, true, false)
#define SCU_ASSERT_MEM_NOT_EQUAL_FATAL(actual, expected, size) \
	_SCU_ASSERT_EQUAL_MEMORY("SCU_ASSERT_MEM_NOT_EQUAL_FATAL", #actual, #expected, actual, expected, size, true, true)

#ifdef __cplusplus
}
#endif

#endif
