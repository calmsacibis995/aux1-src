#ifndef lint	/* .../appletalk/at/lib/at_pap_rig.c */
#define _AC_NAME at_pap_rig_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:51}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_rig.c on 87/11/11 20:59:51";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

at_pap_read_ignore(fd)
int fd;
{
	struct strioctl s;

	s.ic_len = 0;
	s.ic_timout = -1;
	s.ic_cmd = AT_PAP_READ_IGNORE;
	if(ioctl(fd, I_STR, &s))
		return(-1);
	return(0);
}

