#ifndef lint	/* .../appleprint/iw/iwprep/cmd_hor.c */
#define _AC_NAME cmd_hor_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:08}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_hor_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_hor.c on 87/11/11 21:42:08";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

cmd_hor (argc, argv)
    char            *argv[];
{
    int              hor;
    
    if (argc != 2) {
	error ("hor requires a numeric resolution");
	return 1;
    } else {
	hor = atoi (argv[1]);
	if (hor == 0) {
	    error ("hor resolution must be greater than zero");
	    return 1;
	} else {
	    prep_desc.pd_hor = hor;
	    return 0;
	}
    }
}
