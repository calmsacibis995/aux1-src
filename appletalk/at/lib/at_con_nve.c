#ifndef lint	/* .../appletalk/at/lib/at_con_nve.c */
#define _AC_NAME at_con_nve_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:17}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_con_nve.c on 87/11/11 20:58:17";
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


int at_confirm_nve(object,olen,type,tlen,zone,zlen,
                   trys,seconds,net,node,socket)
        char    *object;
        int      olen;
        char    *type;
        int      tlen;
        char    *zone;
        int      zlen;
        int      trys;
        int      seconds;
        int      net;
        int      node;
        int      socket;
        {
        char   *fn;
        int     status;
        at_nve *nveptr;

        fn = "at_confirm_nve";

        if (!(0 <= net && net <= 65535))
            {
            at_error(1600,fn,"net number out of range (%d)",net);
            return (-1);
            }

        if (!(0 < node && node <= 255))
            {
            at_error(1601,fn,"node number out of range (%d)",node);
            return (-1);
            }

        if (!(1 <= socket && socket <= 254))
            {
            at_error(1602,fn,"socket number out of range (%d)",socket);
            return (-1);
            }

        status = at_lookup_or_confirm_nve(object,olen,type,tlen,zone,zlen,
                                          trys,seconds,net,node,
                                          AT_NBP_CONFIRM);
        if (status == -1)
            {
            at_error(1603,fn,"bad return from at_lookup_or_confirm_nve()");
            return(-1);
            }

        /*--------------------------------------------------*/
        /* If there was one (and only one) confirmation ... */
        /*--------------------------------------------------*/
        if (at_nve_lkup_reply_count == 1)
            {
            nveptr = at_nve_lkup_reply_head;

            /*-----------------------------------------------*/
            /* If the net, node, and socket are the same ... */
            /*-----------------------------------------------*/
            if (nveptr->at_nve_net != net)
                {
                return(2);
                }
            else if (nveptr->at_nve_node != node)
                {
                return(3);
                }
            else if (nveptr->at_nve_socket != socket)
                {
                return(4);
                }
            else
                {
                return(1);
                }
            }

        /*----------------------------------*/
        /* If there was no confirmation ... */
        /*----------------------------------*/
        else if (at_nve_lkup_reply_count == 0)
            {
            return(0);
            }

        /*------------------------------------------------*/
        /* Otherwise, multiple (erroneous) confirmations. */
        /*------------------------------------------------*/
        else
            {
            return(-1);
            }
        }
