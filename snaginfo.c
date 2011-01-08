/* snaginfo.c - call sysinfo and process data from it
 *
 *	Copyright (C) 2006,2008,2010,2011 Anthony de Boer
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

void printload(int ai, int af, int bi, int bf, int ci, int cf)
{
	int rc = 0;
	rc |= gthresher("load1", ai);
	rc |= gthresher("load5", bi);
	rc |= gthresher("load15", ci);
	rc = gnormalize(rc);

	printf("Load average;%d;LOAD %s - load average: %d.%02d %d.%02d %d.%02d|load1=%d.%02d load5=%d.%02d load15=%d.%02d\n", rc, statusword(rc), ai, af, bi, bf, ci, cf, ai, af, bi, bf, ci, cf);
}

void printuptime(unsigned long uptime)
{
	int rc = thresher("Uptime", uptime);
	if (rc < 3) {
		char *upu, *upm;
		unsigned long upv = uptime / 86400;
		if (upv) {
			upu = "day";
			}
		else {
			upv = uptime / 3600;
			if (upv) {
				upu = "hour";
				}
			else {
				upv = uptime / 60;
				if (upv) {
					upu = "minute";
					}
				else {
					upv = uptime;
					upu = "second";
					}
				}
			}
		upm = (upv == 1) ? "" : "s";
		printf("Uptime;%d;UPTIME %s - up %lu %s%s|uptime=%lus\n", rc, statusword(rc), upv, upu, upm, uptime);
	}
}

void printswap(unsigned long totalswap, unsigned long freeswap)
{
	long fswap = totalswap / 100;
	if (fswap) {
		fswap = freeswap / fswap;
		int rc = thresher("Swap_free", fswap);
		printf("Swap free;%d;SWAP %s - %ld%% of swap free|swap=%ld\n", rc, statusword(rc), fswap, fswap);
	}
}

void printprocs(unsigned int nprocs)
{
	int rc = thresher("Processes", nprocs);
	printf("Processes;%d;PROCS %s - %u total processes|procs=%u\n", rc, statusword(rc), nprocs, nprocs);
}

#ifdef HAVE_SYSINFO

#include <sys/sysinfo.h>
#include <sys/stat.h>

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
	struct stat sbuf;
	int gotuptime = 0;

	if (stat("/proc/kmsg", &sbuf) != -1) {
		time_t now = time(NULL);
		unsigned long uptime = now - sbuf.st_mtime;
		printuptime(uptime);
		gotuptime = 1;
	}

	if (sysinfo(&info)) {
		perror("sysinfo");
		return 1;
	} else {
		/* processing for load average */
		int a = info.loads[0] + (FIXED_1/200);
		int b = info.loads[1] + (FIXED_1/200);
		int c = info.loads[2] + (FIXED_1/200);
		int ai = LOAD_INT(a);
		int af = LOAD_FRAC(a);
		int bi = LOAD_INT(b);
		int bf = LOAD_FRAC(b);
		int ci = LOAD_INT(c);
		int cf = LOAD_FRAC(c);

		printload(ai, af, bi, bf, ci, cf);
		if (!gotuptime) printuptime(info.uptime);
		printswap(info.totalswap, info.freeswap);
		printprocs(info.procs);
		return snagprocs();
	}
}

#else

/* FreeBSD / OpenBSD model */

#include <sys/param.h>
#include <sys/sysctl.h>
#include <time.h>
#include <vm/vm_param.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/user.h>

#ifdef HAVE_KVM_H
#include <kvm.h>
#endif

#ifdef HAVE_SYS_SWAP_H
#include <sys/swap.h>
#define MAXSWAPS 16
#endif

int snaginfo()
{
	size_t oldlen;

	/* loadavg */
	int getloadavg[] = { CTL_VM, VM_LOADAVG, };
	struct loadavg lav;
	oldlen = sizeof(lav);
	if (sysctl(getloadavg, 2, &lav, &oldlen, NULL, 0) == -1) {
		perror("sysctl - LOADAVG");
	} else {
		int ai = lav.ldavg[0] / lav.fscale;
		int af = 100 * (lav.ldavg[0] - lav.fscale * ai) / lav.fscale;
		int bi = lav.ldavg[1] / lav.fscale;
		int bf = 100 * (lav.ldavg[1] - lav.fscale * bi) / lav.fscale;
		int ci = lav.ldavg[2] / lav.fscale;
		int cf = 100 * (lav.ldavg[2] - lav.fscale * ci) / lav.fscale;
		printload(ai, af, bi, bf, ci, cf);
	}

	/* uptime */
	int getboottime[] = { CTL_KERN, KERN_BOOTTIME, };
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long uptime = tv.tv_sec;
	oldlen = sizeof(tv);
	if (sysctl(getboottime, 2, &tv, &oldlen, NULL, 0) == -1) {
		perror("sysctl - BOOTTIME");
	} else {
		uptime -= tv.tv_sec;
		printuptime(uptime);
	}

#ifdef HAVE_KVM_OPEN
	/* swap and nprocs */
	int nprocs;
	kvm_t *kd;
	struct kvm_swap kswap;

	kd = kvm_open(NULL, "/dev/null", NULL, O_RDONLY, "snag/kvm");
	if (kd != NULL) {
		kvm_getswapinfo(kd, &kswap, 1, 0);
		printswap(kswap.ksw_total, kswap.ksw_total - kswap.ksw_used);
		unsigned int nprocs;
		struct kinfo_proc *kip;
		kip = kvm_getprocs(kd, KERN_PROC_PROC, 0, &nprocs);
		if (kip == NULL) {
			printf("kvm_getprocs failed\n");
		} else {
			int proci;
			int sysuidmax = getvar("sysuidmax");
			int transient = getvar("transient_time");
			struct timeval now;
			gettimeofday(&now, NULL);
			printprocs(nprocs);
			for (proci = 0; proci < nprocs; proci++) {
				unsigned long long sex = now.tv_sec -
					kip->ki_start.tv_sec;
				int oldproc = (sex > transient);
				if (kip->ki_uid <= sysuidmax) {
					procfound(kip->ki_comm, oldproc);
				}
				kip++;
			}
			procfinal();
		}
	}
	kvm_close(kd);
#else
	/* nprocs */
	int getnprocs[] = { CTL_KERN, KERN_NPROCS, };
	int numprocs;
	oldlen = sizeof(numprocs);
	if (sysctl(getnprocs, 2, &numprocs, &oldlen, NULL, 0) == -1) {
		perror("sysctl - NPROCS");
	} else {
		printprocs(numprocs);
	}

	/* swap */
	struct swapent swe[MAXSWAPS];
	int nswaps = swapctl(SWAP_STATS, swe, MAXSWAPS);
	if (nswaps == -1) {
		perror("swapctl");
	} else {
		unsigned long tblks;
		unsigned long binuse;
		int i;
		for (i=0; i<nswaps; i++) {
			tblks += swe[i].se_nblks;
			binuse += swe[i].se_inuse;
		}
		printswap(tblks, tblks-binuse);
	}
#endif

}

#endif /* HAVE_SYSINFO */
