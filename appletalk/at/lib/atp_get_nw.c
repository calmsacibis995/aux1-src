#ifndef lint	/* .../appletalk/at/lib/atp_get_nw.c */
#define _AC_NAME atp_get_nw_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:27}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_get_nw.c on 87/11/11 21:00:27";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

#ifndef NULL
#define	NULL	0
#endif

int
at_atp_get_request_nw(fd)
int fd;
{
	struct strioctl s;

	s.ic_cmd = AT_ATP_GET_REQUEST_NW;
	s.ic_timout = -1;
	s.ic_dp = NULL;
	s.ic_len = 0;
	return(ioctl(fd, I_STR, &s));
}
