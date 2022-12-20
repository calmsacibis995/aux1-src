#ifndef lint	/* .../appleprint/dwb/text/roff.d/troff.d/devi10/glob.c */
#define _AC_NAME glob_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:04:44}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of glob.c on 87/11/11 22:04:44";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS

/*
 *
 * Definition and initialization of most of the global variables.
 *
 */


#include "gen.h"			/* general purpose definitions */
#include "init.h"			/* printer and system definitions */


char	**argv;				/* global so everyone can use them */
int	argc;

char	*prog_name = "";		/* really just for error messages */

int	x_stat = 0;			/* program exit status */
int	debug = OFF;			/* debug flag */
int	ignore = OFF;			/* what we do with FATAL errors */
long	lineno = 0;			/* really just for post-processor */

char	*fontdir = FONTDIR;		/* troff's binary font table directory */
char	*rastdir = ".";			/* raster files are found right here */

