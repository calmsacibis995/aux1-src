#ifndef lint	/* .../appleprint/iw/iwprep/cmd_papwid.c */
#define _AC_NAME cmd_papwid_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:17}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_papwid_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_papwid.c on 87/11/11 21:42:17";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

cmd_paperwidth (argc, argv)
    char            *argv[];
{
    int              paperwidth;
    
    if (argc != 2) {
	error ("paperwidth requires a numeric value");
	return 1;
    } else {
	paperwidth = atoi (argv[1]);
	if (paperwidth == 0) {
	    error ("paperwidth must be greater than zero");
	    return 1;
	} else {
	    prep_desc.pd_paperwidth = paperwidth;
	    return 0;
	}
    }
}
