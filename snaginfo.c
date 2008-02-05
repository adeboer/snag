/* snaginfo.c - call sysinfo and process data from it
 *
 *	Copyright (C) 2006,2008 Anthony de Boer
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

#include <stdio.h>
#include <sys/sysinfo.h>
#include "snag.h"

/*
 * Macros for load average, borrowed from Linux kernel.
 * Kernel internal is 11 but gets standardized to 16 by sys_sysinfo
 */

#define FSHIFT 16
#define FIXED_1 (1<<FSHIFT)
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

int snaginfo() {
	struct sysinfo info;
	if (sysinfo(&info)) {
		perror("sysinfo");
		return(1);
		}
	else {
		/* processing for load average */
		int a = info.loads[0] + (FIXED_1/200);
		int b = info.loads[1] + (FIXED_1/200);
		int c = info.loads[2] + (FIXED_1/200);
		int rc = 0;
		int ai = LOAD_INT(a);
		int af = LOAD_FRAC(a);
		int bi = LOAD_INT(b);
		int bf = LOAD_FRAC(b);
		int ci = LOAD_INT(c);
		int cf = LOAD_FRAC(c);

		rc |= gthresher("load1", ai);
		rc |= gthresher("load5", bi);
		rc |= gthresher("load15", ci);
		rc = gnormalize(rc);

		printf("Load average;%d;LOAD %s - load average: %d.%02d %d.%02d %d.%02d|load1=%d.%02d load5=%d.%02d load15=%d.%02d\n", rc, statusword(rc), ai, af, bi, bf, ci, cf, ai, af, bi, bf, ci, cf);

		rc = thresher("Uptime", info.uptime);
		if (rc < 3) {
			char *upu, *upm;
			unsigned long upv = info.uptime / 86400;
			if (upv) {
				upu = "day";
				}
			else {
				upv = info.uptime / 3600;
				if (upv) {
					upu = "hour";
					}
				else {
					upv = info.uptime / 60;
					if (upv) {
						upu = "minute";
						}
					else {
						upv = info.uptime;
						upu = "second";
						}
					}
				}
			upm = (upv == 1) ? "" : "s";
			printf("Uptime;%d;UPTIME %s - up %lu %s%s|uptime=%lus\n", rc, statusword(rc), upv, upu, upm, info.uptime);
			}

		/* processing for percent swap free */
		long fswap = info.totalswap / 100;
		if (fswap) {
			fswap = info.freeswap / fswap;
			rc = thresher("Swap_free", fswap);
			printf("Swap free;%d;SWAP %s - %ld%% of swap free|swap=%ld\n", rc, statusword(rc), fswap, fswap);
			}

		/* processing for number of processes */
		rc = thresher("Processes", info.procs);
		printf("Processes;%d;PROCS %s - %u total processes|procs=%u\n", rc, statusword(rc), info.procs, info.procs);

		}

	return(0);
	}
