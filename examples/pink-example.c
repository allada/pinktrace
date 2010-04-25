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

#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pinktrace/pink.h>

#define MAX_STRING_LEN 128

/* Utility functions */
static void
print_ret(pid_t pid)
{
	long ret;

	if (!pink_util_get_return(pid, &ret)) {
		fprintf(stderr, "pink_util_get_return: %s\n",
				strerror(errno));
		return;
	}

	if (ret >= 0)
		printf("= %ld", ret);
	else
		printf("= %ld (%s)", ret, strerror(-ret));
}

static void
print_open_flags(long flags)
{
	long aflags;
	bool found;

	found = true;

	/* Check out access flags */
	aflags = flags & 3;
	switch (aflags) {
	case O_RDONLY:
		printf("O_RDONLY");
		break;
	case O_WRONLY:
		printf("O_WRONLY");
		break;
	case O_RDWR:
		printf("O_RDWR");
		break;
	default:
		/* Nothing found */
		found = false;
	}

	if (flags & O_CREAT) {
		printf("%s | O_CREAT", found ? "" : "0");
		found = true;
	}

	if (!found)
		printf("%#x", (unsigned)flags);
}

/* A generic decoder for system calls. */
static void
decode_simple(pink_bitness_t bitness, long scno)
{
	const char *scname;

	/* Figure out the name of the system call. */
	scname = pink_name_syscall(scno, bitness);
	if (scname == NULL)
		printf("%ld", scno);
	else
		printf("%s", scname);
	printf("()");
}

/* A very basic decoder for open(2) system call. */
static void
decode_open(pid_t pid, pink_bitness_t bitness)
{
	long flags;
	char buf[MAX_STRING_LEN];

	if (!pink_decode_string(pid, bitness, 0, buf, MAX_STRING_LEN)) {
		fprintf(stderr, "pink_decode_string: %s\n",
				strerror(errno));
		return;
	}
	if (!pink_util_get_arg(pid, bitness, 1, &flags)) {
		fprintf(stderr, "pink_util_get_arg: %s\n",
				strerror(errno));
		return;
	}

	printf("open(\"%s\", ", buf);
	print_open_flags(flags);
	fputc(')', stdout);
}

int
main(int argc, char **argv)
{
	bool dead, insyscall;
	int sig, status, exit_code;
	long scno;
	pid_t pid;
	pink_bitness_t bitness;
	pink_event_t event;
	pink_context_t *ctx;

	/* Parse arguments */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s program [argument...]\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Create a tracing context. */
	ctx = pink_context_new();
	if (!ctx) {
		fprintf(stderr, "pink_context_new: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	/* Set tracing options */
	pink_context_set_options(ctx, PINK_TRACE_OPTION_EXEC);

	/* Fork */
	if ((pid = pink_fork(ctx)) < 0) {
		fprintf(stderr, "pink_fork: %s, %s\n",
				pink_error_tostring(pink_context_get_error(ctx)),
				strerror(errno));
		return EXIT_FAILURE;
	}
	else if (!pid) { /* child */
		++argv;
		execvp(argv[0], argv);
		fprintf(stderr, "execvp: %s\n", strerror(errno));
		_exit(-1);
	}
	else {
		/* Figure out the bitness of the process. */
		bitness = pink_bitness_get(pid);
		printf("Child %i runs in %s mode\n", pid, pink_bitness_tostring(bitness));

		dead = insyscall = false;
		sig = exit_code = 0;
		for (;;) {
			/* At this point child is stopped and needs to be resumed.
			 */
			if (!pink_trace_syscall(pid, sig)) {
				fprintf(stderr, "pink_trace_syscall: %s\n", strerror(errno));
				return (errno == ESRCH) ? 0 : 1;
			}
			sig = 0;

			/* Wait for the child */
			if ((pid = waitpid(pid, &status, 0)) < 0) {
				fprintf(stderr, "waitpid: %s\n", strerror(errno));
				return (errno == ECHILD) ? 0 : 1;
			}

			/* Check the event. */
			event = pink_event_decide(ctx, status);
			switch (event) {
			case PINK_EVENT_SYSCALL:
				/* We get this event twice, one at entering a
				 * system call and one at exiting a system
				 * call. */
				if (insyscall) {
					/* Exiting the system call, print the
					 * return value. */
					fputc(' ', stdout);
					print_ret(pid);
					fputc('\n', stdout);
					insyscall = false;
				}
				else {
					insyscall = true;
					/* Get the system call number and call
					 * the appropriate decoder. */
					if (!pink_util_get_syscall(pid, &scno)) {
						fprintf(stderr, "pink_util_get_syscall: %s\n",
								strerror(errno));
					}
					else if (scno == SYS_open)
						decode_open(pid, bitness);
					else
						decode_simple(bitness, scno);
				}
				break;
			case PINK_EVENT_EXEC:
				/* Update bitness */
				bitness = pink_bitness_get(pid);
				break;
			case PINK_EVENT_GENUINE:
			case PINK_EVENT_UNKNOWN:
				/* Send the signal to the child as it was a genuine
				 * signal.
				 */
				sig = WSTOPSIG(status);
				break;
			case PINK_EVENT_EXIT_GENUINE:
				exit_code = WEXITSTATUS(status);
				printf("Child %i exited normally with return code %d\n",
						pid, exit_code);
				dead = true;
				break;
			case PINK_EVENT_EXIT_SIGNAL:
				exit_code = 128 + WTERMSIG(status);
				printf("Child %i exited with signal %d\n",
						pid, WTERMSIG(status));
				dead = true;
				break;
			default:
				/* Nothing */
				;
			}
			if (dead)
				break;
		}

		/* Cleanup and exit */
		pink_context_free(ctx);
		return exit_code;
	}
}