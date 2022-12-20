#ifndef lint	/* .../appleprint/iw/iwprep/cmd_res.c */
#define _AC_NAME cmd_res_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:20}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_res_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_res.c on 87/11/11 21:42:20";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

cmd_res (argc, argv)
    char            *argv[];
{
    int              res;
    
    if (argc != 2) {
	error ("res keyword requires a numeric resolution");
	return 1;
    } else {
	res = atoi (argv[1]);
	if (res == 0) {
	    error ("res keyword requires a numeric resolution");
	    return 1;
	} else {
	    prep_desc.pd_res = res;
	    return 0;
	}
    }
}
