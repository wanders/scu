#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "scu.h"

SCU_MODULE("Framework");

static size_t _before_counter = 0;
static size_t _after_counter = 0;
static char *super_complex_global_state;

SCU_SETUP()
{
	char *state = "42";
	size_t size = strlen(state);
	super_complex_global_state = malloc(size);
	memcpy(super_complex_global_state, state, size);
}

SCU_TEARDOWN()
{
	free(super_complex_global_state);
}

SCU_BEFORE_EACH()
{
	_before_counter++;
}

SCU_AFTER_EACH()
{
	_after_counter++;
}

SCU_TEST(assert_true, "Assert that should succeed")
{
	SCU_ASSERT(true);
}

SCU_TEST(assert_false, "Assert that should fail")
{
	SCU_ASSERT(false);
}

SCU_TEST(stringify, "Assert that presents the failed condition")
{
	SCU_ASSERT((5 / 2) == 3);
}

SCU_TEST(fail, "Explicitly failing test")
{
	SCU_FAIL("This should fail");
}

SCU_TEST(print_to_stdout, "Print to stdout")
{
	printf("Output some text...\n");
	printf("...and some more!\n");
	SCU_ASSERT(true);
}

SCU_TEST(print_to_stdout_fail, "Print to stdout and fail")
{
	printf("Output some text...\n");
	printf("...and some more!\n");
	SCU_FAIL("");
}

SCU_TEST(fail_with_message, "Assert that should fail with a message")
{
	SCU_ASSERT_WITH_MESSAGE(false, "The answer is %d", 42);
}

SCU_TEST(zzz, "Sleepy testcase")
{
	sleep(2);
	SCU_ASSERT(true);
}

SCU_TEST(parse_error, "Test that produces a parsing error")
{
	write(3, "sassafras", 9);
	SCU_ASSERT(true);
}

SCU_TEST(mem_equal, "Assert memory equality")
{
	char *foo = "foo";
	size_t size = strlen(foo);
	char *foo1 = malloc(size);
	char *foo2 = malloc(size);
	memcpy(foo1, foo, size);
	memcpy(foo2, foo, size);
	SCU_ASSERT_MEM_EQUAL(foo1, foo2, size);
	free(foo1);
	free(foo2);
}

SCU_TEST(mem_equal_fail, "Assert memory equality failure")
{
	SCU_ASSERT_MEM_EQUAL("foo", "bar", 3);
	unsigned char a[20];
	unsigned char b[20];
	for (size_t i = 0; i < 20; i++) {
		a[i] = i;
		b[i] = 128 - i;
	}
	SCU_ASSERT_MEM_EQUAL(a, b, 20);
}

SCU_TEST(string_equal, "Assert string equality")
{
	SCU_ASSERT_NSTRING_EQUAL("foo", "foo", 3);
	SCU_ASSERT_NSTRING_EQUAL("foo", "foo", 10);
	SCU_ASSERT_NSTRING_EQUAL("foobar", "foo", 3);
}

SCU_TEST(string_equal_fail, "Assert string equality failure")
{
	SCU_ASSERT_NSTRING_EQUAL("foo", "bar", 3);
}

SCU_TEST(setup, "Test that uses values from setup")
{
	SCU_ASSERT_NSTRING_EQUAL(super_complex_global_state, "42", 2);
}

SCU_TEST(json_escape, "Test that produces output that needs to be escaped")
{
	SCU_ASSERT_NSTRING_EQUAL("\"\\", "xx", 2);
}

SCU_TEST(before_after, "Test that makes sure before and after functions have run")
{
	SCU_ASSERT(_before_counter == 16);
	SCU_ASSERT(_after_counter == 15);
}

SCU_TEST(tags, "Test with tags", SCU_TAGS("tag1", "tag2"))
{
	SCU_ASSERT(true);
}

SCU_TEST(assert_equal_int, "Assert equal pretty printing - integer")
{
	int x = 1;
	SCU_ASSERT_INT_EQUAL(x, 2);
}

