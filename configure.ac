#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([snag], [0.6], [adb@adb.ca])
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
# FIXME: Replace `main' with a function in `-lfl':
AC_CHECK_LIB([fl], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h mntent.h stdlib.h string.h sys/statvfs.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_GETMNTENT
AC_FUNC_MALLOC
AC_CHECK_FUNCS([alarm dup2 getmntent gettimeofday hasmntopt select strchr strdup strerror sysinfo])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
