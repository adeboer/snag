# configure.ac
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#	Copyright (C) 2010 Anthony de Boer
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of version 2 of the GNU General Public License as
#	published by the Free Software Foundation.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
#	USA

AC_PREREQ([2.65])
AC_INIT([snag], [0.8], [adb@adb.ca])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([snag.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_YACC

# Checks for libraries.
AC_CHECK_LIB([fl], [yywrap])
AC_CHECK_LIB([kvm], [kvm_getprocs])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h mntent.h stdlib.h string.h sys/statvfs.h sys/time.h unistd.h kvm.h sys/swap.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_GETMNTENT
AC_FUNC_MALLOC
AC_CHECK_FUNCS([alarm dup2 getmntent gettimeofday hasmntopt select strchr strdup strerror sysinfo getfsstat kvm_open])

AC_ARG_WITH([cmdpipe],
[AS_HELP_STRING([--with-cmdpipe],
[support read-write pipe @<:@default=check@:>@])],
[],
[with_cmdpipe=check])

AC_ARG_WITH([plugins],
[AS_HELP_STRING([--with-plugins],
[support read-write pipe @<:@default=check@:>@])],
[],
[with_plugins=check])

if test "x$with_plugins" = "xcheck"; then
	for p in /usr/local/libexec/nagios /usr/lib/nagios/plugins /usr/nagios/libexec; do
		if test -d $p; then
			with_plugins=p
			break
		fi
	done
fi

if test "x$with_plugins" = "xcheck"; then
	echo Cannot find your monitoring plugins location
	exit 1
fi

AC_SUBST([pluginsdir],[$with_plugins])

AC_DEFINE_UNQUOTED([CMDPIPE], ["$with_cmdpipe"], [Named pipe used for external command injection...])
AC_DEFINE_UNQUOTED([PLUGINS], ["$with_plugins"], [Where monitor plugins (check_*) live...])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
