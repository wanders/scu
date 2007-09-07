#ifndef __SCU_H_
#define __SCU_H_

/* Copyright 2007 Anders Waldenborg <anders@0x63.nu> */

#include "CUnit/Basic.h"

#define SCU_CLEANUP_NAME __scu_cleanup
#define SCU_INIT_NAME __scu_init
#define SCU_NAME_NAME __scu_name
#define SCU_LIST_START_NAME __scu_tests_start
#define SCU_LIST_END_NAME __scu_tests_end

struct scutestcase {
	CU_TestFunc func;
	char *desc;
};

#define TEST(name, desc)						\
	static void name (void);					\
	static struct scutestcase __scu_test_##name			\
	  __attribute__((__used__))					\
	  __attribute__ ((__section__ (".testcases.fptrs"))) =		\
	  {name, desc};							\
	static void name (void)

#define INIT(desc)				\
	char SCU_NAME_NAME[] = desc;		\
	int SCU_INIT_NAME (void)

#define CLEANUP(desc)				\
	int SCU_CLEANUP_NAME (void)

#endif
