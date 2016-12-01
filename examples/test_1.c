#include "scu.h"

SCU_SUITE ("Really lame testsuite");

SCU_TEST (test_num, "Test case 1") {

	SCU_ASSERT (0 == 10 - 10);

}

SCU_TEST (test_korv, "Test case 13") {

	SCU_ASSERT (0 == 13 - 13);

}

SCU_TEST (test_korv2, "Testcase 11") {

	SCU_ASSERT (0 == 11 - 10);

}
