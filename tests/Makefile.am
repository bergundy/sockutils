check_PROGRAMS = sock.t

srcdir = $(top_builddir)/src
AM_CFLAGS = -I$(srcdir) 

sock_t_SOURCES = check_sock.c sockutils.h
sock_t_CFLAGS  = @CHECK_CFLAGS@ $(AM_CFLAGS)
sock_t_LDADD   = @CHECK_LIBS@ $(srcdir)/*.o
