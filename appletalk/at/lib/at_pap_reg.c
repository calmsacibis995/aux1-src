#ifndef lint	/* .../appletalk/at/lib/at_pap_reg.c */
#define _AC_NAME at_pap_reg_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:48}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_reg.c on 87/11/11 20:59:48";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

int
at_papsl_register_nve(fd, object, object_len, type, type_len, trys, secs)
char *object;
char *type;
int trys, secs;
{
	struct stat st;

	if (fstat(fd, &st))
		return(-1);
	return(at_register_nve(object, 	object_len,
			       type, 	type_len,
			       st.st_rdev&0xff,
			       trys,	secs));
}

