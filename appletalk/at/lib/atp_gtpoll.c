#ifndef lint	/* .../appletalk/at/lib/atp_gtpoll.c */
#define _AC_NAME atp_gtpoll_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:33}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_gtpoll.c on 87/11/11 21:00:33";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_get_poll(fd, buff)
int fd;
char *buff;
{
	struct strioctl s;

	s.ic_cmd = AT_ATP_GET_POLL;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = 0;
	return(ioctl(fd, I_STR, &s));
}
