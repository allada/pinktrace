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

#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <pinktrace/internal.h>
#include <pinktrace/pink.h>

pid_t
pink_fork(int options, pink_error_t *error_r)
{
	int save_errno, status;
	pid_t pid;

	/* Force this for now as pink_event_decide() doesn't work without it. */
	options |= PINK_TRACE_OPTION_SYSGOOD;

	if ((pid = fork()) < 0) {
		if (error_r)
			*error_r = PINK_ERROR_FORK;
		return -1;
	}
	else if (!pid) { /* child */
		if (!pink_trace_me())
			_exit(1);
		if (kill(getpid(), SIGSTOP) < 0)
			_exit(2);
	}
	else { /* parent */
		if (waitpid(pid, &status, 0) < 0) {
			/* Careful here, if the child is dead, this means that
			 * pink_trace_me() function failed.
			 */
			if (error_r)
				*error_r = (errno == ECHILD)
						? PINK_ERROR_TRACE
						: PINK_ERROR_WAIT;
			return -1;
		}

		if (WIFEXITED(status)) {
			if (error_r) {
				switch (WEXITSTATUS(status)) {
				case 1:
					*error_r = PINK_ERROR_TRACE;
				case 2:
					*error_r = PINK_ERROR_STOP;
				default:
					*error_r = PINK_ERROR_UNKNOWN;
				}
			}
			return -1;
		}

		/* If we're at this point, both functions that the child called
		 * should have succeeded. Hence we can assert these: */
		assert(WIFSTOPPED(status));
		assert(WSTOPSIG(status) == SIGSTOP);

		if (!pink_trace_setup(pid, options)) {
			/* Setting up child failed, kill it with fire! */
			save_errno = errno;
			kill(pid, SIGKILL);
			errno = save_errno;

			if (error_r)
				*error_r = PINK_ERROR_TRACE_SETUP;
			return -1;
		}
	}
	return pid;
}
