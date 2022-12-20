#ifndef lint	/* .../appletalk/at/lib/at_atp_ssb.c */
#define _AC_NAME at_atp_ssb_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:03}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_atp_ssb.c on 87/11/11 20:58:03";
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
extern at_atpreq  *at_send_request_mb();

at_atpreq *at_send_request_sb(socket,to_net,to_node,to_socket,treq_user_bytes,
                              treq_data,treq_data_length,tresp_count,xo,seconds,
                              tries,tresp_buff_ptr,tresp_buff_length)
        int     socket;
        int     to_net;
        int     to_node;
        int     to_socket;
        int     treq_user_bytes;
        char   *treq_data;
        int     treq_data_length;
        int     tresp_count;
        int     xo;
        int     seconds;
        int     tries;
        char   *tresp_buff_ptr;
        int     tresp_buff_length;
        {
        auto    char        tresp_buff0[AT_ATP_DATA_SIZE];
        auto    char        tresp_buff1[AT_ATP_DATA_SIZE];
        auto    char        tresp_buff2[AT_ATP_DATA_SIZE];
        auto    char        tresp_buff3[AT_ATP_DATA_SIZE];
        auto    char        tresp_buff4[AT_ATP_DATA_SIZE];
        auto    char        tresp_buff5[AT_ATP_DATA_SIZE];
        auto    char        tresp_buff6[AT_ATP_DATA_SIZE];
        auto    char        tresp_buff7[AT_ATP_DATA_SIZE];
        static  at_atpreq  *treq_buff;
        char   *fn;
        int     loop;
        int     tot_tresp_length;
        char   *tresp_ptr;
 
        fn = "at_send_request_sb";
        treq_buff = at_send_request_mb(socket,to_net,to_node,to_socket,
                                treq_user_bytes,treq_data,
                                treq_data_length,tresp_count,xo,seconds,tries,
                                tresp_buff0,tresp_buff1,tresp_buff2,
                                tresp_buff3,tresp_buff4,tresp_buff5,
                                tresp_buff6,tresp_buff7);
        if (treq_buff == (at_atpreq *)NULL)
            {
            at_error(1400,fn,"bad return from at_send_request_mb()");
            return((at_atpreq *)NULL);
            }

        tot_tresp_length = 0;
        for (loop=0;loop<AT_ATP_TRESP_MAX;loop++)
            {
            tot_tresp_length += (*treq_buff).at_atpreq_tresp_lengths[loop];
            }
        if (tot_tresp_length > tresp_buff_length)
            {
            at_error(1401,fn,"response buffer to small");
            return((at_atpreq *)NULL);
            }

        tresp_ptr = (char *) tresp_buff_ptr;
        for (loop=0;loop<AT_ATP_TRESP_MAX;loop++)
            {
            (void) memcpy(tresp_ptr, 
			  (char *) (*treq_buff).at_atpreq_tresp_data[loop],
                          (*treq_buff).at_atpreq_tresp_lengths[loop]);
            tresp_ptr += (*treq_buff).at_atpreq_tresp_lengths[loop];
            }
        return(treq_buff);
        }
