#!/usr/bin/env python
# coding: utf-8

"""
An example demonstrating the tracing fork on FreeBSD.
"""

from __future__ import print_function

import os, signal

import pinktrace.event
import pinktrace.trace

pid = os.fork()
if not pid:
    # Prepare for tracing.
    pinktrace.trace.me()
    # Stop to give the parent a chance to resume execution after setting options.
    os.kill(os.getpid(), signal.SIGSTOP)

    print("hello world")
else: # parent
    pid, status = os.waitpid(pid, 0)
    assert os.WIFSTOPPED(status), "%#x" % status
    assert os.WSTOPSIG(status) == signal.SIGSTOP, "%#x" % status

    # Let the child resume its execution.
    pinktrace.trace.resume(pid)
    # Wait for the child to exit.
    os.waitpid(pid, 0)
