
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

int snagprocs()
{
	time_t now = time(NULL);
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
		/* any errors reading file likely races from procs exiting */
		struct stat sbuf;
		if (fstat(fd, &sbuf) == -1) continue;
		if (sbuf.st_uid > SYSUIDMAX) continue;
		unsigned int age = now - sbuf.st_mtime;
		int oldproc = (age > MAXTRANSIENT);
		if (fd == -1) continue;
		int rb = read(fd, bufr, sizeof(bufr) - 1);
		/* zero bytes would be a kernel thread */
		if (rb <= 0) continue;
		bufr[rb] = '\0';
		int ec = strcspn(bufr, " \t:");
		bufr[ec] = '\0';
		char *ls = rindex(bufr, '/');
		if (ls == NULL) {
			ls = bufr;
		} else {
			ls++;
		}
		/*printf("%s : %s by %d %s\n", de->d_name, ls, sbuf.st_uid, oldproc ? "OLD" : "NEW");*/
		procfound(ls, oldproc);
	}
	closedir(dp);
	procfinal();
	return 0;
}

