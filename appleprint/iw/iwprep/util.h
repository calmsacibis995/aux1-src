#ifndef lint	/* .../appleprint/iw/iwprep/util.h */
#define _AC_NAME util_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:50}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _util_h[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of util.h on 87/11/11 21:42:50";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include <stdio.h>

extern char     *malloc();
extern char     *strsave();

#define NEW(type)       (type *) malloc (sizeof (type))
#define NEWN(type, n)   (type *) malloc (n * sizeof (type))
