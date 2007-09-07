/* Copyright 2007 Anders Waldenborg <anders@0x63.nu> */

SECTIONS
{
	__scu_tests_start = ADDR(.testcases.fptrs);
	__scu_tests_end = ADDR(.testcases.fptrs) + SIZEOF(.testcases.fptrs);
}
