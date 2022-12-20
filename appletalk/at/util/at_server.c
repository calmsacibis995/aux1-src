#ifndef lint	/* .../appletalk/at/util/at_server.c */
#define _AC_NAME at_server_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:03:21}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_server.c on 87/11/11 21:03:21";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
main(argc, argv)
int argc;
char **argv;
{
	int fd, fd2;
	int i;
	char buff[512];
	char *type;
	FILE *file;
	union wait stat;

	if (argc < 3 || argc > 4) {
		fprintf(stderr, "Usage: %s command object [type]\n",argv[0]);
		exit(2);
	}
	if (argc == 3) {
		type = "LaserWriter";
	} else {
		type = argv[3];
	}
	at_error_set(0);
	if ((fd = at_papsl_init_nve(argv[2], 0, type, 0, 1, 1, "status: idle"))<0) {
		fprintf(stderr, "%s: registration failed for %s:%s@*\n",argv[0],
					argv[2],type);
		exit(2);
	}
	for (;;) {
		wait3(&stat, WNOHANG, 0);
		fd2 = at_papsl_get_next_job(fd);
		if (fd2 < 0) {
			perror("get next failed");
			exit(2);
		}
		if (fork() == 0) {
			file = popen(argv[1], "w");
			if (file == NULL) {
				fprintf(stderr, "%s: exec of '%s' failed\n",
					argv[0], argv[1]);
				exit(2);
			}
			for (;;) {
				i = at_pap_read(fd2, buff, 512);
				if (i == 0) {
					break;
				}
				fwrite(buff, 1, i, file);
			}
			exit(0);
		} else {
			at_pap_close(fd2);
		}
	}
}
