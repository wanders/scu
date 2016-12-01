/*
 * This is supposed to be a 1:1 mapping of
 * http://cunit.sourceforge.net/example.html
 */

#include <stdio.h>
#include <string.h>
#include "scu.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
SCU_SUITE ("Suite_1");

SCU_SETUP ()
{
  temp_file = fopen ("temp.txt", "w+");
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
SCU_TEARDOWN ()
{
  if (0 != fclose (temp_file)) {
    return;
  }
  else {
    temp_file = NULL;
  }
}

/* Simple test of fprintf().
 * Writes test data to the temporary file and checks
 * whether the expected number of bytes were written.
 */
SCU_TEST (test_fprintf, "test of fprintf()")
{
   int i1 = 10;

   if (NULL != temp_file) {
      SCU_ASSERT(2 == fprintf (temp_file, "Q\n"));
      SCU_ASSERT(7 == fprintf (temp_file, "i1 = %d", i1));
   }
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
SCU_TEST (test_fread, "test of fread()")
{
   char buffer[20];

   if (NULL != temp_file) {
      rewind(temp_file);
      SCU_ASSERT (9 == fread (buffer, sizeof(unsigned char), 20, temp_file));
      SCU_ASSERT_NSTRING_EQUAL (buffer, "Q\ni1 = 10", 9);
   }
}
