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

#include "check_pinktrace.h"

#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <check.h>

#include <pinktrace/pink.h>

START_TEST(t_event_stop)
{
	int status;
	pid_t pid;
	pink_event_t event;
	pink_error_t error;

	if ((pid = pink_fork(CHECK_OPTIONS, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) /* child */
		kill(getpid(), SIGSTOP);
	else { /* parent */
		/* Resume the child, it will stop with a SIGSTOP. */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_STOP,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_STOP, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_syscall)
{
	int status;
	pid_t pid;
	pink_event_t event;
	pink_error_t error;

	if ((pid = pink_fork(CHECK_OPTIONS, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) { /* child */
		/* At this point Glibc may have cached getpid() so we call it
		 * using syscall(2).
		 */
		syscall(SYS_getpid);
	}
	else { /* parent */
		/* Resume the child and arrange it to be stopped at the next
		 * system call. */
		fail_unless(pink_trace_syscall(pid, 0),
				"pink_trace_syscall failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_SYSCALL,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_SYSCALL, event);

		pink_trace_kill(pid);
	}
}
END_TEST


START_TEST(t_event_fork)
{
	int status;
	pid_t pid, cpid;
	pink_event_t event;
	pink_error_t error;

	if ((pid = pink_fork(CHECK_OPTIONS | PINK_TRACE_OPTION_FORK, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) { /* child */
		if ((cpid = fork()) < 0)
			_exit(-1);
		_exit(0);
	}
	else { /* parent */
		/* Resume the child, it will stop at the next fork(2) call. */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_FORK,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_FORK, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_vfork)
{
	int status;
	pid_t pid, cpid;
	pink_event_t event;
	pink_error_t error;

	if ((pid = pink_fork(CHECK_OPTIONS | PINK_TRACE_OPTION_VFORK, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) { /* child */
		if ((cpid = vfork()) < 0)
			_exit(-1);
		_exit(0);
	}
	else { /* parent */
		/* Resume the child, it will stop at the next fork(2) call. */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_VFORK,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_VFORK, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_clone)
{
}
END_TEST

START_TEST(t_event_vfork_done)
{
}
END_TEST

START_TEST(t_event_exec)
{
	int status;
	pid_t pid;
	pink_event_t event;
	pink_error_t error;
	char *myargv[] = { "/bin/true", NULL };
	char *myenviron[] = { NULL };

	if ((pid = pink_fork(CHECK_OPTIONS | PINK_TRACE_OPTION_EXEC, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) /* child */
		execve(myargv[0], myargv, myenviron);
	else { /* parent */
		/* Resume the child, it will stop at the beginning of execve(2). */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_EXEC,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_EXEC, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_exit)
{
	int status;
	pid_t pid;
	pink_event_t event;
	pink_error_t error;

	if ((pid = pink_fork(CHECK_OPTIONS | PINK_TRACE_OPTION_EXIT, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) /* child */
		_exit(0);
	else { /* parent */
		/* Resume the child, it will stop at the beginning of exit(2). */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_EXIT,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_EXIT, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_genuine)
{
	int status;
	pid_t pid;
	pink_event_t event;
	pink_error_t error;

	if ((pid = pink_fork(CHECK_OPTIONS, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) /* child */
		kill(getpid(), SIGINT);
	else { /* parent */
		/* Resume the child, it will send itself a signal. */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_GENUINE,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_EXIT, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_exit_genuine)
{
	int status;
	pid_t pid;
	pink_event_t event;
	pink_error_t error;

	/* Do NOT set EXIT option, we want the exit(2) to be genuine.
	 */
	if ((pid = pink_fork(CHECK_OPTIONS, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) /* child */
		_exit(0);
	else { /* parent */
		/* Resume the child, it will stop at the exit(2). */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_EXIT_GENUINE,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_EXIT_GENUINE, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_exit_signal)
{
	int status;
	pid_t pid;
	pink_error_t error;
	pink_event_t event;

	/* Do NOT set EXIT option, we want the exit(2) to be genuine.
	 */
	if ((pid = pink_fork(CHECK_OPTIONS, &error)) < 0)
		fail("pink_fork: %s (%s)", pink_error_tostring(error),
			strerror(errno));
	else if (!pid) /* child */
		kill(getpid(), SIGKILL);
	else { /* parent */
		/* Resume the child, it will stop at the exit(2). */
		fail_unless(pink_trace_cont(pid, 0),
				"pink_trace_cont failed: %s",
				strerror(errno));
		fail_if(waitpid(pid, &status, 0) < 0,
				"waitpid failed: %s",
				strerror(errno));
		event = pink_event_decide(status, true);
		fail_unless(event == PINK_EVENT_EXIT_SIGNAL,
				"Wrong event, expected: %d got: %d",
				PINK_EVENT_EXIT_SIGNAL, event);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_event_unknown)
{
}
END_TEST

Suite *
event_suite_create(void)
{
	Suite *s = suite_create("event");

	/* pink_event() */
	TCase *tc_pink_event = tcase_create("pink_event");

	tcase_add_test(tc_pink_event, t_event_stop);
	tcase_add_test(tc_pink_event, t_event_syscall);
	tcase_add_test(tc_pink_event, t_event_fork);
	tcase_add_test(tc_pink_event, t_event_vfork);
	tcase_add_test(tc_pink_event, t_event_clone);
	tcase_add_test(tc_pink_event, t_event_vfork_done);
	tcase_add_test(tc_pink_event, t_event_exec);
	tcase_add_test(tc_pink_event, t_event_exit);
	tcase_add_test(tc_pink_event, t_event_genuine);
	tcase_add_test(tc_pink_event, t_event_exit_genuine);
	tcase_add_test(tc_pink_event, t_event_exit_signal);
	tcase_add_test(tc_pink_event, t_event_unknown);

	suite_add_tcase(s, tc_pink_event);

	return s;
}
