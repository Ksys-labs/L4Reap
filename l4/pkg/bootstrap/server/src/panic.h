/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#ifdef __cplusplus
extern "C"
#endif
void panic(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

