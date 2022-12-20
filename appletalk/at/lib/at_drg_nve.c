#ifndef lint	/* .../appletalk/at/lib/at_drg_nve.c */
#define _AC_NAME at_drg_nve_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:37}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_drg_nve.c on 87/11/11 20:58:37";
  static char *Version1 = "@(#)at_lib.c	5.2 10/13/86 (at_lib.c version)";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |	   #    #######         #         ###   ######           #####      |
  |	  # #      #            #          #    #     #         #     #     |
  |	 #   #     #            #          #    #     #         #           |
  |	#     #    #            #          #    ######          #           |
  |	#######    #            #          #    #     #   ###   #           |
  |	#     #    #            #          #    #     #   ###   #     #     |
  |	#     #    #    ####### #######   ###   ######    ###    #####      |
  |                                                                         |
  |                                                                         |
  |                                                                         |
  +-------------------------------------------------------------------------+
  |
  |  Description:
  |
  |  SCCS:
  |      @(#)at_lib.c	5.2 10/13/86
  |
  |  Copyright:
  |      Copyright 1986 by UniSoft Systems Corporation.
  |
  |  History:
  |      24-Jun-85: Created by Philip K. Ronzone.
  |
  +*/

#include "appletalk.h"
#include <fcntl.h>
#include "sys/types.h"
#include "sys/stropts.h"

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


int at_deregister_nve(nve_number)
        int     nve_number;
        {
        char   *fn;
        int     status;
	struct strioctl s;
	int	fd;

        fn = "at_deregister_nve";
	s.ic_len = sizeof(int);
	s.ic_dp = (char *)&nve_number;
	s.ic_timout = -1;
	s.ic_cmd = AT_NBP_DELETE;

	/*
	 *	set up the socket to make the call on
	 */

	fd = at_open_dynamic_socket(O_RDWR, NULL);
	if (fd < 0) {
                at_error(1850,fn,"socket open failed");
                return(-1);
        }
	if (ioctl(fd, I_PUSH, "at_nbp") < 0) {
                at_error(1851,fn,"socket push failed");
quit:
		at_close_dynamic_socket(fd);
		return(-1);
	}
	if (ioctl(fd, I_STR, &s) < 0) {
                at_error(1852,fn,"name deletion failed");
		goto quit;
	}
	at_close_dynamic_socket(fd);
        return(0);
        }
