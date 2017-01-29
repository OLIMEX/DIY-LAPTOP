/*
 * A lightwight way to do unittest for kernel code.
 *
 * Copyright (C) 2013 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _KERNEL_UT_H_
#define _KERNEL_UT_H_

#include <linux/kernel.h>

/* Usage:
	1. Call UT_DECLARE(), define some global variables;
	2. Define an array of ut_case_t, for example:
		static ut_case_t gs_ut_case[] = {
			{"test case1 name", NULL, ut_test_case1, NULL},
			{"test case2 name", NULL, ut_test_case2, NULL},
		};
	3. Call UT_RUN(gs_ut_case), gs_ut_case is defined in 2.
	4. Call UT_END(), display the statistics result. 	   
*/

/* This macro must be called in the global section, not in a function. */
#define UT_DECLARE() \
	static unsigned int gs_ut_ok = 0; \
	static unsigned int gs_ut_fail = 0; \
	static unsigned int gs_ut_assert_per_case = 0;

#define UT_ASSERT(value) \
	do { \
		gs_ut_assert_per_case++; \
		if (value) \
			gs_ut_ok++; \
		else { \
			gs_ut_fail++; \
			printk(KERN_NOTICE"%s, %d - (%s) failed! \n", __FILE__, __LINE__, #value); \
		} \
	} while (0)

#define UT_INIT() \
	do { \
		gs_ut_ok = 0; \
		gs_ut_fail = 0; \
	} while (0)

#define UT_RUN(testcase) \
	do { \
		int i; \
		printk(KERN_NOTICE"Run %s[] ... \n", #testcase); \
		for (i=0; i<ARRAY_SIZE(testcase); i++) { \
			gs_ut_assert_per_case = 0; \
			printk(KERN_NOTICE"Test case %d: %s \n", i, testcase[i].name); \
			if (testcase[i].init != NULL) \
				testcase[i].init(); \
			if (testcase[i].run != NULL) \
				testcase[i].run(); \
			if (testcase[i].cleanup != NULL) \
				testcase[i].cleanup(); \
			printk(KERN_NOTICE"\tAssert cnt %d. \n", gs_ut_assert_per_case); \
		} \
	} while (0)

#define UT_END() \
	printk(KERN_NOTICE"Unittest assert: Total: %u, Ok: %u, Fail: %u \n", \
		gs_ut_ok + gs_ut_fail, gs_ut_ok, gs_ut_fail)

#define UT_RESULT(buf, len) \
	snprintf(buf, len, "Unittest assert: Total: %u, Ok: %u, Fail: %u \n", \
		gs_ut_ok + gs_ut_fail, gs_ut_ok, gs_ut_fail)

typedef struct {
	char *name;
	void (*init)(void);
	void (*run)(void);
	void (*cleanup)(void);
} ut_case_t;

#endif /* end of _KERNEL_UT_H_ */

