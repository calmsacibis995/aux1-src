#ifndef lint	/* .../appleprint/dwb/text/subndx.d/style.h */
#define _AC_NAME style_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:13:07}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *style_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of style.h on 87/11/11 22:13:07";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
extern int	part;
extern int	style;
extern int	topic;
extern int	pastyle;
extern int	lstyle;
extern int	rstyle;
extern int	estyle;
extern int	nstyle;
extern int	Nstyle;
extern int	rthresh;
extern int	lthresh;
extern int	pstyle;
extern int	sstyle;
extern int	penflg;
extern int	pepflg;
extern int	pnpflg;
extern int	penpflg;
extern int	sthresh;
extern int	count;
extern int	sleng[50];
extern int	numsent;
extern long	letnonf;
extern long	numnonf;
extern long	vowel;
extern long	numwds;
extern long	twds;
extern long	numlet;
extern int	maxsent;
extern int	maxindex;
extern int	minindex;
extern int	qcount;
extern int	icount;
extern int	minsent;
extern int	simple;
extern int	compound;
extern int	compdx;
extern int	complex;
extern int	nomin;
extern int	tobe;
extern int	noun, infin, pron, aux, adv;
extern int	passive;
extern int	beg[15];
extern int	prepc;
extern int	conjc;
extern int	verbc;
extern int	tverbc;
extern int	adj;
#define MAXPAR 20
extern int	leng[];
extern sentno;
