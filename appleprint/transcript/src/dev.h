#ifndef lint	/* .../dev.h */
#define _AC_NAME dev_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985 Adobe Systems Incorporated, All Rights Reserved.  {Apple version 1.2 87/11/11 21:45:59}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char ___src_dev_h[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of dev.h on 87/11/11 21:45:59";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/* ditroff device descript structures (DESC file) */
/* RCSID: $Header: dev.h,v 2.1 85/11/24 11:48:53 shore Rel $ */
/*
	dev.h: characteristics of a typesetter
*/

struct dev {
	short	filesize;	/* number of bytes in file, */
				/* excluding dev part */
	short	res;		/* basic resolution in goobies/inch */
	short	hor;		/* goobies horizontally */
	short	vert;
	short	unitwidth;	/* size at which widths are given, in effect */
	short	nfonts;		/* number of fonts physically available */
	short	nsizes;		/* number of sizes it has */
	short	sizescale;	/* scaling for fractional point sizes */
	short	paperwidth;	/* max line length in units */
	short	paperlength;	/* max paper length in units */
	short	nchtab;		/* number of funny names in chtab */
	short	lchname;	/* length of chname table */
	short	spare1;		/* in case of expansion */
	short	spare2;
};

struct font {		/* characteristics of a font */
	char	nwfont;		/* number of width entries for this font */
	char	specfont;	/* 1 == special font */
	char	ligfont;	/* 1 == ligatures exist on this font */
	char	spare1;		/* unused for now */
	char	namefont[10];	/* name of this font (e.g., "R" */
	char	intname[10];	/* internal name (=number) on device, in ascii */
};

/* ligatures, ORed into ligfont */

#define	LFF	01
#define	LFI	02
#define	LFL	04
#define	LFFI	010
#define	LFFL	020
