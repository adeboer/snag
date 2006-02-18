#include <stdio.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <string.h>
#include "snag.h"

#define debug 0

int snagdf () {
	FILE *mntf;
	struct mntent *ment;
	struct statvfs vfs;
	int rc = 0;

	mntf = setmntent("/etc/mtab", "r");
	if (!mntf) return(1);

	while (ment = getmntent(mntf)) {
		if (*(ment->mnt_fsname) == '/') {
			if (debug) fprintf(stderr, "looking at fsname %s on dir %s of type %s with options %s.\n", ment->mnt_fsname, ment->mnt_dir, ment->mnt_type, ment->mnt_opts);
			int rc = statvfs(ment->mnt_dir, &vfs);
			if (rc) {
				perror(ment->mnt_dir);
				rc = 1;
				}
			else if (hasmntopt(ment, "ro")) {
				if (debug) fprintf(stderr, "it is readonly\n");
				}
			else {
				if (debug) fprintf(stderr, "got data\n");
				if (debug) fprintf(stderr, "bavail %lu blocks %lu favail %lu files %lu bzise %lu\n", vfs.f_bavail, vfs.f_blocks, vfs.f_favail, vfs.f_files, vfs.f_bsize);
				int col;
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
				if (vfs.f_blocks && vfs.f_bsize) {
					int bfree = 100 * vfs.f_bavail / vfs.f_blocks;
					unsigned long megavail = vfs.f_bavail / (1048576 / vfs.f_bsize);
					col = thresher("Disk_space", bfree);
					printf("Disk space on %s;%d;DISK %s - %s %lu MB (%d%%) free space|%s=%luMB\n", show, col, sword[col], ment->mnt_dir, megavail, bfree, ment->mnt_dir, megavail);
					}
				if (vfs.f_files) {
					int ifree = 100 * vfs.f_favail / vfs.f_files;
					unsigned long iavail = vfs.f_favail;
					col = thresher("Disk_inodes", ifree);
					printf("Disk inodes on %s;%d;INODES %s - %s %lu inodes (%d%%) free|%s=%lu inodes\n", show, col, sword[col], ment->mnt_dir, iavail, ifree, ment->mnt_dir, iavail);
					}
				free(sdup);

				}
			}
		}

	endmntent(mntf);
	return(rc);
	}
