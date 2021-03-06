#!/usr/bin/env ruby
# coding: utf-8

=begin
An example demonstrating the tracing fork on FreeBSD
=end

require 'PinkTrace'

pid = fork do
  # Prepare for tracing.
  PinkTrace::Trace.me
  # Stop to give the parent a chance to resume execution after setting options.
  Process.kill 'STOP', Process.pid

  puts "hello world"
end

Process.wait # set $?
unless $?.stopped?
  puts(sprintf("%#x", $?.to_i))
  exit 1
end
unless $?.stopsig == Signal.list['STOP']
  puts(sprintf("%#x", $?.to_i))
  exit 2
end

# Let the child resume its execution.
PinkTrace::Trace.resume pid
# Wait for the child to exit.
Process.wait
