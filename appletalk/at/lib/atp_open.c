#ifndef lint	/* .../appletalk/at/lib/atp_open.c */
#define _AC_NAME atp_open_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:01:13}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_open.c on 87/11/11 21:01:13";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include <fcntl.h>
#include "appletalk.h"
#ifndef NULL
#define NULL	0
#endif NULL

int
at_atp_open()
{
	int fd;

	fd = at_open_dynamic_socket(O_RDWR, NULL);
	if (fd < 0)
		return(-1);
	if (ioctl(fd, I_PUSH, "at_atp")) {
		close(fd);
		return(-1);
	}
	return(fd);
}
