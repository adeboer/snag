
AM_LFLAGS = -s
AM_YFLAGS = -d

bin_PROGRAMS = snag check_snag

bin_SCRIPTS = snagmsgs

check_snag_SOURCES = check_snag.c

snag_SOURCES = snag.c snag.h snagcmd.c snagdf.c snaginfo.c thresh.c \
	snag_parse.y snag_lex.l

EXTRA_DIST = snagmsgs

man_MANS = check_snag.8 snag.8
