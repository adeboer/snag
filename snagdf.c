/* snagdf.c - get disk-related statistics
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

#define debug 0

/* Convert to percentage, making sure we don't run into arithmetic
 * overflow.
 */
int hundiv (unsigned long numer, unsigned long denom) {
	if (denom > 1000000) {
		numer >>= 8;
		denom >>= 8;
		}
	return 100 * numer / denom;
	}

/*
 * filesystem type rules:
 * 0 to hide, 1 to show, 2 to show without inodes.
 * No need for OS-specific hacks unless the same filesystem name
 * gets used for sufficiently different things in different places.
 */

static struct fstable_t {
	char *fstname;
	int showit;
	} fstable[] = {
		{ "nfs", 0 },
		{ "zfs", 2 },
		{ "swap", 0 },
		{ NULL, 0 },
	};
/*
 * Handle one mountpoint:
 * mntdir - path to mountpoint
 * mntdev - device name
 * mnttype - filesystem type string
 * funny - flags like read-only that indicate to probably skip this mount
 * blocksize
 * tblocks - total blocks
 * avblocks - number of blocks free
 * tinodes - total inodes
 * avinodes - number of inodes free
 */

void printdisk(char *mntdir, char *mntdev, char *mnttype, int funny, unsigned long blocksize, unsigned long tblocks, unsigned long avblocks, unsigned long tinodes, unsigned long avinodes)
{
	int col;

	if (debug) {
		printf("Consider DIR=%s, DEV=%s TYPE=%s (%s)\n",
			mntdir ? mntdir : "NULL",
			mntdev ? mntdev : "NULL",
			mnttype ? mnttype : "NULL",
			funny ? "FUNNY" : "NORMAL");
	}

	char *sdup = strdup(mntdir);
	if (sdup == NULL) {
		/* memallocfail */
		return;
	}
	char *show = sdup;
	while(*show) {
		if (!isalnum(*show) && *show != '/') {
			*show = '_';
			}
		show++;
	}
	show = sdup;
	while(*show == '_') {
		show++;
	}
	if (!*show) {
		show = "root";
	}

	int specific = (thresher(show, 12345) < 3);
	int showme = specific;

	if (!specific && !funny && !getvar(show)) {
		struct fstable_t * fst;
		/* default: check if mntdev is other than /dev/...? */
		showme = mntdev != NULL && strncmp(mntdev, "/dev/", 5) == 0;
		/* unless filesystem type is known */
		for (fst = fstable; fst->fstname; fst++) {
			if (strcmp(fst->fstname, mnttype) == 0) {
				showme = fst->showit;
				break;
			}
		}
	}

	if (showme && tblocks && blocksize) {
		int bfree = hundiv(avblocks, tblocks);
		unsigned long megavail = avblocks / (1048576 / blocksize);
		col = thresher(specific ? show : "Disk_space", bfree);
		printf("Disk space on %s;%d;DISK %s - %s %lu MB (%d%%) free space|%s=%luMB\n", show, col, statusword(col), mntdir, megavail, bfree, mntdir, megavail);
	}
	if (showme == 1 && tinodes) {
		int ifree = hundiv(avinodes, tinodes);
		col = thresher(specific ? show : "Disk_inodes", ifree);
		printf("Disk inodes on %s;%d;INODES %s - %s %lu inodes (%d%%) free|%s=%lu inodes\n", show, col, statusword(col), mntdir, avinodes, ifree, mntdir, avinodes);
	}
	free(sdup);
}

#ifdef HAVE_GETMNTENT
#include <mntent.h>
#include <sys/statvfs.h>
int snagdf () {
	FILE *mntf;
	struct mntent *ment;
	struct statvfs vfs;
	int rc = 0;

	mntf = setmntent("/etc/mtab", "r");
	if (!mntf) return 1;

	while (ment = getmntent(mntf)) {
		if (debug) fprintf(stderr, "looking at fsname %s on dir %s of type %s with options %s.\n", ment->mnt_fsname, ment->mnt_dir, ment->mnt_type, ment->mnt_opts);
		int rc = statvfs(ment->mnt_dir, &vfs);
		if (rc) {
			perror(ment->mnt_dir);
			rc = 1;
			continue;
			}
		int funny = (hasmntopt(ment, "ro") != NULL) ||
			(hasmntopt(ment, "bind") != NULL);
		if (debug) fprintf(stderr, "bavail %lu blocks %lu favail %lu files %lu bsize %lu\n", vfs.f_bavail, vfs.f_blocks, vfs.f_favail, vfs.f_files, vfs.f_bsize);
		printdisk(ment->mnt_dir, ment->mnt_fsname, ment->mnt_type,
			funny, vfs.f_bsize,
			vfs.f_blocks, vfs.f_bavail,
			vfs.f_files, vfs.f_favail);
		}

	endmntent(mntf);
	return rc;
	}
#else

#ifdef HAVE_GETFSSTAT

#include <sys/param.h>
#include <sys/mount.h>

int snagdf ()
{
	size_t n1, n2, bs;
	struct statfs * fsbuf, * fscur;
	int i;

	n1 = getfsstat(NULL, 0, MNT_WAIT);
	if (debug) fprintf(stderr, "fs count %d\n", n1);
	if (n1 < 1) return 0;
	bs = n1 * sizeof(struct statfs);
 	fsbuf = (struct statfs *)malloc(bs);
	if (fsbuf == NULL) {
		fprintf(stderr, "statfs alloc failed\n");
		return 2;
		}
	if ((n2 = getfsstat(fsbuf, bs, MNT_WAIT)) == -1) {
		perror("getfsstat");
		return 2;
		}
	fscur = fsbuf;
	if (debug && n1 != n2) fprintf(stderr, "n1 %d n2 %d\n", n1, n2);
	for (i=0; i<n2; i++) {
		int funny = (fscur->f_flags & MNT_RDONLY);
		printdisk(fscur->f_mntonname, fscur->f_mntfromname,
			fscur->f_fstypename,
			funny, fscur->f_bsize,
			fscur->f_blocks, fscur->f_bavail,
			fscur->f_files, fscur->f_ffree);
		fscur++;
	}
	free(fsbuf);
	return 0;
}
#else
#error No snagdf implementation
#endif /* HAVE_GETFSSTAT */
#endif /* HAVE_GETMNTENT */

