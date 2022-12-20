#ifndef lint	/* .../appletalk/at/lib/atp_st_def.c */
#define _AC_NAME atp_st_def_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:01:28}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_st_def.c on 87/11/11 21:01:28";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"

int
at_atp_set_default(fd, rate, retries)
int fd;
int rate, retries;
{
	struct strioctl s;
	struct atp_set_default def;

	def.def_retries = retries;
	def.def_rate = rate;
	s.ic_cmd = AT_ATP_SET_DEFAULT;
	s.ic_timout = -1;
	s.ic_dp = (char *)&def;
	s.ic_len = sizeof(def);
	return(ioctl(fd, I_STR, &s));
}
