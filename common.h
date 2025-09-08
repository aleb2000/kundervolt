/* SPDX-License-Identifier: GPL-2.0 */
/*
 * kundervolt: a kernel module to undervolt Intel-based Linux system with Secure Boot enabled.
 * Copyright © 2025  Alessandro Balducci
 *
 * The full licence notice is available in the included README.md
*/

#pragma once

// When defined, guards are put in place to prevent overvolting
#define LOCK_OVERVOLT

#define LOGHDR "kundervolt: "

#define UERR -1
#define UERR_RANGE -2 /* The voltage offset is out of range */

#ifdef LOCK_OVERVOLT
#define UERR_OVERVOLT -3
#endif
