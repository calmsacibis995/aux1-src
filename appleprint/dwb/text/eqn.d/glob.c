#ifndef lint	/* .../appleprint/dwb/text/eqn.d/glob.c */
#define _AC_NAME glob_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.3 87/11/11 21:51:17}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.3 of glob.c on 87/11/11 21:51:17";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include "e.h"

int	dbg;		/* debugging print if non-zero */
int	lp[200];		/* stack for things like piles and matrices */
int	ct;		/* pointer to lp */
int	used[100];	/* available registers */
int	ps;		/* default init point size */
int	deltaps	= 3;	/* default change in ps */
int	dps_set = 0;	/* 1 => -p option used */
int	gsize	= 10;	/* default initial point size */
int	ft	= '2';
Font	ftstack[10] = { '2', "2" };	/* bottom is global font */
Font	*ftp	= ftstack;
int	szstack[10];	/* non-zero if absolute size set at this level */
int	nszstack = 0;
int	display	= 0;	/* 1=>display, 0=>.EQ/.EN */

char *typesetter = "psc";
int ttype = DEVPSC;
int res = 576;
int minsize = 2;
int	synerr;		/* 1 if syntax err in this eqn */
float	eht[100];	/* height in ems at gsize */
float	ebase[100];	/* base: where one enters above bottom */
int	eps[100];	/* unused right now */
int	lfont[100];
int	rfont[100];
int	eqnreg;		/* register where final string appears */
float	eqnht;		/* final height of equation */
int	lefteq	= '\0';	/* left in-line delimiter */
int	righteq	= '\0';	/* right in-line delimiter */
int	markline = 0;	/* 1 if this EQ/EN contains mark; 2 if lineup */
