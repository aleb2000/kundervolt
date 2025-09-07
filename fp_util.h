/* SPDX-License-Identifier: GPL-2.0 */
/*
 * kundervolt: a kernel module to undervolt Intel-based Linux system with Secure Boot enabled.
 * Copyright © 2025  Alessandro Balducci
 *
 * The full licence notice is available in the included README.md
*/

#pragma once

#include <linux/kernel.h>
#include "test.h"

typedef int32_t intoff_t;

int atof(const char* s, size_t s_size, float* out);
float offset_int_to_mv(intoff_t offset);
size_t offset_int_to_mv_str(char* buf, size_t buf_size, intoff_t offset);
intoff_t offset_mv_to_int(float mv_offset);
int offset_mv_str_to_int(intoff_t* offset, const char* buf, size_t buf_size);

#ifdef TESTS
int run_fp_tests(void);
#endif // TESTS
