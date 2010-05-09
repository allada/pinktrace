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

#include <errno.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <check.h>

#include <pinktrace/pink.h>

START_TEST(t_util_get_syscall)
{
	int status;
	long scno;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		/* From getpid(2):
		 * Since glibc version 2.3.4, the glibc wrapper function for getpid()
		 * caches PIDs, ...
		 */
		/* Since the child has just called getpid() to send herself a SIGSTOP,
		 * calling it again won't call the system call hence we need to use
		 * syscall(2) here.
		 */
		syscall(SYS_getpid);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_get_syscall(pid, PINKTRACE_DEFAULT_BITNESS, &scno),
			"%d(%s)", errno, strerror(errno));
		fail_unless(scno == SYS_getpid, "%ld != %ld", SYS_getpid, scno);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_set_syscall)
{
	int status;
	long scno;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		/* From getpid(2):
		 * Since glibc version 2.3.4, the glibc wrapper function for getpid()
		 * caches PIDs, ...
		 */
		/* Since the child has just called getpid() to send herself a SIGSTOP,
		 * calling it again won't call the system call hence we need to use
		 * syscall(2) here.
		 */
		syscall(SYS_getpid);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_set_syscall(pid, PINKTRACE_DEFAULT_BITNESS, 0xbadca11),
			"%d(%s)", errno, strerror(errno));
		fail_unless(pink_trace_util_get_syscall(pid, PINKTRACE_DEFAULT_BITNESS, &scno),
			"%d(%s)", errno, strerror(errno));
		fail_unless(scno == 0xbadca11, "%ld != %ld", 0xbadca11, scno);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_get_return_success)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		/* From getpid(2):
		 * Since glibc version 2.3.4, the glibc wrapper function for getpid()
		 * caches PIDs, ...
		 */
		/* Since the child has just called getpid() to send herself a SIGSTOP,
		 * calling it again won't call the system call hence we need to use
		 * syscall(2) here.
		 */
		syscall(SYS_getpid);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the end of next system call */
		for (unsigned int i = 0; i < 2; i++) {
			fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

			/* Make sure we got the right event */
			waitpid(pid, &status, 0);
			event = pink_trace_event_decide(status);
			fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);
		}

		fail_unless(pink_trace_util_get_return(pid, &ret), "%d(%s)", errno, strerror(errno));
		fail_unless(ret == pid, "%i != %d", pid, ret);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_get_return_fail)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		open(NULL, 0); /* Should fail with -EFAULT */
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the end of next system call */
		for (unsigned int i = 0; i < 2; i++) {
			fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

			/* Make sure we got the right event */
			waitpid(pid, &status, 0);
			event = pink_trace_event_decide(status);
			fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);
		}

		fail_unless(pink_trace_util_get_return(pid, &ret), "%d(%s)", errno, strerror(errno));
		fail_unless(ret == -EFAULT, "%ld != %ld", -EFAULT, ret);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_set_return_success)
{
	int ret, status;
	pid_t pid, mypid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		mypid = getpid();
		ret = syscall(SYS_getpid);
		if (ret != (mypid + 1)) {
			fprintf(stderr, "Wrong return, expected: %i got: %i\n",
					mypid + 1, ret);
			_exit(EXIT_FAILURE);
		}
		_exit(EXIT_SUCCESS);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the end of next system call */
		for (unsigned int i = 0; i < 2; i++) {
			fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

			/* Make sure we got the right event */
			waitpid(pid, &status, 0);
			event = pink_trace_event_decide(status);
			fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);
		}

		fail_unless(pink_trace_util_set_return(pid, pid + 1), "%d(%s)", errno, strerror(errno));

		/* Let the child exit and check her exit status */
		fail_unless(pink_trace_cont(pid, 0, NULL), "%d(%s)", errno, strerror(errno));
		waitpid(pid, &status, 0);
		fail_unless(WEXITSTATUS(status) == EXIT_SUCCESS, "%#x", status);
	}
}
END_TEST

START_TEST(t_util_set_return_fail)
{
	int ret, status;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		ret = syscall(SYS_getpid);
		if (ret > 0) {
			fprintf(stderr, "ret: %i\n", ret);
			_exit(EXIT_FAILURE);
		}
		else if (errno != ENAMETOOLONG) {
			fprintf(stderr, "errno: %d(%s)\n",
					errno, strerror(errno));
			_exit(EXIT_FAILURE);
		}
		_exit(EXIT_SUCCESS);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the end of next system call */
		for (unsigned int i = 0; i < 2; i++) {
			fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

			/* Make sure we got the right event */
			waitpid(pid, &status, 0);
			event = pink_trace_event_decide(status);
			fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);
		}

		fail_unless(pink_trace_util_set_return(pid, -ENAMETOOLONG), "%d(%s)", errno, strerror(errno));

		/* Let the child exit and check her exit status */
		fail_unless(pink_trace_cont(pid, 0, NULL), "%d(%s)", errno, strerror(errno));
		waitpid(pid, &status, 0);
		fail_unless(WEXITSTATUS(status) == EXIT_SUCCESS, "%#x", errno, strerror(errno));
	}
}
END_TEST

