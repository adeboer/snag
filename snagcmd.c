/* snagcmd.c
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

#include "snag.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/* the standardmost Unix shell */
#define THESHELL "/bin/sh"

static void alcatch(int signo) {
	exit(1);
	}

/*
 * Each cnode is used to store information regarding a process we
 * started, so that we can handle it appropriately when it exits
 * to us.
 *
 * Since snag itself exits soon thereafter, freeing these would be
 * more trouble than it's worth.
 */

struct cnode {
	char *name;		/* name of check */
	pid_t cpid;		/* PID of process that was started */
	int cfd;		/* descriptor of pipe from process */
	struct cnode *next;	/* linked list next element */
	};

/* linked list of cnodes */
struct cnode *chead = NULL;

void sysdie(char *s) {
	perror(s);
	printf("CRITICAL - %s: %s\n", s, strerror(errno));
	exit(3);
	}

void die(char *s) {
	fprintf(stderr, "%s\n", s);
	printf("CRITICAL - %s\n", s);
	exit(3);
	}

/* initialize signals and set trap for slow ones */
void setupcmd() {
	if (signal(SIGCHLD, SIG_DFL) == SIG_ERR) die("signal() failure\n");
	if (signal(SIGALRM, alcatch) == SIG_ERR) die("signal() failure\n");
	alarm(8);
	}

/* start a check.  Args are checkname and command line */
void startcmd(char *name, char *cmdline) {
	struct cnode *ncn;
	int fildes[2];
	pid_t child;

	ncn = (struct cnode *)malloc(sizeof(struct cnode));
	if (!ncn) {
		fprintf(stderr, "malloc failure\n");
		exit(1);
		}
	if (pipe(fildes)) sysdie("pipe");
	child = fork();
	switch(child) {
		case 0:
			if (dup2(fildes[1], 1) == -1) sysdie("dup2 stdout");
			close(fildes[0]);
			close(fildes[1]);
			closeconfig();
			execl(THESHELL, THESHELL, "-c", cmdline, NULL);
			sysdie(THESHELL);
			;;
		case -1:
			sysdie("cannot fork");
			;;
		default:
			close(fildes[1]);
		}
	ncn->cpid = child;
	ncn->cfd = fildes[0];
	ncn->name = name;
	ncn->next = chead;
	chead = ncn;
	}

void cleancmd() {
	char buf[514];
	struct cnode *cn;
	int status, es, rc;
	pid_t wpid;
	char *sn;

	while(1) {
		wpid = wait(&status);
		if (wpid == -1) {
			if (errno == ECHILD) {
				break;
				}
			else {
				sysdie("wait error");
				}
			}
		if (WIFEXITED(status)) {
			es = WEXITSTATUS(status);
			if (es < 0 || es > 3) {
				es = 3;
				}
			}
		else {
			es = 3;
			}
		cn = chead;
		while(cn) {
			if (cn->cpid == wpid)
				break;
			else cn = cn->next;
			}
		if (cn) {
			rc = read (cn->cfd, buf, 512);
			if (rc < 0) {
				sysdie("read");
				}
			else {
				buf[rc] = '\0';
				}
			/* ensure that buf is exactly one line */
			buf[512] = '\0';
			sn = strchr(buf, '\n');
			if (sn) {
				*(sn+1) = '\0';
				}
			else {
				strcat(buf, "\n");
				}
			printf("%s;%d;%s", cn->name, es, buf);
			}
		else {
			printf("unknown child pid %d;3;???\n", wpid);
			}
		}
	}
