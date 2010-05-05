/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/*
 * Copyright (c) 2010 Ali Polatel <alip@exherbo.org>
 *
 * This file is part of Pink's Tracing Library. pinktrace is free software; you
 * can redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License version 2.1, as published by the Free Software
 * Foundation.
 *
 * pinktrace is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PINKTRACE_GUARD_CHECK_PINKTRACE_H
#define PINKTRACE_GUARD_CHECK_PINKTRACE_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE 1
#endif /* !_ATFILE_SOURCE */

#include <check.h>

#include <pinktrace/pink.h>

/* Check options */
#define CHECK_OPTIONS PINK_TRACE_OPTION_SYSGOOD

/* Check bitness */
#if defined(I386) || defined(POWERPC)
#define CHECK_BITNESS (PINK_BITNESS_32)
#elif defined(X86_64) || defined(IA64) || defined(POWERPC64)
#define CHECK_BITNESS (PINK_BITNESS_64)
#else
#error unsupported architecture
#endif

Suite *
bitness_suite_create(void);

Suite *
decode_suite_create(void);

Suite *
encode_suite_create(void);

Suite *
event_suite_create(void);

Suite *
trace_suite_create(void);

Suite *
util_suite_create(void);

#endif /* !PINKTRACE_GUARD_CHECK_PINKTRACE_H */
