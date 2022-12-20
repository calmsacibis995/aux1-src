#ifndef lint	/* .../appletalk/at/lib/at_op_skt.c */
#define _AC_NAME at_op_skt_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:23}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_op_skt.c on 87/11/11 20:59:23";
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

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


int at_open_socket(open_mode,socket)
        int    open_mode;
        int    socket;
        {
        char  *fn;
        char   buffer[64];
        int    socket_file;
	int    status;

        fn = "at_open_socket";
	if (!(*at_network))
            {
	    status = at_find_dflt_netw();
            if (status == -1)
                {
                at_error(2550,fn,"can't find default network");
                }
	    }
        (void) sprintf(buffer,"%s/socket%d",at_network,socket);
        socket_file = open(buffer,open_mode);
        if (socket_file == -1)
            {
            at_error(2551,fn,"can't open socket 0x%x (<%s>)",socket,buffer);
            }
        return(socket_file);
        }
