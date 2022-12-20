#ifndef lint	/* .../appleprint/iw/iwprep/cmd_paplen.c */
#define _AC_NAME cmd_paplen_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:14}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_paplen_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_paplen.c on 87/11/11 21:42:14";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

cmd_paperlength (argc, argv)
    char            *argv[];
{
    int              paperlength;
    
    if (argc != 2) {
	error ("paperlength requires a numeric value");
	return 1;
    } else {
	paperlength = atoi (argv[1]);
	if (paperlength == 0) {
	    error ("paperlength must be greater than zero");
	    return 1;
	} else {
	    prep_desc.pd_paperlength = paperlength;
	    return 0;
	}
    }
}
