#include <stdio.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <string.h>
#include "snag.h"

int snagdf () {
	FILE *mntf;
	struct mntent *ment;
	struct statvfs vfs;
	int rc = 0;

	mntf = setmntent("/etc/mtab", "r");
	if (!mntf) return(1);

	while (ment = getmntent(mntf)) {
		if (*(ment->mnt_fsname) == '/') {
			int rc = statvfs(ment->mnt_dir, &vfs);
			if (rc) {
				perror(ment->mnt_dir);
				rc = 1;
				}
			else {
				unsigned long fblox = vfs.f_bavail;
				int col;
				int bfree = 100 * vfs.f_bavail / vfs.f_blocks;
				int ifree = 100 * vfs.f_favail / vfs.f_files;
				unsigned long megavail = vfs.f_bavail / (1048576 / vfs.f_bsize);
				unsigned long iavail = vfs.f_favail;
				char *sdup = strdup(ment->mnt_dir);
				char *show;
				if (!sdup) {
					show = "NULL";
					}
				else {
					show = sdup;
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
					}
				col = thresher("Disk_space", bfree);
				printf("Disk space on %s;%d;DISK %s - %s %lu MB (%d%%) free space|%s=%luMB\n", show, col, sword[col], ment->mnt_dir, megavail, bfree, ment->mnt_dir, megavail);
				col = thresher("Disk_inodes", ifree);
				printf("Disk inodes on %s;%d;INODES %s - %s %lu inodes (%d%%) free|%s=%lu inodes\n", show, col, sword[col], ment->mnt_dir, iavail, ifree, ment->mnt_dir, iavail);
				free(sdup);

				}
			}
		}

	endmntent(mntf);
	return(rc);
	}
