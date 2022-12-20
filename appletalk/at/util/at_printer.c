#ifndef lint	/* .../appletalk/at/util/at_printer.c */
#define _AC_NAME at_printer_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:03:18}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_printer.c on 87/11/11 21:03:18";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
	int fd, fd2;
	int i;
	int lflg = 0;
	char buff[512];
	char *object, *type, *zone;
	char *exec;
	extern char at_pap_status[];

	exec = argv[0];
	if (argc >= 2 && strcmp(argv[1], "-l") == 0) {
		lflg = 1;
		argc--;
		argv++;
	}
	if (argc < 2) {
		object = "=";
	} else {
		object = argv[1];
	}
	if (argc < 3) {
		type = "Laserwriter";
	} else {
		type = argv[2];
	}
	if (argc < 4) {
		zone = "*";
	} else {
		zone = argv[3];
	}
	if (argc > 4) {
		fprintf(stderr, "Usage: %s [object [type [zone]]]\n", exec);
		exit(2);
	}
	at_error_set(0);
	if ((fd = at_pap_open_nve(object, 0, type, 0, zone, 0, 2, 1, -1, buff))<0) {
		fprintf(stderr, "%s: cannot access %s:%s@%s %s\n", exec,
				object, type, zone, at_pap_status);
		exit(2);
	}
	if (lflg)
		at_pap_read_ignore(fd);
	fprintf(stderr, "%s: printing on %s\n", exec, buff);
	for (;;) {
		i = read(0, buff, 512);
		if (i <= 0) {
			i = at_pap_write_eof(fd, buff, 0);
			if (i < 0) {
				fprintf(stderr, "%s: write failed ", exec);
				perror("");
				exit(2);
			}
			break;
		}
		i = at_pap_write(fd, buff, i);
		if (i < 0) {
			fprintf(stderr, "%s: write failed ", exec);
			perror("");
			exit(2);
		}
	}
}

