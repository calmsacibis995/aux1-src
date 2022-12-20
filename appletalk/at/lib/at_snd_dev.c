#ifndef lint	/* .../appletalk/at/lib/at_snd_dev.c */
#define _AC_NAME at_snd_dev_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:21}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_snd_dev.c on 87/11/11 21:00:21";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/stropts.h>
#include "appletalk.h"


int at_send_to_dev(fd, cmd, dp, length)
int	fd;
int	cmd;
char	*dp;
int	*length;
{
	int		i;
	struct strioctl i_str;


	/* set up the iocblk for the I_STR call */
	i_str.ic_cmd = cmd;
	i_str.ic_timout = -1;
	if (length)
	    i_str.ic_len = *length;
	else
	    i_str.ic_len = 0;
	i_str.ic_dp = dp;

	if(ioctl(fd, I_STR, &i_str) < 0)
	    return(-1);

	if (length)
	    *length = i_str.ic_len;
	return(0);
}
