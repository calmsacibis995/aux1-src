#ifndef lint	/* .../appletalk/at/util/at_status.c */
#define _AC_NAME at_status_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:03:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_status.c on 87/11/11 21:03:24";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
	extern char at_pap_status[];
	char *type, *zone;

	if (argc < 2 || argc > 4) {
		fprintf(stderr, "Usage: %s object [type [zone]]\n",argv[0]);
		exit(2);
	}
	if (argv[1][0] == '=' && argv[1][1] == '\0') {
		fprintf(stderr, "%s: wild cards ('=') not supported\n",argv[0]);
		exit(2);
	}
	if (argc >= 3) {
		if (argv[2][0] == '=' && argv[2][1] == '\0') {
			fprintf(stderr, "%s: wild cards ('=') not supported\n",argv[0]);
			exit(2);
		}
		type = argv[2];
	} else {
		type = "LaserWriter";
	}
	if (argc >= 4) {
		zone = argv[3];
	} else {
		zone = "*";
	}
	at_error_set(0);
	if (at_pap_status_nve(argv[1], 0, type, 0, zone, 0, 5, 1)<0) {
		fprintf(stderr, "%s: status for '%s' failed\n",argv[0], argv[1]);
		exit(2);
	}
	printf("%s\n", at_pap_status);
	exit(0);
}

