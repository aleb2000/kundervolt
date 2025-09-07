/* SPDX-License-Identifier: GPL-2.0 */
/*
 * kundervolt: a kernel module to undervolt Intel-based Linux system with Secure Boot enabled.
 * Copyright © 2025  Alessandro Balducci
 *
 * The full licence notice is available in the included README.md
*/

#pragma once

#define LOGHDR "kundervolt: "

#define UERR -1
#define UERR_OVERVOLT -2
#define UERR_RANGE -3 /* The voltage offset is out of range */