START_TEST(t_util_get_arg_first)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		mmap((void *)13, 0, 0, 0, 0, 0);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_get_arg(pid, PINKTRACE_DEFAULT_BITNESS, 0, &ret),
			"%d(%s)", errno, strerror(errno));
		fail_unless(ret == 13, "13 != %ld", ret);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_get_arg_second)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		mmap(NULL, 13, 0, 0, 0, 0);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_get_arg(pid, PINKTRACE_DEFAULT_BITNESS, 1, &ret),
			"%d(%s)", errno, strerror(errno));
		fail_unless(ret == 13, "13 != %ld", ret);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_get_arg_third)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		mmap(NULL, 0, 13, 0, 0, 0);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_get_arg(pid, PINKTRACE_DEFAULT_BITNESS, 2, &ret),
			"%d(%s)", errno, strerror(errno));
		fail_unless(ret == 13, "13 != %ld", ret);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_get_arg_fourth)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		mmap(NULL, 0, 0, 13, 0, 0);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_get_arg(pid, PINKTRACE_DEFAULT_BITNESS, 3, &ret),
			"%d(%s)", errno, strerror(errno));
		fail_unless(ret == 13, "13 != %ld", ret);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_get_arg_fifth)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		mmap(NULL, 0, 0, 0, 13, 0);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_get_arg(pid, PINKTRACE_DEFAULT_BITNESS, 4, &ret),
			"%d(%s)", errno, strerror(errno));
		fail_unless(ret == 13, "13 != %ld", ret);

		pink_trace_kill(pid);
	}
}
END_TEST

START_TEST(t_util_get_arg_sixth)
{
	int status;
	long ret;
	pid_t pid;
	pink_trace_event_t event;

	if ((pid = fork()) < 0)
		fail("fork: %d(%s)", errno, strerror(errno));
	else if (!pid) { /* child */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(-1);
		}
		kill(getpid(), SIGSTOP);
		mmap(NULL, 0, 0, 0, 0, 13);
	}
	else { /* parent */
		waitpid(pid, &status, 0);

		fail_unless(WIFSTOPPED(status), "%#x", status);
		fail_unless(WSTOPSIG(status) == SIGSTOP, "%#x", status);
		fail_unless(pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD), "%d(%s)", errno, strerror(errno));

		/* Resume the child and it will stop at the next system call */
		fail_unless(pink_trace_syscall(pid, 0), "%d(%s)", errno, strerror(errno));

		/* Make sure we got the right event */
		waitpid(pid, &status, 0);
		event = pink_trace_event_decide(status);
		fail_unless(event == PINK_TRACE_EVENT_SYSCALL, "%d != %d", PINK_TRACE_EVENT_SYSCALL, event);

		fail_unless(pink_trace_util_get_arg(pid, PINKTRACE_DEFAULT_BITNESS, 5, &ret),
			"%d(%s)", errno, strerror(errno));
		fail_unless(ret == 13, "13 != %ld", ret);

		pink_trace_kill(pid);
	}
}
END_TEST

Suite *
util_suite_create(void)
{
	Suite *s = suite_create("util");

	/* pink_trace_util_*() */
	TCase *tc_pink_trace_util = tcase_create("pink_trace_util");

	tcase_add_test(tc_pink_trace_util, t_util_get_syscall);
	tcase_add_test(tc_pink_trace_util, t_util_set_syscall);
	tcase_add_test(tc_pink_trace_util, t_util_get_return_success);
	tcase_add_test(tc_pink_trace_util, t_util_get_return_fail);
	tcase_add_test(tc_pink_trace_util, t_util_set_return_success);
	tcase_add_test(tc_pink_trace_util, t_util_set_return_fail);
	tcase_add_test(tc_pink_trace_util, t_util_get_arg_first);
	tcase_add_test(tc_pink_trace_util, t_util_get_arg_second);
	tcase_add_test(tc_pink_trace_util, t_util_get_arg_third);
	tcase_add_test(tc_pink_trace_util, t_util_get_arg_fourth);
	tcase_add_test(tc_pink_trace_util, t_util_get_arg_fifth);
	tcase_add_test(tc_pink_trace_util, t_util_get_arg_sixth);

	suite_add_tcase(s, tc_pink_trace_util);

	return s;
}
