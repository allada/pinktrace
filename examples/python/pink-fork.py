#!/usr/bin/env python
# coding: utf-8
# vim: set sw=4 ts=4 sts=4 et :

"""\
An example demonstrating the tracing fork
"""

from __future__ import print_function

import os, signal

import pinktrace.event
import pinktrace.fork
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
    event = pinktrace.event.decide(status)
    assert event == pinktrace.event.EVENT_STOP, "event: %d" % event

    # Set tracing options
    pinktrace.trace.setup(pid)
    # Let the child continue...
    pinktrace.trace.cont(pid)