SCU_TEST(assert_equal_int_macro, "Assert equal pretty printing - integer - MACRO")
{
#define THE_MAGIC_NUMBER 42
	int x = 1;
	SCU_ASSERT_INT_EQUAL(x, THE_MAGIC_NUMBER);
}

SCU_TEST(assert_equal_unsigned_int, "Assert equal pretty printing - unsigned integer")
{
	unsigned int x = -1;
	/* these two should have same behavior (both pass) */
	SCU_ASSERT(x == -1);
	SCU_ASSERT_INT_EQUAL(x, -1);

	/* fails */
	SCU_ASSERT_INT_EQUAL(x, -2);
}

SCU_TEST(assert_equal_unsigned_int_different_types, "Assert equal pretty printing - unsigned integer of different length")
{
	unsigned char minus_one_char = -1;
	unsigned short minus_one_short = -1;
	unsigned int minus_one_int = -1;
	unsigned long minus_one_long = -1;
	unsigned long long minus_one_long_long = -1;

	SCU_ASSERT(minus_one_char == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_char, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_char, -2);
	SCU_ASSERT(minus_one_short == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_short, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_short, -2);
	SCU_ASSERT(minus_one_int == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_int, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_int, -2);
	SCU_ASSERT(minus_one_long == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long, -2);
	SCU_ASSERT(minus_one_long_long == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long_long, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long_long, -2);
}

SCU_TEST(assert_equal_signed_int_different_types, "Assert equal pretty printing - signed integer of different length")
{
	signed char minus_one_char = -1;
	signed short minus_one_short = -1;
	signed int minus_one_int = -1;
	signed long minus_one_long = -1;
	signed long long minus_one_long_long = -1;

	SCU_ASSERT(minus_one_char == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_char, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_char, -2);
	SCU_ASSERT(minus_one_short == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_short, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_short, -2);
	SCU_ASSERT(minus_one_int == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_int, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_int, -2);
	SCU_ASSERT(minus_one_long == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long, -2);
	SCU_ASSERT(minus_one_long_long == -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long_long, -1);
	SCU_ASSERT_INT_EQUAL(minus_one_long_long, -2);
}

SCU_TEST(assert_equal_string, "Assert equal pretty printing - string")
{
	const char *x = "foo";
	SCU_ASSERT_STRING_EQUAL(x, "bar");
}

SCU_TEST(assert_equal_string_with_strange_chars, "Assert equal pretty printing - string with strange chars")
{
#define MAGIC_VALUE "bar\xff"
	const char *x = "foo\n";
	SCU_ASSERT_STRING_EQUAL(x, MAGIC_VALUE);
}

SCU_TEST(assert_ptr_null, "Assert pointer null")
{
	const char *x = "foo";
	const char *y = NULL;
	SCU_ASSERT_PTR_NULL(x);
	SCU_ASSERT_PTR_NULL(y);
}

SCU_TEST(assert_ptr_not_null, "Assert pointer not null")
{
	const char *x = "foo";
	const char *y = NULL;
	SCU_ASSERT_PTR_NOT_NULL(x);
	SCU_ASSERT_PTR_NOT_NULL(y);
}

SCU_TEST(assert_ptr_equal, "Assert pointer equal")
{
	const char *x = "foo";
	const char *y = NULL;
	SCU_ASSERT_PTR_EQUAL(x, x);
	SCU_ASSERT_PTR_EQUAL(y, y);
	SCU_ASSERT_PTR_EQUAL(x, y);
}

SCU_TEST(assert_ptr_not_equal, "Assert pointer not equal")
{
	const char *x = "foo";
	const char *y = NULL;
	SCU_ASSERT_PTR_NOT_EQUAL(x, x);
	SCU_ASSERT_PTR_NOT_EQUAL(y, y);
	SCU_ASSERT_PTR_NOT_EQUAL(x, y);
}
