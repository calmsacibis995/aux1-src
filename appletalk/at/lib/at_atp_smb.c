#ifndef lint	/* .../appletalk/at/lib/at_atp_smb.c */
#define _AC_NAME at_atp_smb_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:56}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_atp_smb.c on 87/11/11 20:57:56";
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

at_atpreq *at_send_request_mb(socket,to_net,to_node,to_socket,
                              treq_user_bytes,treq_data,
                              treq_data_length,tresp_count,xo,seconds,tries,
                              tresp_buff0_ptr,tresp_buff1_ptr,tresp_buff2_ptr,
                              tresp_buff3_ptr,tresp_buff4_ptr,tresp_buff5_ptr,
                              tresp_buff6_ptr,tresp_buff7_ptr)
        int     socket;
        int     to_net;
        int     to_node;
        int     to_socket;
        int     treq_user_bytes;
        char   *treq_data;
        int     treq_data_length;
        u8      tresp_count;
        int     xo;
        int     seconds;
        int     tries;
        char   *tresp_buff0_ptr;
        char   *tresp_buff1_ptr;
        char   *tresp_buff2_ptr;
        char   *tresp_buff3_ptr;
        char   *tresp_buff4_ptr;
        char   *tresp_buff5_ptr;
        char   *tresp_buff6_ptr;
        char   *tresp_buff7_ptr;
        {
        char   *fn;
        int     i;
        int     status;
        static  at_atpreq   treq_buff;
 
        fn = "at_send_request_mb";
        treq_buff.at_atpreq_type            = AT_ATP_CMD_TREQ;
        W16(treq_buff.at_atpreq_to_net,       to_net);
        treq_buff.at_atpreq_to_node         = to_node;
        treq_buff.at_atpreq_to_socket       = to_socket;
        (void) memcpy(treq_buff.at_atpreq_treq_user_bytes,
		      (char *) (&treq_user_bytes), 4);
        treq_buff.at_atpreq_treq_data       = (u8 *) treq_data;
        treq_buff.at_atpreq_treq_length     = treq_data_length;

        if (!(tresp_count >= 0 && tresp_count <= 8))
            {
            at_error(1350,fn,"response count out of range (%d)",tresp_count);
            return((at_atpreq *) NULL);
            }
        if (tresp_count != 0)
            {
            treq_buff.at_atpreq_treq_bitmap |= (1 << tresp_count) - 1;
            }
        treq_buff.at_atpreq_xo              = (u8) xo;
        treq_buff.at_atpreq_maximum_retries = tries;
        treq_buff.at_atpreq_retry_timeout   = seconds;
        treq_buff.at_atpreq_tresp_data[0]   = (u8 *) tresp_buff0_ptr;
        treq_buff.at_atpreq_tresp_data[1]   = (u8 *) tresp_buff1_ptr;
        treq_buff.at_atpreq_tresp_data[2]   = (u8 *) tresp_buff2_ptr;
        treq_buff.at_atpreq_tresp_data[3]   = (u8 *) tresp_buff3_ptr;
        treq_buff.at_atpreq_tresp_data[4]   = (u8 *) tresp_buff4_ptr;
        treq_buff.at_atpreq_tresp_data[5]   = (u8 *) tresp_buff5_ptr;
        treq_buff.at_atpreq_tresp_data[6]   = (u8 *) tresp_buff6_ptr;
        treq_buff.at_atpreq_tresp_data[7]   = (u8 *) tresp_buff7_ptr;

        status = at_send_request(socket,&treq_buff);
        if (status == -1)
            {
            at_error(1351,fn,"bad return from at_send_request()");
            return((at_atpreq *)NULL);
            }
        else
            return(&treq_buff);
        }
