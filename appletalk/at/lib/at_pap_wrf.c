#ifndef lint	/* .../appletalk/at/lib/at_pap_wrf.c */
#define _AC_NAME at_pap_wrf_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:08}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_wrf.c on 87/11/11 21:00:08";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

at_pap_write_flush(fd, data, len)
int fd, len;
char *data;
{

	struct strioctl s;

	if (len < 0 || len > 512)
		return(-1);
	s.ic_dp = data;
	s.ic_len = len;
	s.ic_timout = -1;
	s.ic_cmd = AT_PAP_WRITE_FLUSH;
	return(ioctl(fd, I_STR, &s));
}
