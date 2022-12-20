#ifndef lint	/* .../appleprint/dwb/text/subndx.d/conp.h */
#define _AC_NAME conp_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:10:49}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *conp_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of conp.h on 87/11/11 22:10:49";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#define SLENG 250
#define SCHAR 1500
extern struct ss {
	char	*sp, ic, cc; 
	int	leng;
} sent[SLENG];
extern struct ss *sentp;
extern comma, j, i;
extern int	nsleng;
extern question;
extern int	must;
extern int	be;
extern int	sav;
extern int	prep;
extern int	aflg, bflg, subty, verb, verbty;
extern int	hflg;
extern int	iverb, pverb, done;
