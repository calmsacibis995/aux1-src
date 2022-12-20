#ifndef lint	/* .../appleprint/iw/iwprep/cmd_vert.c */
#define _AC_NAME cmd_vert_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:25}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_vert_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_vert.c on 87/11/11 21:42:25";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

cmd_vert (argc, argv)
    char            *argv[];
{   
    int              vert;
    
    if (argc != 2) {
	error ("vert requires a numeric resolution");
	return 1;
    } else {
	vert = atoi (argv[1]);
	if (vert == 0) {
	    error ("vert resolution must be greater than zero");
	    return 1;
	} else {
	    prep_desc.pd_vert = vert;
	    return 0;
	}
    }
}
