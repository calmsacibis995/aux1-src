#ifndef lint	/* .../appletalk/at/lib/at_pap_rd.c */
#define _AC_NAME at_pap_rd_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:46}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_rd.c on 87/11/11 20:59:46";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

at_pap_read(fd, data, len)
int fd, len;
char *data;
{
	struct strioctl s;

	if (len <= 0)
		return(0);
	s.ic_dp = data;
	s.ic_len = len;
	s.ic_timout = -1;
	s.ic_cmd = AT_PAP_READ;
	if(ioctl(fd, I_STR, &s))
		return(-1);
	return(s.ic_len);
}

