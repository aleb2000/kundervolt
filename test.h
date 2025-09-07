/* SPDX-License-Identifier: GPL-2.0 */
/*
 * kundervolt: a kernel module to undervolt Intel-based Linux system with Secure Boot enabled.
 * Copyright © 2025  Alessandro Balducci
 *
 * The full licence notice is available in the included README.md
*/


#pragma once

// #define TESTS

#ifdef TESTS

#define INIT_TEST_SUITE(suite_name)         \
	pr_info("Test suite " suite_name "\n"); \
	int total = 0;                          \
	int fail = 0;

#define END_TEST_SUITE                                    \
	pr_info("%d/%d tests passed\n", total - fail, total); \
	return 0;

#define RUN_TEST(name)                       \
	total++;                                 \
	if (name() != 0) {                       \
		fail++;                              \
		pr_alert("Test " #name " failed\n"); \
	}

#define ASSERT_EQ(a, b)                                   \
	if (a != b) {                                         \
		pr_alert("Assertion " #a " == " #b " failed!\n"); \
		return 1;                                         \
	}

#define ASSERT_EQ_FMT(a, b, a_fmt, b_fmt)                    \
	if (a != b) {                                            \
		pr_alert("Assertion " #a " == " #b " failed!\n");    \
		pr_alert(a_fmt " does not equal " b_fmt "\n", a, b); \
		return 1;                                            \
	}

#define ASSERT_EQ_INT(a, b) ASSERT_EQ_FMT(a, b, "%d", "%d")
#define ASSERT_EQ_HEX(a, b) ASSERT_EQ_FMT(a, b, "0x%x", "0x%x")
#define ASSERT_EQ_LHEX(a, b) ASSERT_EQ_FMT(a, b, "0x%llx", "0x%llx")

#endif // TESTS
