#ifndef lint	/* .../appleprint/dwb/text/subndx.d/sbjprep.c */
#define _AC_NAME sbjprep_c
#define _AC_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:12:55}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of sbjprep.c on 87/11/11 22:12:55";
  char *_Version_ = "A/UX Release 1.0";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#define _AC_MODS

#include <stdio.h>
#define LINELENGTH 256                                           /* max length in chars of each subject */
#define SEP " \t"                                                /* separator string used by strtok */
#define SEP1 "\n"                                                /* separator string used by strtok */
main(argc, argv)
int	argc;
char	*argv[];
{
	FILE * ioptr1;
	char	str[LINELENGTH];
	char	*p, *strtok();
	int	i = 1;
	if ((ioptr1 = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "ndx:  CANNOT OPEN FILE %s\n", argv[1]);
		exit(1);
	} else
	 {
		while ((fgets(str, LINELENGTH, ioptr1)) != NULL) {
			if ((p = strtok(str, SEP)) != NULL)        /* read past subject number in file */
				;
			if ((p = strtok(0, SEP1)) != NULL)         /* read subject */ {
				while (*p == ' ' || *p == '\t')   /* strip leading blanks, tabs */
					p++;
				if (*p == '~')                    /* strip leading tilde */
					p++;
				printf("%d]  %s\n", i++, p);      /* print subject number and subject */
			}
		}
	}
	fclose(ioptr1);
	exit(0);
}


