/*
 * Copyright (c) 2010, 2011, 2012 Ali Polatel <alip@exherbo.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LpIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PINK_EASY_LOOP_H
#define _PINK_EASY_LOOP_H

/**
 * @file pinktrace/easy/loop.h
 * @brief Pink's easy event loop
 * @defgroup pink_easy_loop Pink's easy event loop
 * @ingroup pinktrace-easy
 * @{
 **/

#include <pinktrace/pink.h>
#include <pinktrace/easy/context.h>

PINK_BEGIN_DECL

/**
 * The main event loop
 *
 * @param ctx Tracing context
 * @return In case of success, if the "cb_end" callback exists this function
 *         calls it and returns what that callback returns; otherwise this
 *         function returns zero. In case of any error condition, this callback
 *         calls the "cb_error" callback and returns the error condition
 *         negated.
 **/
int pink_easy_loop(pink_easy_context_t *ctx)
	PINK_GCC_ATTR((nonnull(1)));

PINK_END_DECL
/** @} */
#endif
