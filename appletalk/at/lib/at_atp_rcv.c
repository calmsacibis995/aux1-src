#ifndef lint	/* .../appletalk/at/lib/at_atp_rcv.c */
#define _AC_NAME at_atp_rcv_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:42}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_atp_rcv.c on 87/11/11 20:57:42";
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

int at_receive_request(socket,from_net_ptr,from_node_ptr,from_socket_ptr,
                       treq_user_bytes,treq_data,treq_data_length,xo_ptr,
                       bitmap_ptr,tid_ptr)
        int     socket;
        int    *from_net_ptr;
        int    *from_node_ptr;
        int    *from_socket_ptr;
        int    *treq_user_bytes;
        char   *treq_data;
        int    *treq_data_length;
        int    *xo_ptr;
        u8     *bitmap_ptr;
        int    *tid_ptr;
        {
        char    *fn;
        int      len;
        char   	 buff[620];
	at_atp 	 *atp;
	at_ddp_t *ddp;

        fn = "at_receive_request";
        at_errno = 0;
	len = at_atp_get_request(socket, buff);
        if (len == -1)
            {
            at_error(1100,fn,"bad return from at_atp_get_request()");
            return(-1);
            }

	ddp = ATP_DDP_HDR(buff);
	*from_net_ptr = R16(ddp->src_net);
	*from_node_ptr = ddp->src_node;
	*from_socket_ptr = ddp->src_socket;
	atp = ATP_ATP_HDR(buff);
        *xo_ptr      = (int) atp->at_atp_xo;
        *bitmap_ptr  = atp->at_atp_bitmap_seqno;
        *tid_ptr     = (atp->at_atp_transaction_id[0] << 8) |
                       (atp->at_atp_transaction_id[1] & 0xff);
        (void) memcpy((char *) treq_user_bytes,
                      (char *) atp->at_atp_user_bytes, 
                      sizeof(atp->at_atp_user_bytes));
        *treq_data_length = len - ATP_HDR_SIZE;
        (void) memcpy(treq_data,(char *) atp->at_atp_data,
                      *treq_data_length);
        return(0);
        }
