#ifndef lint	/* .../appleprint/dwb/text/roff.d/ext.h */
#define _AC_NAME ext_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:08:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *ext_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of ext.h on 87/11/11 22:08:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
extern	char	**argp;
extern	char	*chname;
extern	char	*eibuf;
extern	char	*enda;
extern	char	*fitab[];
extern	char	*fontab[];
extern	char	*kerntab[];
extern	char	*ibufp;
extern	char	*obufp;
extern	char	*unlkp;
extern	char	*xbufp;
extern	char	*xeibuf;
extern	char	cfname[NSO+1][NS];
extern	char	devname[];
extern	char	fontfile[];
extern	char	ibuf[IBUFSZ];
extern	char	mfiles[NMF][NS];
extern	char	nextf[];
extern	char	obuf[],	*obufp;
extern	char	termtab[],	fontfile[];
extern	char	xbuf[IBUFSZ];
extern	filep	apptr;
extern	filep	ip;
extern	filep	nextb;
extern	filep	offset;
extern	filep	roff;
extern	filep	woff;
extern	short	*chtab;
extern	int	*pnp;
extern	short	*pstab;
extern	int	app;
extern	int	ascii;
extern	int	bd;
extern	int	bdtab[];
extern	int	ccs;
extern	int	copyf;
extern	int	cs;
extern	int	dfact;
extern	int	dfactd;
extern	int	diflg;
extern	int	dilev;
extern	int	donef;
extern	int	dotT;
extern	int	dpn;
extern	int	ds;
extern	int	ejf;
extern	int	em;
extern	int	eqflg;
extern	int	error;
extern	int	esc;
extern	int	eschar;
extern	int	ev;
extern	int	evi;
extern	int	evlist[EVLSZ];
extern	int	fc;
extern	int	flss;
extern	int	fontlab[];
extern	int	gflag;
extern	int	hflg;
extern	int	ibf;
extern	int	ifi;
extern	int	ifile;
extern	int	ifl[NSO];
extern	int	iflg;
extern	int	init;
extern	int	lead;
extern	int	lg;
extern	int	lgf;
extern	int	macerr;
extern	int	mflg;
extern	int	mfont;
extern	int	mlist[NTRAP];
extern	int	mpts;
extern	int	ndone;
extern	int	newmn;
extern	int	nflush;
extern	int	nfo;
extern	int	nfonts;
extern	int	nform;
extern	int	nhyp;
extern	int	nlflg;
extern	int	nlist[NTRAP];
extern	int	nmfi;
extern	int	no_out;
extern	int	nofeed;
extern	int	nonumb;
extern	int	noscale;
extern	int	npn;
extern	int	npnflg;
extern	int	nx;
extern	int	oldbits;
extern	int	oldmn;
extern	int	over;
extern	int	padc;
extern	int	pfont;
extern	int	pfrom;
extern	int	pipeflg;
extern	int	pl;
extern	int	pnlist[];
extern	int	po1;
extern	int	po;
extern	int	ppts;
extern	int	print;
extern	int	ptid;
extern	int	pto;
extern	int	quiet;
extern	int	ralss;
extern	int	rargc;
extern	int	raw;
extern	int	res;
extern	int	setwdf;
extern	int	sfont;
extern	int	smnt;
extern	int	stdi;
extern	int	stop;
extern	int	sv;
extern	int	tabch,	ldrch;
extern	int	tflg;
extern	int	totout;
extern	int	trap;
extern	int	trtab[];
extern	int	tty;
extern	int	ttyod;
extern	int	ulfont;
extern	int	vflag;
extern	int	wbfi;
extern	int	widthp;
extern	int	xfont;
extern	int	xpts;
extern	no_out;
extern	struct	s	*ejl;
extern	struct	s	*frame,	*stk,	*nxf;
extern	tchar	**hyp;
extern	tchar	*olinep;
extern	tchar	pbbuf[NC];
extern	tchar	*pbp;
extern	tchar	*lastpbp;
extern	tchar	ch;
extern	tchar	nrbits;
extern	tchar	oline[];
extern	struct widcache {	/* width cache, indexed by character */
	short	fontpts;
	short	width;
} widcache[NWIDCACHE];
extern	char gchtab[];
extern	struct	d	d[NDI];
extern	struct	d	*dip;
