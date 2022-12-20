#ifndef lint	/* .../appletalk/at/lib/at_pap_scl.c */
#define _AC_NAME at_pap_scl_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:54}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_scl.c on 87/11/11 20:59:54";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

int
at_papsl_close(fd)
int fd;
{
	close(fd);
}

