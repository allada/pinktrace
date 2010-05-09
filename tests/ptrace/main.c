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

#include "check_pinktrace_ptrace.h"

#include <stdlib.h>
#include <check.h>

int
main(void)
{
	int number_failed;
	SRunner *sr;

	/* Add suites */
	sr = srunner_create(bitness_suite_create());
	srunner_add_suite(sr, trace_suite_create());
	srunner_add_suite(sr, util_suite_create());
	srunner_add_suite(sr, decode_suite_create());
	srunner_add_suite(sr, encode_suite_create());
#if defined(PINKTRACE_LINUX)
	srunner_add_suite(sr, event_suite_create());
#endif /* defined(PINKTRACE_LINUX) */

	/* Run and grab the results */
	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);

	/* Cleanup and exit */
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
