// SPDX-License-Identifier: GPL-2.0
//
// kundervolt: a kernel module to undervolt Intel-based Linux system with Secure Boot enabled.
// Copyright © 2025  Alessandro Balducci
//
// The full licence notice is available in the included README.md

#include "asm/fpu/api.h"
#include "common.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include "fp_util.h"
#include "ftoa/ftoa.h"
#include "test.h"

#define VOLTAGE_HIGH_BITS_MASK 0xFFE00000

// Not the ideal way to round but it's easy
#define ROUND(x) ((int)((x < 0) ? (x - 0.5) : (x + 0.5)))

static int ctod(char c) {
	if (c < '0' || c > '9') {
		return UERR;
	}
	return c - '0';
}

/**
 * Converts ascii string into float.
 *
 * Not a great implementation but it works well enough under our constraints.
 *
 * Must always be called inside `kernel_fpu_begin/end()` guards.
*/
int atof(const char* s, size_t s_size, float* out) {
	int integer = 0;
	float decimal = 0;
	int i = 0;
	char c;
	bool integer_finished = false;
	int sign = 1;
	size_t cur_decimal_place = 0;
	while (i < s_size && (c = s[i])) {
		if (i == 0 && c == '-') {
			sign = -1;
			i++;
			continue;
		}

		if (!integer_finished) {
			if (c == '.') {
				integer_finished = true;
			} else {
				int d = ctod(c);
				if (d == UERR) {
					// Invalid character
					pr_err("Invalid character (%d)\n", c);
					return UERR;
				}

				int tmp;
				if (check_mul_overflow(integer, 10, &tmp)) {
					pr_err("Overflow mul\n");
					return UERR;
				}
				if (check_add_overflow(tmp, d, &integer)) {
					pr_err("Overflow add\n");
					return UERR;
				}
			}
		} else {
			int d = ctod(c);
			if (d == UERR) {
				// Invalid character
				pr_err("Invalid character (%d)\n", c);
				return UERR;
			}
			int divisor = 10;
			for (int j = 0; j < cur_decimal_place; j++) {
				divisor *= 10;
			}

			decimal += (float) d / divisor;
			cur_decimal_place++;
		}
		i++;
	}

	*out = integer;
	*out += decimal;
	*out *= sign;

	return 0;
}

/**
 * Offset calculation obtained from https://github.com/mihic/linux-intel-undervolt
 *
 * Arguments:
 *	mv_offset, voltage offset in millivolts
 *
 * Returns the voltage as an integer value compatible with the MSR 0x150 format
 *
 * Steps:
 * 1. Multiply by 1.024
 * 2. Round to nearest integer
 * 3. Shift left by 21
 * 4. Only retain the high 11 bits
*/
intoff_t offset_mv_to_int(float mv_offset) {
	kernel_fpu_begin();
	int rounded_product = ROUND(mv_offset * 1.024);
	kernel_fpu_end();
	return VOLTAGE_HIGH_BITS_MASK & (rounded_product << 21);
}

/**
 * Inverse of `voltage_mv_to_int()`
 *
 * It must ALWAYS be used inside `kernel_fpu_begin/end()` guards
*/
float offset_int_to_mv(intoff_t offset) {
	return (offset >> 21) / 1.024;
}

size_t offset_int_to_mv_str(char* buf, size_t buf_size, intoff_t offset) {
	kernel_fpu_begin();
	float mv = offset_int_to_mv(offset);
	size_t written = ftoa(buf, buf_size, mv, 2);
	kernel_fpu_end();
	return written;
}

int offset_mv_str_to_int(intoff_t* offset, const char* buf, size_t buf_size) {
	float mv;
	kernel_fpu_begin();
	int ret = atof(buf, buf_size, &mv);
	kernel_fpu_end();
	if (ret) {
		return UERR;
	}
	if (mv > 0) {
		return UERR_OVERVOLT;
	}

	*offset = offset_mv_to_int(mv);
	return 0;
}

#ifdef TESTS

static int offset_mv_to_int_test1(void) {
	ASSERT_EQ_HEX(offset_mv_to_int(-50), 0xF9A00000);
	return 0;
}

static int offset_mv_to_int_test2(void) {
	ASSERT_EQ_HEX(offset_mv_to_int(-150.4), 0xECC00000);
	return 0;
}

