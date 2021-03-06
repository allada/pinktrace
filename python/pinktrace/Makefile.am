CLEANFILES= *.so *.pyc *.pyo
SUBDIRS= .
IF_PYTHON_FILES= \
		 __init__.py.in \
		 pink-python-bitness.c \
		 pink-python-event.c \
		 pink-python-socket.c \
		 pink-python-string.c \
		 pink-python-strarray.c \
		 pink-python-syscall.c \
		 pink-python-trace.c
EXTRA_DIST= $(IF_PYTHON_FILES)
AM_CFLAGS= \
	   -I. -I@PYTHON_INCLUDE_DIR@ \
	   -I$(top_builddir)/include \
	   -I$(top_srcdir)/include \
	   @PINKTRACE_CFLAGS@
BUILT_SOURCES=

noinst_HEADERS= pink-python-hacks.h

if ENABLE_PYTHON
pythonlibdir= @PYTHON_INSTALL_DIR@/pinktrace
pythonlib_LTLIBRARIES=

pythonlib_DATA= __init__.py
CLEANFILES+= __init__.py

__init__.py: __init__.py.in
	$(AM_V_GEN)
	$(AM_V_at)$(SED) \
		-e "s,#PACKAGE#,$(PACKAGE)," \
		-e "s,#VERSION#,$(VERSION)," \
		-e "s,#VERSION_MAJOR#,$(VERSION_MAJOR)," \
		-e "s,#VERSION_MINOR#,$(VERSION_MINOR)," \
		-e "s,#VERSION_MICRO#,$(VERSION_MICRO)," \
		-e "s,#VERSION_SUFFIX#,$(VERSION_SUFFIX)," \
		-e "s,#GIT_HEAD#,$(GITHEAD)," \
		-e "s,#PC_SLOT#,$(PINKTRACE_PC_SLOT)," \
		-e "s,#OS_FREEBSD#,$(OS_FREEBSD)," \
		-e "s,#OS_LINUX#,$(OS_LINUX)," \
		-e "s,#HAVE_IPV6#,$(PYTHON_HAVE_IPV6)," \
		-e "s,#HAVE_NETLINK#,$(PYTHON_HAVE_NETLINK)," \
		$< > $@

pythonlib_LTLIBRARIES+= bitness.la
bitness_la_SOURCES= pink-python-bitness.c
bitness_la_LDFLAGS= -module -avoid-version -shared
bitness_la_LIBADD= $(top_builddir)/src/libpinktrace_@PINKTRACE_PC_SLOT@.la

pythonlib_LTLIBRARIES+= event.la
event_la_SOURCES= pink-python-event.c
event_la_LDFLAGS= -module -avoid-version -shared
event_la_LIBADD= $(top_builddir)/src/libpinktrace_@PINKTRACE_PC_SLOT@.la

pythonlib_LTLIBRARIES+= socket.la
socket_la_SOURCES= pink-python-socket.c
socket_la_LDFLAGS= -module -avoid-version -shared
socket_la_LIBADD= $(top_builddir)/src/libpinktrace_@PINKTRACE_PC_SLOT@.la

pythonlib_LTLIBRARIES+= string.la
string_la_SOURCES= pink-python-string.c
string_la_LDFLAGS= -module -avoid-version -shared
string_la_LIBADD= $(top_builddir)/src/libpinktrace_@PINKTRACE_PC_SLOT@.la

pythonlib_LTLIBRARIES+= strarray.la
strarray_la_SOURCES= pink-python-strarray.c
strarray_la_LDFLAGS= -module -avoid-version -shared
strarray_la_LIBADD= $(top_builddir)/src/libpinktrace_@PINKTRACE_PC_SLOT@.la

pythonlib_LTLIBRARIES+= syscall.la
syscall_la_SOURCES= pink-python-syscall.c
syscall_la_LDFLAGS= -module -avoid-version -shared
syscall_la_LIBADD= $(top_builddir)/src/libpinktrace_@PINKTRACE_PC_SLOT@.la

pythonlib_LTLIBRARIES+= trace.la
trace_la_SOURCES= pink-python-trace.c
trace_la_LDFLAGS= -module -avoid-version -shared
trace_la_LIBADD= $(top_builddir)/src/libpinktrace_@PINKTRACE_PC_SLOT@.la

# Hack to make epydoc & tests work.
%.so: %.la
	$(AM_V_GEN)
	$(AM_V_at)test -h $@ || $(LN_S) $(addprefix .libs/, $(addsuffix .so,$(basename $<))) $@

noinst_DATA= \
	     bitness.so \
	     event.so \
	     socket.so \
	     string.so \
	     strarray.so \
	     syscall.so \
	     trace.so

# Remove the .la files - *.la is never linked against (pinktrace is a module)
# and Python doesn't use libltdl. Note that the library gets installed by
# install-data, so that's where we need to hook.
# XXX: This breaks distcheck!
#install-data-hook:
#	rm -f $(DESTDIR)$(pythonlibdir)/*.la

endif # ENABLE_PYTHON
