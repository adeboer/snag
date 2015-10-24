# Makefile.am
#
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

AM_LFLAGS = -s
AM_YFLAGS = -d
AM_CPPFLAGS = -DSYSCONFDIR='"$(sysconfdir)"'

bin_PROGRAMS = snag check_snag

bin_SCRIPTS = snagmsgs

check_snag_SOURCES = check_snag.c

snag_SOURCES = snag.c snag.h snagcmd.c snagdf.c snaginfo.c thresh.c \
	snag_parse.y snag_lex.l snagprocs.c hashtab.c hashtab.h

man_MANS = check_snag.8 snag.8 snag.conf.5

EXTRA_DIST = snagmsgs $(man_MANS)

snag.8 : snag.8.in Makefile
	sed -e 's|@sysconfdir[@]|$(sysconfdir)|g' snag.8.in > $@

snag.conf.5 : snag.conf.5.in Makefile
	sed -e 's|@sysconfdir[@]|$(sysconfdir)|g' snag.conf.5.in > $@