static int offset_mv_to_int_test3(void) {
	ASSERT_EQ_HEX(offset_mv_to_int(-125.0), 0xF0000000);
	return 0;
}

static int offset_mv_to_int_test4(void) {
	ASSERT_EQ_HEX(offset_mv_to_int(-4), 0xFF800000);
	return 0;
}

static int offset_int_to_mv_test(void) {
	for (int i = -999; i < 1000; i++) {
		kernel_fpu_begin();
		int offset = offset_mv_to_int(i);
		float offset_mv = offset_int_to_mv(offset);
		int reconverted_offset = offset_mv_to_int(offset_mv);
		kernel_fpu_end();
		ASSERT_EQ_HEX(offset, reconverted_offset);
	}
	return 0;
}

static int atof_test1(void) {
	kernel_fpu_begin();
	float f;
	int res = atof("0.0", 4, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, 0);
	ASSERT_EQ(f, 0.0);
	return 0;
}

static int atof_test2(void) {
	kernel_fpu_begin();
	float f;
	int res = atof(".5", 3, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, 0);
	ASSERT_EQ(f, 0.5);
	return 0;
}

static int atof_test3(void) {
	kernel_fpu_begin();
	float f;
	int res = atof("-50.25", 7, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, 0);
	ASSERT_EQ(f, -50.25);
	return 0;
}

static int atof_test4(void) {
	kernel_fpu_begin();
	float f;
	int res = atof("196.75", 7, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, 0);
	ASSERT_EQ(f, 196.75);
	return 0;
}

static int atof_test5(void) {
	kernel_fpu_begin();
	float f;
	int res = atof("-999", 5, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, 0);
	ASSERT_EQ(f, -999);
	return 0;
}

static int atof_test_error1(void) {
	kernel_fpu_begin();
	float f;
	int res = atof("1.0.4", 6, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, -1);
	return 0;
}

static int atof_test_error2(void) {
	kernel_fpu_begin();
	float f;
	int res = atof("11.55asd", 9, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, -1);
	return 0;
}

static int atof_test_error3(void) {
	kernel_fpu_begin();
	float f;
	int res = atof("--1", 4, &f);
	kernel_fpu_end();
	ASSERT_EQ_INT(res, -1);
	return 0;
}

static int offset_mv_str_to_int_test1(void) {
	int offset;
	int ret = offset_mv_str_to_int(&offset, "-50", 4);
	ASSERT_EQ_INT(ret, 0);
	ASSERT_EQ_HEX(offset, 0xF9A00000);
	return 0;
}
static int offset_mv_str_to_int_test2(void) {
	int offset;
	int ret = offset_mv_str_to_int(&offset, "-150.4", 7);
	ASSERT_EQ_INT(ret, 0);
	ASSERT_EQ_HEX(offset, 0xECC00000);
	return 0;
}

static int offset_mv_str_to_int_test3(void) {
	int offset;
	int ret = offset_mv_str_to_int(&offset, "-125.0", 7);
	ASSERT_EQ_INT(ret, 0);
	ASSERT_EQ_HEX(offset, 0xF0000000);
	return 0;
}

static int offset_mv_str_to_int_test4(void) {
	int offset;
	int ret = offset_mv_str_to_int(&offset, "-4", 3);
	ASSERT_EQ_INT(ret, 0);
	ASSERT_EQ_HEX(offset, 0xFF800000);
	return 0;
}

int run_fp_tests(void) {
	INIT_TEST_SUITE("Floating Point utilities")

	RUN_TEST(offset_mv_to_int_test1)
	RUN_TEST(offset_mv_to_int_test2)
	RUN_TEST(offset_mv_to_int_test3)
	RUN_TEST(offset_mv_to_int_test4)
	RUN_TEST(offset_int_to_mv_test)
	RUN_TEST(atof_test1)
	RUN_TEST(atof_test2)
	RUN_TEST(atof_test3)
	RUN_TEST(atof_test4)
	RUN_TEST(atof_test5)
	RUN_TEST(atof_test_error1)
	RUN_TEST(atof_test_error2)
	RUN_TEST(atof_test_error3)
	RUN_TEST(offset_mv_str_to_int_test1)
	RUN_TEST(offset_mv_str_to_int_test2)
	RUN_TEST(offset_mv_str_to_int_test3)
	RUN_TEST(offset_mv_str_to_int_test4)

	END_TEST_SUITE
}

#endif // TESTS
