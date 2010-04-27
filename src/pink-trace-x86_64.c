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
#include <stdlib.h>

#include <pinktrace/internal.h>
#include <pinktrace/pink.h>

#define ORIG_ACCUM	(8 * ORIG_RAX)
#define ACCUM		(8 * RAX)
static const long syscall_args[2][MAX_ARGS] = {
	{8 * RBX, 8 * RCX, 8 * RDX, 8 * RSI, 8 * RDI, 8 * RBP},
	{8 * RDI, 8 * RSI, 8 * RDX, 8 * R10, 8 * R8, 8 * R9}
};

pink_bitness_t
pink_bitness_get(pid_t pid)
{
	long cs;

	/*
	 * Check CS register value,
	 * On x86-64 linux this is:
	 * 0x33    for long mode (64 bit)
	 * 0x23    for compatibility mode (32 bit)
	 */

	if (!pink_util_peek(pid, 8 * CS, &cs))
		return PINK_BITNESS_UNKNOWN;

	switch (cs) {
	case 0x33:
		return PINK_BITNESS_64;
	case 0x23:
		return PINK_BITNESS_32;
	default:
		return PINK_BITNESS_UNKNOWN;
	}
}

bool
pink_util_get_syscall(pid_t pid, long *res)
{
	return pink_util_peek(pid, ORIG_ACCUM, res);
}

bool
pink_util_set_syscall(pid_t pid, long scno)
{
	return pink_util_poke(pid, ORIG_ACCUM, scno);
}

bool
pink_util_get_return(pid_t pid, long *res)
{
	return pink_util_peek(pid, ACCUM, res);
}

bool
pink_util_set_return(pid_t pid, long ret)
{
	return pink_util_poke(pid, ACCUM, ret);
}

bool
pink_util_get_arg(pid_t pid, pink_bitness_t bitness, int arg, long *res)
{
	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);

	return pink_util_peek(pid, syscall_args[bitness][arg], res);
}

bool
pink_decode_simple(pid_t pid, pink_bitness_t bitness, int arg, void *dest, size_t len)
{
	long addr;

	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);

	if (!pink_util_get_arg(pid, bitness, arg, &addr))
		return false;

	return pink_util_moven(pid, addr, dest, len);
}

bool
pink_decode_string(pid_t pid, pink_bitness_t bitness, int arg, char *dest, size_t len)
{
	long addr;

	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);

	if (!pink_util_get_arg(pid, bitness, arg, &addr))
		return false;

	return pink_util_movestr(pid, addr, dest, len);
}

char *
pink_decode_string_persistent(pid_t pid, pink_bitness_t bitness, int arg)
{
	long addr;

	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);

	if (!pink_util_get_arg(pid, bitness, arg, &addr))
		return false;

	return pink_util_movestr_persistent(pid, addr);
}

bool
pink_encode_simple(pid_t pid, pink_bitness_t bitness, int arg, const void *src, size_t len)
{
	long addr;

	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);

	if (!pink_util_get_arg(pid, bitness, arg, &addr))
		return false;

	return pink_util_putn(pid, addr, src, len);
}

bool
pink_encode_simple_safe(pid_t pid, pink_bitness_t bitness, int arg, const void *src, size_t len)
{
	long addr;

	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);

	if (!pink_util_get_arg(pid, bitness, arg, &addr))
		return false;

	return pink_util_putn_safe(pid, addr, src, len);
}

bool
pink_decode_socket_call(pid_t pid, pink_bitness_t bitness, long *call, bool *decoded)
{
	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(call != NULL);

	switch (bitness) {
	case PINK_BITNESS_32:
		/* Decode socketcall(2) */
		if (!pink_util_get_arg(pid, PINK_BITNESS_32, 0, call))
			return false;
		if (decoded)
			*decoded = true;
		return true;
	case PINK_BITNESS_64:
		if (!pink_util_get_syscall(pid, call))
			return false;
		if (decoded)
			*decoded = false;
		return true;
	case PINK_BITNESS_UNKNOWN:
	default:
		abort();
	}
}

bool
pink_decode_socket_fd(pid_t pid, pink_bitness_t bitness, int arg, long *fd)
{
	long args;

	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);
	assert(fd != NULL);

	switch (bitness) {
	case PINK_BITNESS_32:
		/* Decode socketcall(2) */
		if (!pink_util_get_arg(pid, PINK_BITNESS_32, 1, &args))
			return false;
		args += arg * sizeof(unsigned int);
		return pink_util_move(pid, args, fd);
	case PINK_BITNESS_64:
		return pink_util_get_arg(pid, PINK_BITNESS_64, 0, fd);
	case PINK_BITNESS_UNKNOWN:
	default:
		abort();
	}
}

bool
pink_decode_socket_address(pid_t pid, pink_bitness_t bitness, int arg,
	long *fd_r, pink_socket_address_t *addr_r)
{
	unsigned int iaddr, iaddrlen;
	long addr, addrlen, args;

	assert(bitness == PINK_BITNESS_32 || bitness == PINK_BITNESS_64);
	assert(arg >= 0 && arg < MAX_ARGS);

	switch (bitness) {
	case PINK_BITNESS_32:
		/* Decode socketcall(2) */
		if (!pink_util_get_arg(pid, PINK_BITNESS_32, 1, &args))
			return false;
		if (fd_r && !pink_util_move(pid, args, fd_r))
			return false;
		args += arg * sizeof(unsigned int);
		if (!pink_util_move(pid, args, &iaddr))
			return false;
		args += sizeof(unsigned int);
		if (!pink_util_move(pid, args, &iaddrlen))
			return false;
		addr = iaddr;
		addrlen = iaddrlen;
		break;
	case PINK_BITNESS_64:
		if (fd_r && !pink_util_get_arg(pid, PINK_BITNESS_64, 0, fd_r))
			return false;
		if (!pink_util_get_arg(pid, PINK_BITNESS_64, arg, &addr))
			return false;
		if (!pink_util_get_arg(pid, PINK_BITNESS_64, arg + 1, &addrlen))
			return false;
		break;
	case PINK_BITNESS_UNKNOWN:
	default:
		abort();
	}

	return pink_internal_decode_socket_address(pid, addr, addrlen, addr_r);
}
