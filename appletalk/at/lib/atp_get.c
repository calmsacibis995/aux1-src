#ifndef lint	/* .../appletalk/at/lib/atp_get.c */
#define _AC_NAME atp_get_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_get.c on 87/11/11 21:00:24";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_get_request(fd, buff)
int fd;
char *buff;
{
	struct strioctl s;

	s.ic_cmd = AT_ATP_GET_REQUEST;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = 0;
	if (ioctl(fd, I_STR, &s) < 0)
		return(-1);
	return(s.ic_len);
}
