

#include "scu.h"

INIT("Another suite") {
	return 0;
}

CLEANUP() {
	return 0;
}

TEST(test_xyz, "Some testcase") {

	CU_ASSERT(0);

}

TEST(test_abc, "Other testcase") {

	CU_ASSERT(1);

}
