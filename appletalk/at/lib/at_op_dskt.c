#ifndef lint	/* .../appletalk/at/lib/at_op_dskt.c */
#define _AC_NAME at_op_dskt_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:20}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_op_dskt.c on 87/11/11 20:59:20";
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
#include <sys/stat.h>

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


int at_open_dynamic_socket(open_mode,socket_number_ptr)
        int         open_mode;
        int        *socket_number_ptr;
        {
        char       *fn;
        int         socket_number;
        int         socket_file;
	int	    status;
	char        socket_name[MAXNAMLEN];
	struct stat st;

        fn = "at_open_dynamic_socket()";
	if (!(*at_network))
	    {
	    status = at_find_dflt_netw();
            if (status == -1)
                {
                at_error(2500,fn,"can't find default network");
                return(-1);
                }
	    }

	sprintf(socket_name,"%s/socket",at_network);
        socket_file = open(socket_name, open_mode);
        if (socket_file == -1)
            {
            at_error(2501,fn,"can't open dynamic socket");
            return(-1);
            }
        if (socket_number_ptr != NULL)
            {
	    if (fstat(socket_file, &st) < 0)
            	{
            	at_error(2502,fn,"can't get socket's number");
            	return(-1);
            	}
            *socket_number_ptr = st.st_rdev&0xff;;
            }
        return(socket_file);
        }
