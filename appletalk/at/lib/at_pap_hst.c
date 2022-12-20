#ifndef lint	/* .../appletalk/at/lib/at_pap_hst.c */
#define _AC_NAME at_pap_hst_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:36}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_hst.c on 87/11/11 20:59:36";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

int
at_papsl_heres_status(fd, status)
int fd;
char *status;
{
	struct strioctl s;
	char buff[256];
	int i;

	s.ic_dp = buff;
	i = strlen(status);
	if (i > 255)
		i = 255;
	strncpy(&buff[1], status, i+1);
	s.ic_len = i + 1;
	buff[0] = i;
	s.ic_timout = -1;
	s.ic_cmd = AT_PAPD_SET_STATUS;
	return(ioctl(fd, I_STR, &s));
}

