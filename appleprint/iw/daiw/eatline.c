#ifndef lint	/* .../appleprint/iw/daiw/eatline.c */
#define _AC_NAME eatline_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:13}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _eatline_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of eatline.c on 87/11/11 21:44:13";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

void
eatline (ifile, buffer)
    register FILE  	*ifile;
    register char  	*buffer;
{
    register int	c;

    while ((c = eatc(ifile)) != EOF) {
	if (c == '\n')
	    break;
	*buffer++ = c;
    }
    *buffer = '\0';
}
