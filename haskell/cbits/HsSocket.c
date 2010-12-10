/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/*
 * Copyright (c) 2010 Ali Polatel <alip@exherbo.org>
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "HsSocket.h"

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif /* !INET_ADDRSTRLEN */

#if PINKTRACE_HAVE_IPV6
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif /* !INET6_ADDRSTRLEN */
#endif

#ifdef PINKTRACE_LINUX
#define IS_ABSTRACT(addr) ((addr)->u.sa_un.sun_path[0] == '\0' && (addr)->u.sa_un.sun_path[1] != '\0')
#else
#define IS_ABSTRACT(addr) 0
#endif /* PINKTRACE_LINUX */

int __pinkhs_socket_family(pink_socket_address_t *addr) { return addr->family; }
socklen_t __pinkhs_socket_length(pink_socket_address_t *addr) { return addr->length; }
int __pinkhs_socket_isabstract(pink_socket_address_t *addr) { return (addr->family == AF_UNIX && IS_ABSTRACT(addr)); }
const char *__pinkhs_socket_path(pink_socket_address_t *addr) { return addr->u.sa_un.sun_path + (IS_ABSTRACT(addr) ? 1 : 0); }
const char *__pinkhs_socket_inet_ntop(pink_socket_address_t *addr, char *dest) { inet_ntop(AF_INET, &addr->u.sa_in.sin_addr, dest, INET_ADDRSTRLEN); return dest; }
#if PINKTRACE_HAVE_IPV6
const char *__pinkhs_socket_inet_ntop6(pink_socket_address_t *addr, char *dest) { inet_ntop(AF_INET6, &addr->u.sa6.sin6_addr, dest, INET6_ADDRSTRLEN); return dest; }
#endif /* PINKTRACE_HAVE_IPV6 */
#if PINKTRACE_HAVE_NETLINK
pid_t __pinkhs_socket_pid(pink_socket_address_t *addr) { return addr->u.nl.nl_pid; }
long __pinkhs_socket_groups(pink_socket_address_t *addr) { return addr->u.nl.nl_groups; }
#endif /* PINKTRACE_HAVE_NETLINK */