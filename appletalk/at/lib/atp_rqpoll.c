#ifndef lint	/* .../appletalk/at/lib/atp_rqpoll.c */
#define _AC_NAME atp_rqpoll_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:01:19}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_rqpoll.c on 87/11/11 21:01:19";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_poll_request(fd, id, buff)
int fd;
int id;
char *buff;
{
	struct strioctl s;
	at_atp *atp;

	s.ic_cmd = AT_ATP_POLL_REQUEST;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = sizeof(int);
	*(int *)buff = id;
	return(ioctl(fd, I_STR, &s));
}
