#ifndef lint	/* .../appleprint/dwb/text/roff.d/troff.d/devi10/ext.h */
#define _AC_NAME ext_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:04:38}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *ext_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of ext.h on 87/11/11 22:04:38";
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
 * External definitions for the variables defined in glob.c. I've also
 * included a few commonly used function declarations just for convenience.
 *
 */


extern char	**argv;			/* global so everyone can use them */
extern int	argc;

extern char	*prog_name;		/* really just for error messages */

extern int	x_stat;			/* program exit status */
extern int	debug;			/* debug flag */
extern int	ignore;			/* what we do with FATAL errors */
extern long	lineno;			/* really just for post-processor */

extern char	*fontdir;		/* troff's binary font table directory */
extern char	*rastdir;		/* Imagen raster table directory */


extern char	*malloc();
extern char	*rastalloc();
extern char	*cuserid();

