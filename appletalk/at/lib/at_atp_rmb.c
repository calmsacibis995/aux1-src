#ifndef lint	/* .../appletalk/at/lib/at_atp_rmb.c */
#define _AC_NAME at_atp_rmb_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:45}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_atp_rmb.c on 87/11/11 20:57:45";
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

int at_send_response_mb(socket,to_net,to_node,to_socket,tid,eom_seqno,bitmap,
                        user_bytes0,buffer0,buffer0_len,
                        user_bytes1,buffer1,buffer1_len,
                        user_bytes2,buffer2,buffer2_len,
                        user_bytes3,buffer3,buffer3_len,
                        user_bytes4,buffer4,buffer4_len,
                        user_bytes5,buffer5,buffer5_len,
                        user_bytes6,buffer6,buffer6_len,
                        user_bytes7,buffer7,buffer7_len)
        int     socket;
        int     to_net;
        int     to_node;
        int     to_socket;
        int     tid;
        int     eom_seqno;
        u8      bitmap;
        int     user_bytes0;
        char   *buffer0;
        int     buffer0_len;
        int     user_bytes1;
        char   *buffer1;
        int     buffer1_len;
        int     user_bytes2;
        char   *buffer2;
        int     buffer2_len;
        int     user_bytes3;
        char   *buffer3;
        int     buffer3_len;
        int     user_bytes4;
        char   *buffer4;
        int     buffer4_len;
        int     user_bytes5;
        char   *buffer5;
        int     buffer5_len;
        int     user_bytes6;
        char   *buffer6;
        int     buffer6_len;
        int     user_bytes7;
        char   *buffer7;
        int     buffer7_len;
        {
        char      *fn;
        int        status;
        int        i;
        at_atpreq  tresp_buff;
        char      *arg_ptr;
        char     **arg_str_ptr;
        int       *arg_int_ptr;

        fn = "at_send_response_mb";
        tresp_buff.at_atpreq_type            = AT_ATP_CMD_TRESP;
        W16(tresp_buff.at_atpreq_to_net,       to_net);
        tresp_buff.at_atpreq_to_node         = to_node;
        tresp_buff.at_atpreq_to_socket       = to_socket;
        tresp_buff.at_atpreq_tid             = tid;
        tresp_buff.at_atpreq_tresp_eom_seqno = eom_seqno;
        tresp_buff.at_atpreq_tresp_bitmap    = bitmap;

        for(i=0,arg_ptr=(char *) &user_bytes0; i<AT_ATP_TRESP_MAX; 
            i++,arg_ptr+=12)
            {
            (void) memcpy(tresp_buff.at_atpreq_tresp_user_bytes[i],arg_ptr,4);
            }
        for(i=0,arg_str_ptr=(char **) &buffer0; i<AT_ATP_TRESP_MAX; 
            i++,arg_str_ptr+=3)
            {
            tresp_buff.at_atpreq_tresp_data[i] = (u8 *) *arg_str_ptr;
            }
        for(i=0,arg_int_ptr=&buffer0_len; i<AT_ATP_TRESP_MAX; i++,arg_int_ptr+=3)
            {
            tresp_buff.at_atpreq_tresp_lengths[i] = *arg_int_ptr;
            }

        status = at_send_response(socket,&tresp_buff);
        if (status == -1)
            {
            at_error(1200,fn,"bad return from at_send_response()");
            return(-1);
            }
        else
            {
            return(0);
            }
        }
