#ifndef lint	/* .../appleprint/dwb/text/checkmm.d/chekmain1.c */
#define _AC_NAME chekmain1_c
#define _AC_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:50:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of chekmain1.c on 87/11/11 21:50:24";
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

	/* This is the second pass through the file */
	/* This is required because a single lex file  generates
	 * a file too big for the 'as' on SVR2 to create the '.o'
	 * file. 						
	 * This is executed from within 'checkmm'		*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int errors = 0;

main(argc, argv) 

char **argv ; 
int argc ;

{
extern int yylineno ;
extern FILE *yyin ;
FILE *fin ;

struct stat stbuf ;
	if (argc == 1)
		{ yyin = stdin;
		  yylineno = 1 ;
		  yylex() ;
		}
	else
	while (--argc > 0) {
		stat(*++argv, &stbuf) ;

		if ( stbuf.st_mode &  S_IFDIR ) {
			continue ;
		}

		if ((fin = fopen(*argv, "r")) == NULL) {
			printf("Can't open %s\n", *argv) ;
			continue ; 
		}
		yyin = fin ;
		yylineno = 1 ;
		yylex() ;
		fclose(fin) ;
	}

	exit(errors);
}
