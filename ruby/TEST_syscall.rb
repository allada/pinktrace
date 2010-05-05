#!/usr/bin/env ruby
# coding: utf-8
# vim: set sw=2 sts=2 et nowrap fenc=utf-8 :

require 'test/unit'

$:.insert(0, '.libs')
require 'PinkTrace'

class TestPinkSyscall < Test::Unit::TestCase
  def test_syscall_get_arg_invalid
    assert_raise ArgumentError do
      PinkTrace::Syscall.get_arg
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.get_arg 0, 1, 2, 3
    end
    assert_raise TypeError do
      PinkTrace::Syscall.get_arg 'pink', 0
    end
    assert_raise TypeError do
      PinkTrace::Syscall.get_arg 0, 'pink'
    end
    assert_raise TypeError do
      PinkTrace::Syscall.get_arg 0, 1, 'pink'
    end
    assert_raise PinkTrace::BitnessError do
      PinkTrace::Syscall.get_arg 0, 1, 13
    end
    assert_raise PinkTrace::IndexError do
      PinkTrace::Syscall.get_arg 0, PinkTrace::MAX_INDEX
    end
  end

  def test_syscall_get_arg_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::Syscall.get_arg 0, 1
    end
  end

  def test_syscall_get_no
    assert_raise ArgumentError do
      PinkTrace::Syscall.get_no
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.get_no 0, 1
    end
    assert_raise TypeError do
      PinkTrace::Syscall.get_no 'pink'
    end
  end

  def test_syscall_get_no_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::Syscall.get_no 0
    end
  end

  def test_syscall_get_ret
    assert_raise ArgumentError do
      PinkTrace::Syscall.get_ret
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.get_ret 0, 1
    end
    assert_raise TypeError do
      PinkTrace::Syscall.get_ret 'pink'
    end
  end

  def test_syscall_get_ret_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::Syscall.get_ret 0
    end
  end

  def test_syscall_name_invalid
    assert_raise ArgumentError do
      PinkTrace::Syscall.name
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.name 0, 1, 2
    end
    assert_raise TypeError do
      PinkTrace::Syscall.name 'pink'
    end
    assert_raise TypeError do
      PinkTrace::Syscall.name 0, 'pink'
    end
    assert_raise PinkTrace::BitnessError do
      PinkTrace::Syscall.name 0, 13
    end
  end

  def test_syscall_set_no_invalid
    assert_raise ArgumentError do
      PinkTrace::Syscall.set_no
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.set_no 0
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.set_no 0, 1, 2
    end
    assert_raise TypeError do
      PinkTrace::Syscall.set_no 'pink', 1
    end
    assert_raise TypeError do
      PinkTrace::Syscall.set_no 0, 'pink'
    end
  end

  def test_syscall_set_no_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::Syscall.set_no 0, 1
    end
  end

  def test_syscall_set_ret_invalid
    assert_raise ArgumentError do
      PinkTrace::Syscall.set_ret
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.set_ret 0
    end
    assert_raise ArgumentError do
      PinkTrace::Syscall.set_ret 0, 1, 2
    end
    assert_raise TypeError do
      PinkTrace::Syscall.set_ret 'pink', 1
    end
    assert_raise TypeError do
      PinkTrace::Syscall.set_ret 0, 'pink'
    end
  end

  def test_syscall_set_ret_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::Syscall.set_ret 0, 1
    end
  end
end

# These test cases depend on generated system call names.
# Don't run them if they weren't generated.
unless PinkTrace::Syscall.name 0
  exit 0
end

class TestPinkSyscall
  def test_syscall_get_no
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      Process.kill 0, Process.pid
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid

    # Loop until we get to the kill() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    found = false
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if name == 'kill' then
          found = true
          break
        end
      end
    end

    assert found, 'Failed to get system call number for kill'

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_syscall_set_no
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      Process.kill 0, Process.pid
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid

    # Loop until we get to the kill() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if name == 'kill' then
          PinkTrace::Syscall.set_no pid, 0xbadca11
          scno = PinkTrace::Syscall.get_no pid
          assert(scno == 0xbadca11, "Wrong system call no, expected: 0xbadca11 got: #{scno}")
          break
        end
      end
    end

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_syscall_get_ret_success
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      Process.kill 0, Process.pid
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid

    # Loop until we get to the kill() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    insyscall = false
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if insyscall and name == 'kill' then
          ret = PinkTrace::Syscall.get_ret pid
          assert(ret == 0, "Wrong system call return, expected: 0 got: #{ret}")
          break
        end
        insyscall = insyscall ? false : true
      end
    end

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_syscall_get_ret_fail
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      begin File.open ''
      rescue Errno::ENOENT ;end
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid

    # Loop until we get to the kill() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    insyscall = false
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if insyscall and name == 'open' then
          ret = PinkTrace::Syscall.get_ret pid
          assert(ret == -Errno::ENOENT::Errno, "Wrong system call return, expected: #{-Errno::ENOENT::Errno} got: #{ret}")
          break
        end
        insyscall = insyscall ? false : true
      end
    end

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_syscall_set_ret_success
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      ret = Process.kill 0, Process.pid
      exit 0 if ret == 1
      exit 1
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid

    # Loop until we get to the kill() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    insyscall = false
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if insyscall and name == 'kill' then
          PinkTrace::Syscall.set_ret pid, 1
        end
        insyscall = insyscall ? false : true
      end
    end

    assert $?.exited?, "Child hasn't exited!"
    assert($?.exitstatus == 0, "Wrong exit status, expected: 0 got: #{$?.exitstatus}")

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_syscall_set_ret_fail
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      begin
        Process.kill 0, Process.pid
      rescue Errno::EPERM
        exit 0
      end
      exit 1
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid

    # Loop until we get to the kill() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    insyscall = false
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if insyscall and name == 'kill' then
          PinkTrace::Syscall.set_ret pid, -Errno::EPERM::Errno
        end
        insyscall = insyscall ? false : true
      end
    end

    assert $?.exited?, "Child hasn't exited!"
    assert($?.exitstatus == 0, "Wrong exit status, expected: 0 got: #{$?.exitstatus}")

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_syscall_get_arg_first
  end

  def test_syscall_get_arg_second
  end

  def test_syscall_get_arg_third
  end

  def test_syscall_get_arg_fourth
  end

  def test_syscall_get_arg_fifth
  end

  def test_syscall_get_arg_sixth
  end
end