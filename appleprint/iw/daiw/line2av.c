#ifndef lint	/* .../appleprint/iw/daiw/line2av.c */
#define _AC_NAME line2av_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:39}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _line2av_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of line2av.c on 87/11/11 21:44:39";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

line2av (buf, argv)
    char                *buf;
    char                *argv[];
{
    register char       *p;
    register char       **av;
    int                  quote;
    
    p = buf;
    av = argv;
    while (*p) {
	while (*p == ' ' || *p == '\t')
	    p++;
	if (*p == '"' || *p == '\'') {
	    quote = *p++;
	    *av++ = p;
	    do
		p++;
	    while (*p && *p != quote);
	} else if (*p != '\0') {
	    *av++ = p;
	    while (*p && *p != ' ' && *p != '\t')
		p++;
	}
	if (*p == '\0')
	    return av - argv;
	*p++ = '\0';
    }
    return av - argv;
}
