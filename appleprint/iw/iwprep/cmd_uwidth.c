#ifndef lint	/* .../appleprint/iw/iwprep/cmd_uwidth.c */
#define _AC_NAME cmd_uwidth_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:22}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_uwidth_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_uwidth.c on 87/11/11 21:42:22";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

cmd_unitwidth (argc, argv)
    char            *argv[];
{
    int              unitwidth;
    
    if (argc != 2) {
	error ("unitwidth requires a numeric resolution");
	return 1;
    } else {
	unitwidth = atoi (argv[1]);
	if (unitwidth == 0) {
	    error ("unitwidth specification must be greater than zero");
	    return 1;
	} else {
	    prep_desc.pd_unitwidth = unitwidth;
	    return 0;
	}
    }
}
