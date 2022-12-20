#ifndef lint	/* .../appletalk/at/lib/at_pap_int.c */
#define _AC_NAME at_pap_int_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:38}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_int.c on 87/11/11 20:59:38";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

int
at_papsl_init_nve(object, object_len, type, type_len, trys, secs, status)
char *object;
char *type;
int trys, secs;
char *status;
{
	int i, fd;
	struct strioctl s;
	char buff[256];

	fd = at_open_dynamic_socket(O_RDWR, NULL);
	if (fd < 0) {
		return(-1);
	}
	if (ioctl(fd, I_PUSH, "at_atp") < 0) {
		at_close_dynamic_socket(fd);
		return(-1);
	}
	if (ioctl(fd, I_PUSH, "at_papd") < 0) {
		at_close_dynamic_socket(fd);
		return(-1);
	}
	s.ic_dp = buff;
	i = strlen(status);
	if (i > 255)
		i = 255;
	strncpy(&buff[1], status, i+1);
	s.ic_len = i + 1;
	buff[0] = i;
	s.ic_timout = -1;
	s.ic_cmd = AT_PAPD_SET_STATUS;
	if (ioctl(fd, I_STR, &s) < 0) {
		at_close_dynamic_socket(fd);
		return(-1);
	}
	if (at_papsl_register_nve(fd, object, object_len, type, type_len, trys, secs)<0){
		at_close_dynamic_socket(fd);
		return(-1);
	}
	return(fd);
}

