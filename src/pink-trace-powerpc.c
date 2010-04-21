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

#include <pinktrace/internal.h>
#include <pinktrace/pink.h>

#define ORIG_ACCUM	(sizeof(unsigned long) * PT_R0)
#define ACCUM		(sizeof(unsigned long) * PT_R3)
#define ACCUM_FLAGS	(sizeof(unsigned long) * PT_CCR)
#define SO_MASK		0x10000000

pink_bitness_t
pink_bitness_get(pink_unused pid_t pid)
{
#if defined(POWERPC)
	return PINK_BITNESS_32;
#elif defined(POWERPC64)
	return PINK_BITNESS_64;
#else
#error unsupported architecture
}

bool
pink_util_get_syscall(pid_t pid, long *res)
{
	return pink_util_upeek(pid, ORIG_ACCUM, res);
}

bool
pink_util_set_syscall(pid_t pid, long scno)
{
	return (0 == ptrace(PTRACE_POKEUSER, pid, ORIG_ACCUM, scno));
}

bool
pink_util_get_return(pid_t pid, long *res)
{
	long flags;

	if (!pink_util_upeek(pid, ACCUM, res) ||
			pink_util_upeek(pid, ACCUM_FLAGS, &flags))
		return false;

	if (flags & SO_MASK)
		*res = -(*res);
	return true;
}

bool
pink_util_set_return(pid_t pid, long ret)
{
	long flags;

	if (!pink_util_upeek(pid, ACCUM_FLAGS, &flags))
		return false;

	if (val < 0) {
		flags |= SO_MASK;
		val = -val;
	}
	else
		flags &= ~SO_MASK;

	return (0 == ptrace(PTRACE_POKEUSER, pid, ACCUM, ret)) &&
		(0 == ptrace(PTRACE_POKEUSER, pid, ACCUM_FLAGS, flags));
}
