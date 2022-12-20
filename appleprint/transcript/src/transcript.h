#ifndef lint	/* .../transcript.h */
#define _AC_NAME transcript_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985 Adobe Systems Incorporated, All Rights Reserved.  {Apple version 1.2 87/11/11 21:48:46}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char ___src_transcript_h[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of transcript.h on 87/11/11 21:48:46";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/* transcript.h
 *
 * Copyright (C) 1985 Adobe Systems Incorporated
 *
 * RCSID: $Header: transcript.h,v 2.1 85/11/24 11:51:21 shore Rel $
 *
 * some general global defines
 *
 * Edit History:
 * Andrew Shore: Sun Nov 17 15:40:47 1985
 * End Edit History.
 *
 * RCSLOG:
 * $Log:	transcript.h,v $
 * Revision 2.1  85/11/24  11:51:21  shore
 * Product Release 2.0
 * 
 * Revision 1.3  85/11/20  00:58:05  shore
 * changes for Release 2 with System V support
 * 
 * Revision 1.2  85/05/14  11:31:52  shore
 * 
 * 
 *
 */

#define private static

#define TRUE	(1)
#define FALSE	(0)

#define PSVERSION 	23.0
#define COMMENTVERSION	"PS-Adobe-1.0"

#define VOID void
#define VOIDC (void)

#ifdef SYSV
#define INDEX strchr
#define RINDEX strrchr
#else
#define INDEX index
#define RINDEX rindex
#endif

/* external globals (from config.c) */

extern char LibDir[];
extern char TroffFontDir[];
extern char DitDir[];
extern char TempDir[];

/* definitions of basenames of various prologs, filters, etc */
/* all of these get concatenated as LibDir/filename */

#define BANNERPRO	"/banner.pro"
#define PLOTPRO		"/psplot.pro"
#define CATPRO		"/pscat.pro"
#define TEXTPRO		"/pstext.pro"
#define ENSCRIPTPRO	"/enscript.pro"
#define PS4014PRO	"/ps4014.pro"
#define PS630PRO	"/ps630.pro"
#define PSDITPRO	"/psdit.pro"
#define PSSUNPRO	"/pssun.pro"
#define FONTMAP		"/font.map"

#define ENSCRIPTTEMP	"/ESXXXXXX"
#define REVTEMP		"/RVXXXXXX"

#define PSTEXT		"pstext"
#define PSPLOT		"psplot"
#define PSCAT		"pscat"
#define PSDIT		"psdit"
#define PSIF		"psif"
#define PSSUN		"pssun"

/* psutil functions */
extern char *mstrcat();
extern char *envget();
extern VOID pexit();
extern VOID pexit2();

extern char *mapname();

/* system functions */
extern VOID perror();
extern VOID exit();
extern char *ctime();
extern long time();
extern char *mktemp();
extern char *gets();
extern char *malloc();
