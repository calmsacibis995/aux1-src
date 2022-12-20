#ifndef lint	/* .../appleprint/iw/daiw/strsave.c */
#define _AC_NAME strsave_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:45:02}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _strsave_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of strsave.c on 87/11/11 21:45:02";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

char *
strsave (string)
    char	*string;
{
    char	*buf;

    if ((buf = malloc (strlen (string) + 1)) == NULL)
	return NULL;
    strcpy (buf, string);
    return buf;
}
