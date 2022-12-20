#ifndef lint	/* .../appleprint/iw/iwprep/cmd_name.c */
#define _AC_NAME cmd_name_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:11}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_name_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_name.c on 87/11/11 21:42:11";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

cmd_name (argc, argv)
    char            *argv[];
{
    if (argc != 2) {
	error ("name keyword requires a device name");
	return 1;
    } else {
	prep_desc.pd_device = strsave (argv[1]);
	return 0;
    }
}
