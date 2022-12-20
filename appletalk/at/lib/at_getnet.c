#ifndef lint	/* .../appletalk/at/lib/at_getnet.c */
#define _AC_NAME at_getnet_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:03}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_getnet.c on 87/11/11 20:59:03";
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
#include <sys/types.h>
#include <sys/stropts.h>

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();



static at_cfg_valid = 0;

static int at_get_cfg()
{
	char *fn = "at_get_cfg";
	int fd;
	struct strioctl s;
	at_ddp_cfg_t cfg;

	if ((fd = at_open_dynamic_socket(O_RDWR, NULL)) < 0) {
                at_error(2250,fn,"can't open dynamic socket");
		return(-1);
	}
	s.ic_timout = -1;
	s.ic_cmd = AT_DDP_GET_CFG;
	s.ic_dp = (char *)&cfg;
	s.ic_len = sizeof(cfg);
	if (ioctl(fd, I_STR, &s) < 0) {
		at_close_dynamic_socket(fd);
                at_error(2251,fn,"can't ioctl AT_GET_CONFIGURATION");
		return(-1);
	}
	at_close_dynamic_socket(fd);
	at_net_number = cfg.this_net;
	at_node_number = cfg.this_node;
	at_cfg_valid = 1;
	return(0);
}

int at_get_net_number()
        {
	if (!at_cfg_valid && at_get_cfg())
		return(-1);
        return(at_net_number);
        }

int at_get_node_number()
        {
	if (!at_cfg_valid && at_get_cfg())
		return(-1);
        return(at_node_number);
        }
