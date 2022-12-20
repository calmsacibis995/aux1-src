#ifndef lint	/* .../appletalk/at/lib/atp_is.c */
#define _AC_NAME atp_is_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:36}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_is.c on 87/11/11 21:00:36";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_issue_request(fd, count, buff, len)
int fd;
int count;
char *buff;
int len;
{
	struct strioctl s;
	at_atp *atp;

	s.ic_cmd = AT_ATP_ISSUE_REQUEST;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = len;
	atp = ATP_ATP_HDR(buff);
	atp->at_atp_xo = 0;
	atp->at_atp_bitmap_seqno = (1 << count) - 1;
	return(ioctl(fd, I_STR, &s));
}

