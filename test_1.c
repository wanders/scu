
#include "scu.h"

INIT("Really lame testsuite") {
	return 0;
}

CLEANUP() {
	return 0;
}

TEST(test_num, "Test case 1") {

	CU_ASSERT(0 == 10 - 10);

}

TEST(test_korv, "Test case 13") {

	CU_ASSERT(0 == 13 - 13);

}

TEST(test_korv2, "Testcase 11") {

	CU_ASSERT(0 == 11 - 10);

}

