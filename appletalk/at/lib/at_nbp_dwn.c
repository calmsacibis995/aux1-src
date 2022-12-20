#ifndef lint	/* .../appletalk/at/lib/at_nbp_dwn.c */
#define _AC_NAME at_nbp_dwn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:17}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_nbp_dwn.c on 87/11/11 20:59:17";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include <errno.h>
#include "appletalk.h"
#ifndef NULL
#define NULL	0
#endif NULL

at_nbp_shutdown()
{
	int fd;
	struct strioctl s;
	int i;

	fd = at_open_dynamic_socket(O_RDWR, NULL);
	if (fd < 0)
		return(-1);
	i = ioctl(fd, I_PUSH, "at_nbp");
	if (i < 0) { 
quit:
		at_close_dynamic_socket(fd);
		return(-1);
	}
	s.ic_len = 0;
	s.ic_dp = NULL;
	s.ic_timout = -1;
	s.ic_cmd = AT_NBP_SHUTDOWN;
	if (ioctl(fd, I_STR, &s) < 0)
		goto quit;
	at_close_dynamic_socket(fd);
	return(0);
}
