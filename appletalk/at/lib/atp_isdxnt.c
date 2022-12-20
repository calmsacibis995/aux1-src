#ifndef lint	/* .../appletalk/at/lib/atp_isdxnt.c */
#define _AC_NAME atp_isdxnt_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:01:07}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_isdxnt.c on 87/11/11 21:01:07";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_issue_request_def_xo_note(fd, count, buff, len, rate, retries)
int fd;
int count;
char *buff;
int len;
int rate;
int retries;
{
	struct strioctl s;
	at_atp *atp;
	struct atp_set_default *def;

	s.ic_cmd = AT_ATP_ISSUE_REQUEST_DEF_NOTE;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = len;
	def = (struct atp_set_default *)buff;
	def->def_rate = rate;
	def->def_retries = retries;
	atp = ATP_ATP_HDR(&buff[sizeof(struct atp_set_default)]);
	atp->at_atp_xo = 1;
	atp->at_atp_bitmap_seqno = (1 << count) - 1;
	return(ioctl(fd, I_STR, &s));
}

