#ifndef lint	/* .../appleprint/iw/iwprep/error.c */
#define _AC_NAME error_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:31}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _error_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of error.c on 87/11/11 21:42:31";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include <stdio.h>

extern int      line_no;
extern int      errors;

/*VARARGS*/
error (fmt, a1, a2, a3, a4, a5)
    char        *fmt;
{
    errors++;
    fprintf (stderr, "line %d:", line_no);
    fprintf (stderr, fmt, a1, a2, a3, a4, a5);
    fprintf (stderr, "\n");
}
