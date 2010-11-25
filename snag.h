/* snag.h
 *
 *	Copyright (C) 2006,2008,2010 Anthony de Boer
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of version 2 of the GNU General Public License as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *	USA
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

/* config file */
#define cfile "/etc/snag.conf"


/*
 * -----------------------------
 * DEFINE ENTRYPOINTS TO MODULES
 * -----------------------------
 */

/* entry to df (disk-full) code in snagdf.c */
int snagdf();

/* entry to info (system info) code in snaginfo.c */
int snaginfo();

/* return the name of the numerical status given */
char *statusword(int sw);

/* thresher returns a Nagios result, 0=OK, 1=WARNING, 2=CRITICAL, 3=UNDEFINED
 * based on the thing "s" having the specified value.
 */
int thresher(char *s, long val);

/* gthresher returns a bitmask used internally by snag, where the 1 bit
 * means a WARNING condition is true, the 2 bit means a CRITICAL condition
 * is true, and the 4 bit means an UNDEFINED condition is true.  All bits
 * clear means we don't know of anything wrong at the moment.  Arguments
 * are the same as thresher above.  Internal-format bitmasks can be OR-ed
 * together and ultimately normalized by gnormalize.  The highest bit set
 * is trump.
 */
int gthresher(char *s, long val);

/* normalize internal format to Nagios external format */
int gnormalize(int c);

/* initialize an internal hash table */
void hinit(int showdefaults);

/* add a rule to the internal hash table */
void hashadd(char *s, long lcrit, long lwarn, long hwarn, long hcrit);

/* open the config file, return 1 if ok, 0 if no config file */
int openconfig();

/* close the config file, especially pre-exec */
void closeconfig();

/* set up for external commands */
void setupcmd();

/* set up an external command */
void startcmd(char *name, char *cmdline);

/* clean up external commands */
void cleancmd();

/* used by lex and yacc */
extern int lineno;
