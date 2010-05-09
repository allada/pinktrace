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

#include <pinktrace/internal.h>
#include <pinktrace/pink.h>

#ifndef PT_ORIG_R3
#define PT_ORIG_R3 34
#endif /* !PT_ORIG_R3 */

#define ORIG_ACCUM	(sizeof(unsigned long) * PT_R0)
#define ACCUM		(sizeof(unsigned long) * PT_R3)
#define ACCUM_FLAGS	(sizeof(unsigned long) * PT_CCR)
#define SO_MASK		0x10000000

#define ARG_OFFSET(i)	(((i) == 0)				\
		? (sizeof(unsigned long) * PT_ORIG_R3)		\
		: (sizeof(unsigned long) * ((i) + PT_R3)))

pink_bitness_t
pink_trace_util_get_bitness(pink_unused pid_t pid)
{
#if defined(POWERPC)
	return PINK_BITNESS_32;
#elif defined(POWERPC64)
	return PINK_BITNESS_64;
#else
#error unsupported architecture
#endif
}

bool
pink_trace_util_get_syscall(pid_t pid, pink_unused pink_bitness_t bitness, long *res)
{
	return pink_trace_util_peek(pid, ORIG_ACCUM, res);
}

bool
pink_trace_util_set_syscall(pid_t pid, pink_unused pink_bitness_t bitness, long scno)
{
	return pink_trace_util_poke(pid, ORIG_ACCUM, scno);
}

bool
pink_trace_util_get_return(pid_t pid, long *res)
{
	long flags;

	if (!pink_trace_util_peek(pid, ACCUM, res) ||
			!pink_trace_util_peek(pid, ACCUM_FLAGS, &flags))
		return false;

	if (flags & SO_MASK)
		*res = -(*res);

	return true;
}

bool
pink_trace_util_set_return(pid_t pid, long ret)
{
	long flags;

	if (!pink_trace_util_peek(pid, ACCUM_FLAGS, &flags))
		return false;

	if (ret < 0) {
		flags |= SO_MASK;
		ret = -ret;
	}
	else
		flags &= ~SO_MASK;

	return pink_trace_util_poke(pid, ACCUM, ret) &&
		pink_trace_util_poke(pid, ACCUM_FLAGS, flags);
}

bool
pink_trace_util_get_arg(pid_t pid, pink_unused pink_bitness_t bitness, unsigned ind, long *res)
{
	assert(ind < PINK_TRACE_MAX_INDEX);

	return pink_trace_util_peek(pid, ARG_OFFSET(ind), res);
}

bool
pink_trace_decode_simple(pid_t pid, pink_bitness_t bitness, unsigned ind, void *dest, size_t len)
{
	long addr;

	assert(ind < PINK_TRACE_MAX_INDEX);

	if (!pink_trace_util_get_arg(pid, bitness, ind, &addr))
		return false;

	return pink_trace_util_moven(pid, addr, dest, len);
}

bool
pink_trace_decode_string(pid_t pid, pink_bitness_t bitness, unsigned ind, char *dest, size_t len)
{
	long addr;

	assert(ind < PINK_TRACE_MAX_INDEX);

	if (!pink_trace_util_get_arg(pid, bitness, ind, &addr))
		return false;

	return pink_trace_util_movestr(pid, addr, dest, len);
}

char *
pink_trace_decode_string_persistent(pid_t pid, pink_bitness_t bitness, unsigned ind)
{
	long addr;

	assert(ind < PINK_TRACE_MAX_INDEX);

	if (!pink_trace_util_get_arg(pid, bitness, ind, &addr))
		return false;

	return pink_trace_util_movestr_persistent(pid, addr);
}

bool
pink_trace_encode_simple(pid_t pid, pink_bitness_t bitness, unsigned ind, const void *src, size_t len)
{
	long addr;

	assert(ind < PINK_TRACE_MAX_INDEX);

	if (!pink_trace_util_get_arg(pid, bitness, ind, &addr))
		return false;

	return pink_trace_util_putn(pid, addr, src, len);
}

bool
pink_trace_encode_simple_safe(pid_t pid, pink_bitness_t bitness, unsigned ind, const void *src, size_t len)
{
	long addr;

	assert(ind < PINK_TRACE_MAX_INDEX);

	if (!pink_trace_util_get_arg(pid, bitness, ind, &addr))
		return false;

	return pink_trace_util_putn_safe(pid, addr, src, len);
}

bool
pink_trace_decode_socket_call(pid_t pid, pink_bitness_t bitness, long *subcall_r)
{
	assert(subcall_r != NULL);

	/* Decode socketcall(2) */
	if (!pink_trace_util_get_arg(pid, bitness, 0, subcall_r))
		return false;

	return true;
}

bool
pink_trace_decode_socket_fd(pid_t pid, pink_bitness_t bitness, unsigned ind, long *fd)
{
	long args;

	assert(ind < PINK_TRACE_MAX_INDEX);

	/* Decode socketcall(2) */
	if (!pink_trace_util_get_arg(pid, bitness, 1, &args))
		return false;
	args += ind * sizeof(unsigned int);

	return pink_trace_util_move(pid, args, fd);

}

bool
pink_trace_decode_socket_address(pid_t pid, pink_bitness_t bitness, unsigned ind,
	long *fd_r, pink_socket_address_t *addr_r)
{
	unsigned int iaddr, iaddrlen;
	long addr, addrlen, args;

	assert(ind < PINK_TRACE_MAX_INDEX);

	/* Decode socketcall(2) */
	if (!pink_trace_util_get_arg(pid, bitness, 1, &args))
		return false;
	if (fd_r && !pink_trace_util_move(pid, args, fd_r))
		return false;
	args += ind * sizeof(unsigned int);
	if (!pink_trace_util_move(pid, args, &iaddr))
		return false;
	args += sizeof(unsigned int);
	if (!pink_trace_util_move(pid, args, &iaddrlen))
		return false;
	addr = iaddr;
	addrlen = iaddrlen;

	return pink_trace_internal_decode_socket_address(pid, addr, addrlen, addr_r);
}
