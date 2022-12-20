#ifndef lint	/* .../appletalk/at/lib/at_atp_rsb.c */
#define _AC_NAME at_atp_rsb_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:52}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_atp_rsb.c on 87/11/11 20:57:52";
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

int at_send_response_sb (socket,to_net,to_node,to_socket,tid,eom,
                         user_bytes,user_bytes_len,buffer,buffer_len)
        int     to_net;
        int     to_node;
        int     to_socket;
        int     tid;
        int     eom;
        int    *user_bytes;
        int     user_bytes_len;
        char   *buffer;
        int     buffer_len;
        {
        char   *fn;
        auto    char       tresp_buff[AT_ATP_TRESP_MAX][AT_ATP_DATA_SIZE];
        auto    int        tresp_lengths[AT_ATP_TRESP_MAX];
        auto    char       tresp_user_bytes[AT_ATP_TRESP_MAX][4];
        int     copy_len;
        int     status;
        int     loop;
        char   *gen_ptr;

        fn = "at_send_response_sb";
        if (!(buffer_len >= 0 && buffer_len <= eom*AT_ATP_DATA_SIZE))
            {
            at_error(1250,fn,"buffer length out of range for given eom (%d)",
                     buffer_len);
            return(-1);
            }
        if (!(user_bytes_len >= 0 && user_bytes_len <= eom*4))
            {
            at_error(1251,fn,"user bytes length out of range for given eom (%d)",
                     user_bytes_len);
            return(-1);
            }

        copy_len = AT_ATP_DATA_SIZE;
        gen_ptr  = buffer;
        for (loop=0; loop<eom && buffer_len>0; loop++)
            {
            if (buffer_len < AT_ATP_DATA_SIZE)
                {
                copy_len = buffer_len;
                }
            (void) memcpy(tresp_buff[loop],gen_ptr,copy_len);
            tresp_lengths[loop] = copy_len;
            gen_ptr            += copy_len;
            buffer_len         -= copy_len;
            }
        if (loop != eom)
            {
            at_error(1252,fn,"not enough data specified");
            return(-1);
            }

        copy_len = 4;
        gen_ptr  = (char *) user_bytes;
        for (loop=0; loop<eom && user_bytes_len>0; loop++)
            {
            if (user_bytes_len < 4)
                {
                copy_len = user_bytes_len;
                }
            (void) memcpy(tresp_user_bytes[loop],gen_ptr,copy_len);
            gen_ptr        += copy_len;
            user_bytes_len -= copy_len;
            }
        if (loop != eom)
            {
            at_error(1253,fn,"not enough user bytes specified");
            return(-1);
            }

        status = at_send_response_mb (socket,to_net,to_node,to_socket,
                         tid,eom,(1 << eom) - 1,
                         tresp_user_bytes[0],tresp_buff[0],tresp_lengths[0],
                         tresp_user_bytes[1],tresp_buff[1],tresp_lengths[1],
                         tresp_user_bytes[2],tresp_buff[2],tresp_lengths[2],
                         tresp_user_bytes[3],tresp_buff[3],tresp_lengths[3],
                         tresp_user_bytes[4],tresp_buff[4],tresp_lengths[4],
                         tresp_user_bytes[5],tresp_buff[5],tresp_lengths[5],
                         tresp_user_bytes[6],tresp_buff[6],tresp_lengths[6],
                         tresp_user_bytes[7],tresp_buff[7],tresp_lengths[7]);
        if (status == -1)
            {
            at_error(1254,fn,"bad return from at_send_response_mb()");
            return(-1);
            }
        else
            {
            return(0); 
            }
        }
