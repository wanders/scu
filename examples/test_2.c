#include <unistd.h>

#include "scu.h"

SCU_SUITE ("Another suite");

SCU_TEST (test_xyz, "Some testcase") {

	SCU_ASSERT (0);

}

SCU_TEST (test_abc, "Other testcase") {

	printf ("Output lots of text...\n");
	printf ("...and some more text!\n");
	SCU_FAIL ("This should fail");

}

SCU_TEST (test_zzz, "Sleepy testcase") {

	sleep (2);

	SCU_ASSERT (1);

}
