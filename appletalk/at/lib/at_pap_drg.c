#ifndef lint	/* .../appletalk/at/lib/at_pap_drg.c */
#define _AC_NAME at_pap_drg_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:29}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_drg.c on 87/11/11 20:59:29";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

int
at_papsl_deregister_nve(object, object_len, type, type_len)
char *object;
char *type;
{
	return(at_deregister_name_nve(object, 	object_len,
			         type, 		type_len));
}

