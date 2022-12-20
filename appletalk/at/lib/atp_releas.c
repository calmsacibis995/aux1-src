#ifndef lint	/* .../appletalk/at/lib/atp_releas.c */
#define _AC_NAME atp_releas_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:01:16}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_releas.c on 87/11/11 21:01:16";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_release_response(fd, buff)
int fd;
char *buff;
{
	struct strioctl s;

	s.ic_cmd = AT_ATP_RELEASE_RESPONSE;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = ATP_HDR_SIZE;
	return(ioctl(fd, I_STR, &s));
}
