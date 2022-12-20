#ifndef lint	/* .../appleprint/iw/daiw/dev.h */
#define _AC_NAME dev_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:45:11}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _dev_h[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of dev.h on 87/11/11 21:45:11";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *	dev.h: characteristics of a typesetter
 */

struct dev {
	unsigned short	filesize;	/* number of bytes in file, */
					/* excluding dev part */
	unsigned short	res;		/* basic resolution in goobies/inch */
	unsigned short	hor;		/* goobies horizontally */
	unsigned short	vert;
	unsigned short	unitwidth;	/* size at which widths are given, in effect */
	unsigned short	nfonts;		/* number of fonts physically available */
	unsigned short	nsizes;		/* number of sizes it has */
	unsigned short	sizescale;	/* scaling for fractional point sizes */
	unsigned short	paperwidth;	/* max line length in units */
	unsigned short	paperlength;	/* max paper length in units */
	unsigned short	nchtab;		/* number of funny names in chtab */
	unsigned short	lchname;	/* length of chname table */
	unsigned short	biggestfont;	/* #chars in largest ever font */
	unsigned short	spare2;		/* in case of expansion */
};

struct font {			/* characteristics of a font */
	unsigned char	nwfont;		/* number of width entries for this font */
	unsigned char	specfont;	/* 1 == special font */
	unsigned char	ligfont;	/* 1 == ligatures exist on this font */
	unsigned char	spare1;		/* unused for now */
	char		namefont[10];	/* name of this font (e.g., "R" */
	char		intname[10];    /* internal name(=number) on device,in ascii */
};

/* ligatures, ORed into ligfont */

#define	LFF	01
#define	LFI	02
#define	LFL	04
#define	LFFI	010
#define	LFFL	020
