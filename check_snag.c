/* check_snag.c
 *
 *	Copyright (C) 2005,2011,2015 Anthony de Boer
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include "config.h"

#define ME "check_snag"

char inbuf[1025];
char outbuf[2048];
int insize = 0;
int outsize = 0;
time_t now;
long pipemax;
int ifd, ofd;
char *host;
int debug = 0;

void die(char *s) {
	fprintf(stderr, "%s\n", s);
	printf("CRITICAL - %s\n", s);
	exit(3);
	}

void sysdie(char *s) {
	perror(s);
	printf("CRITICAL - %s: %s\n", s, strerror(errno));
	exit(3);
	}

void processaline(char *s) {
	if (debug) fprintf(stderr, "Got \"%s\"\n", s);
	int need = 44 + strlen(host) + strlen(s);
	int pmax = pipemax > sizeof(outbuf) ? sizeof(outbuf) : pipemax;
	int pres;
	if (outsize > 0 && outsize + need > pmax) {
		if (debug) fprintf(stderr, "writing %d bytes\n", outsize);
		int rc = write(ofd, outbuf, outsize);
		if (rc < 0) sysdie("write");
		if (rc < outsize) die("short write");
		outsize = 0;
		}
	pmax = sizeof(outbuf) - outsize;
	pres = snprintf(outbuf+outsize, pmax, "[%lu] PROCESS_SERVICE_CHECK_RESULT;%s;%s\n", now, host, s);
	outsize += pres;
	if (pres > need) fprintf(stderr, "warning: need calc insufficient\n");
	}

int my_read() {
	char *p, *q;
	int rc, rsz;
	rsz = sizeof(inbuf)-1-insize;
	if (rsz < 1) {
		die("excessiveline");
		}
	rc = read(ifd, inbuf+insize, sizeof(inbuf)-1-insize);
	if (debug) fprintf(stderr, "read returns %d on %d\n", rc, ifd);
	if (rc > 0) {
		insize += rc;
		inbuf[insize] = '\0';
		p = inbuf;
		while (q = strchr(p, '\n')) {
			*q++ = '\0';
			processaline(p);
			p = q;
			}
		insize = insize - (p - inbuf);
		if (insize) memcpy(inbuf, p, insize);
		return 0;
		}
	else if (rc < 0 && errno == EAGAIN) {
		return 0;
		}
	else if (rc < 0) {
		sysdie("read");
		}	
	else {
		return 1;
		}
	}

int sloop() {
	int rc;
	fd_set readfds, writefds, exceptfds;
	struct timeval timeout;
	time_t expiry = 0;
	while(1) {
		gettimeofday(&timeout, NULL);
		if (!expiry) {
			now = timeout.tv_sec;
			expiry = timeout.tv_sec + 8;
			}
		timeout.tv_sec = expiry - timeout.tv_sec;
		if (timeout.tv_sec < 1) return 1;
		timeout.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
		FD_SET(ifd, &readfds);
		FD_SET(ifd, &exceptfds);
		if (debug) fprintf(stderr, "selecting...\n");
		rc = select(ifd+1, &readfds, &writefds, &exceptfds, &timeout);
		if (rc > 0) {
			if (debug) fprintf(stderr, "something...\n");
			if (FD_ISSET(ifd, &readfds)) {
				if (my_read()) return 0;
				}
			else if (FD_ISSET(ifd, &exceptfds)) {
				die("Exception on pipe socket");
				}
			}
		else if (rc < 0 && errno != EINTR) {
			sysdie("select");
			}
		}
	}

void usage() {
	die("usage: " ME " host command [args]\n");
	}

int main (int argc, char **argv) {

	int i, fildes[2], pid, wpid, status, flags, es;
	int nagtest = 0;

	while(1) {
		switch(getopt(argc, argv, "+dt")) {
			case 'd':
				debug = 1;
				break;
			case 't':
				nagtest = 1;
				break;
			case EOF:
				goto doneopts;
			default:
				usage();
			}
		}
	doneopts:
	/* nagtest = debug = 1;
	optind = 1; */

	if (argc < optind+2) usage();
	if (debug) fprintf(stderr, "optind %d\n", optind);
	host = argv[optind++];
	if (debug) fprintf(stderr, "host %s\n", host);

	if (nagtest) {
		ofd = 1;
		}
	else {
		/* Set up pipe to Nagios FIFO.  Open nonblocking, so we get
		ENXIO if Nagios is dead, then set blocking so we can write to
		it without being fancy.  */
		ofd = open(CMDPIPE, O_WRONLY | O_NONBLOCK, 0);
		if (ofd == -1) sysdie(CMDPIPE);
		flags = fcntl(ofd, F_GETFL, 0);
		if (flags == -1) sysdie("F_GETFL");
		flags &= ~O_NONBLOCK;
		if (fcntl(ofd, F_SETFL, flags) == -1) sysdie("F_SETFL");
		}

	/* get max atomic pipe write size */
	pipemax = fpathconf(ofd, _PC_PIPE_BUF);
	if (debug) fprintf(stderr, "PC_PIPE_BUF %ld\n", pipemax);
	if (pipemax < 512) pipemax = 512;

	/* Fork to run command that gets data as a child, with its STDOUT
	coming to us. */
	if (pipe(fildes)) sysdie("pipe");
	if (signal(SIGCHLD, SIG_DFL) == SIG_ERR) die("signal() failure\n");
	pid = fork();
	switch(pid) {
		case 0:
			if (dup2(fildes[1], 1) == -1) sysdie("dup2 stdout");
			close(fildes[0]);
			close(fildes[1]);
			if (!nagtest) close(ofd);
			if (debug) fprintf(stderr, "exec %s\n", argv[optind]);
			execvp(argv[optind], argv+optind);
			sysdie(argv[optind]);
			;;
		case -1:
			sysdie("cannot fork");
			;;
		default:
			close(fildes[1]);
			/* close(0); */
			/* close(1); */
			;;
		}

	/* nonblocking on read from subcommand pipe */
	ifd = fildes[0];
	if (debug) fprintf(stderr, "ifd is %d ofd is %d\n", ifd, ofd);
	flags = fcntl(ifd, F_GETFL, 0);
	if (flags == -1) sysdie("F_GETFL");
	flags |= O_NONBLOCK;
	if (fcntl(ifd, F_SETFL, flags) == -1) sysdie("F_SETFL");

	flags = sloop();
	close(ifd);

	if (debug) fprintf(stderr, "flushing %d bytes\n", outsize);
	if (outsize) {
		int rc = write(ofd, outbuf, outsize);
		if (rc < 0) sysdie("write");
		if (rc < outsize) die("short write");
		}

	/* if (fsync(ofd) == -1) sysdie("fsync"); */
	if (!nagtest) close(ofd);

	wpid = wait(&status);
	if (wpid == -1) sysdie("wait error");
	if (wpid != pid) die("wrong pid?!?\n");

	if (flags) {
		printf("WARNING - timeout\n");
		es = 1;
		}
	else if (WIFEXITED(status)) {
		es = WEXITSTATUS(status);
		if (es) {
			printf("CRITICAL - check exit %d\n", es);
			es = 2;
			}
		else {
			printf("OK - check fine\n");
			}
		}
	else {
		printf("CRITICAL - check killed\n");
		es = 2;
		}

	if (debug) fprintf(stderr, "exit status %d\n", es);
	exit(es);
	}
