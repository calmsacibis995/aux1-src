#ifndef lint	/* .../appletalk/at/lib/at_gddplen.c */
#define _AC_NAME at_gddplen_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:00}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_gddplen.c on 87/11/11 20:59:00";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"

int at_get_ddp_length(ptr)
	u8 *ptr;
	{
	int length;

	length = (*ptr++ & 3) << 8;
	length += *ptr;
	return(length);
	}
