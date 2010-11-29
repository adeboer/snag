
/* snagprocs.c
 *
 *	Copyright (C) 2010 Anthony de Boer
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

#include "snag.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define SYSUIDMAX 999
#define MAXTRANSIENT 1000

/*
 * Linux: getting the process name is too much fun.  The string in
 * /proc/#/stat is truncated at 15 characters, and too many Linux
 * GUI designers have never heard of Unix and create executables
 * with terribly long names.  The string in /proc/#/cmdline will
 * be the whole thing but can be modified at runtime by argv-frobbing.
 *
 * /proc/#/stat has no clear indicator of a kernel thread, but cmdline
 * will read as zero bytes for kernel threads.  For added joy, though,
 * cmdline always stats as a zero-byte file even if reading will find
 * data.
 *
 * We ignore processes from users past SYSUIDMAX.
 *
 * Setuid/setgid result in things like stat and cmdline being root-owned
 * while the directory itself belongs to the user, so we have to stat
 * that to find out whose process this is.
 */

int snagprocs()
{
	time_t now = time(NULL);
	long tix = sysconf(_SC_CLK_TCK);
	printf("Clock ticks %ld\n", tix);
		FILE *fp = fopen("/proc/self/stat", "r");
		if (fp == NULL) {
			exit(1);
		}
	unsigned long long mystartj;
	int nelts = fscanf(fp,
		"%*d (%*s %*s %*d %*d %*d %*d %*d %*u %*u "
		"%*u %*u %*u %*u %*u %*d %*d %*d %*d %*d "
		"%*d %llu ",
		&mystartj);
	fclose(fp);
	DIR *dp = opendir("/proc");
	if (dp == NULL) {
		perror("/proc");
		return 1;
	}
	struct dirent *de;
	char bufr[1024];
	while (de = readdir(dp)) {
		if (de->d_type != DT_DIR) continue;
		int numlen = strspn(de->d_name, "0123456789");
		if (de->d_name[numlen] != '\0') continue;
		int sz = snprintf(bufr, sizeof(bufr), "/proc/%s/cmdline", de->d_name);
		if (sz < 1 || sz >= sizeof(bufr)) {
			fprintf(stderr, "Name %s too long\n", de->d_name);
			continue;
		}
		int fd = open(bufr, O_RDONLY);
		if (fd == -1) continue;
		/* any errors reading file likely races from procs exiting */
		int rb = read(fd, bufr, sizeof(bufr) - 1);
		/* zero bytes would be a kernel thread */
		if (rb <= 0) continue;
		/* stat the directory */
		/* shorter name than cmdline, will work if that one did */
		snprintf(bufr, sizeof(bufr), "/proc/%s", de->d_name);
		struct stat sbuf;
		if (stat(bufr, &sbuf) == -1) continue;
		if (sbuf.st_uid > SYSUIDMAX) continue;
		snprintf(bufr, sizeof(bufr), "/proc/%s/stat", de->d_name);
		FILE *fp = fopen(bufr, "r");
		if (fp == NULL) continue;
		char pstate;
		unsigned long long startj;
		int nelts = fscanf(fp,
			"%*d (%1023s %1c %*d %*d %*d %*d %*d %*u %*u "
			"%*u %*u %*u %*u %*u %*d %*d %*d %*d %*d "
			"%*d %llu ",
			bufr, &pstate, &startj);
		fclose(fp);
		if (nelts != 3 || pstate == 'Z') continue;

		char *closeparen = index(bufr, ')');
		if (closeparen) *closeparen = '\0';

		/* convert process start clock to seconds runtime */
		unsigned long long sex = (mystartj - startj) / tix;
		int oldproc = (sex > MAXTRANSIENT);

		procfound(bufr, oldproc);
	}
	closedir(dp);
	procfinal();
	return 0;
}

