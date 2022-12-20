#ifndef lint	/* .../appletalk/at/lib/atp_send.c */
#define _AC_NAME atp_send_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:01:22}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_send.c on 87/11/11 21:01:22";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_send_response(fd, index, buff, len)
int fd;
unsigned int index;
char *buff;
int len;
{
	struct strioctl s;
	at_atp *atp;

	s.ic_cmd = AT_ATP_SEND_RESPONSE;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = len;
	atp = ATP_ATP_HDR(buff);
	atp->at_atp_bitmap_seqno = index;
	return(ioctl(fd, I_STR, &s));
}
