#ifndef lint	/* .../appletalk/at/lib/at_chooser.c */
#define _AC_NAME at_chooser_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:06}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_chooser.c on 87/11/11 20:58:06";
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

#define FILE 

#include "appletalk.h"

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


int at_chooser(object,type,zone,trys,seconds)
        char   *object;
        char   *type;
        char   *zone;
        int     trys;
        int     seconds;
        {
        char   *fn;
        int     count;
        int     item;
        int     item_idx;
        int     maxlen_object;
        int     maxlen_type;
        int     maxlen_zone;
        int     maxlen_net;
        int     maxlen_node;
        int     maxlen_socket;
        char    buffer[128];
        at_nve *nve;
        at_nve *nve_head;

        fn = "at_chooser";
        count = at_lookup_nve(object,0,type,0,zone,0,trys,seconds);
        if (count == -1)
            {
            at_error(1450,fn,"bad return from at_lookup_nve()");
            return(-1);
            }
        if (count == 0)
            {
            return(0);
            }
        maxlen_object = strlen(at_chooser_print_object_header);
        maxlen_type   = strlen(at_chooser_print_type_header);
        maxlen_zone   = strlen(at_chooser_print_zone_header);
        maxlen_net    = strlen(at_chooser_print_net_header);
        maxlen_node   = strlen(at_chooser_print_node_header);
        maxlen_socket = strlen(at_chooser_print_socket_header);
        nve_head = at_nve_lkup_reply_head;
        for (nve = nve_head; nve != (at_nve *)NULL; nve = nve->at_nve_next)
            {
            if (nve->at_nve_object_length > maxlen_object)
                {
                maxlen_object = nve->at_nve_object_length;
                }
            if (nve->at_nve_type_length > maxlen_type)
                {
                maxlen_type = nve->at_nve_type_length;
                }
            if (nve->at_nve_zone_length > maxlen_zone)
                {
                maxlen_zone = nve->at_nve_zone_length;
                }
            (void) sprintf(buffer,at_chooser_print_net_fmt,nve->at_nve_net);
            if (strlen(buffer) > maxlen_net)
                {
                maxlen_net = strlen(buffer);
                }
            (void) sprintf(buffer,at_chooser_print_node_fmt,nve->at_nve_node);
            if (strlen(buffer) > maxlen_node)
                {
                maxlen_node = strlen(buffer);
                }
            (void) sprintf(buffer,at_chooser_print_socket_fmt,nve->at_nve_socket);
            if (strlen(buffer) > maxlen_socket)
                {
                maxlen_socket = strlen(buffer);
                }
            }
        while (1)
            {
            (void) fprintf(at_chooser_stdout,"\nITEM ");
            if (at_chooser_print_object)
                {
                (void) fprintf(at_chooser_stdout,
                        "%-*s ",maxlen_object,at_chooser_print_object_header);
                }
            if (at_chooser_print_type)
                {
                (void) fprintf(at_chooser_stdout,
                        "%-*s ",maxlen_type,at_chooser_print_type_header);
                }
            if (at_chooser_print_zone)
                {
                (void) fprintf(at_chooser_stdout,
                        "%-*s ",maxlen_zone,at_chooser_print_zone_header);
                }
            if (at_chooser_print_net)
                {
                (void) fprintf(at_chooser_stdout,
                        "%-*s ",maxlen_net,at_chooser_print_net_header);
                }
            if (at_chooser_print_node)
                {
                (void) fprintf(at_chooser_stdout,
                        "%-*s ",maxlen_node,at_chooser_print_node_header);
                }
            if (at_chooser_print_socket)
                {
                (void) fprintf(at_chooser_stdout,
                        "%-*s ",maxlen_socket,at_chooser_print_socket_header);
                }
            (void) fprintf(at_chooser_stdout,"\n");
            item = 0;
            for (nve = nve_head; nve != (at_nve *)NULL; nve = nve->at_nve_next)
                {
                (void) fprintf(at_chooser_stdout,
                        "%3d: ",item++ + 1);
                if (at_chooser_print_object)
                    {
                    (void) fprintf(at_chooser_stdout,
                            "%-*s ",maxlen_object,nve->at_nve_object);
                    }
                if (at_chooser_print_type)
                    {
                    (void) fprintf(at_chooser_stdout,
                            "%-*s ",maxlen_type,nve->at_nve_type);
                    }
                if (at_chooser_print_zone)
                    {
                    (void) fprintf(at_chooser_stdout,
                            "%-*s ",maxlen_zone,nve->at_nve_zone);
                    }
                if (at_chooser_print_net)
                    {
                    (void) sprintf(buffer,at_chooser_print_net_fmt,nve->at_nve_net);
                    (void) fprintf(at_chooser_stdout,"%-*s ",maxlen_net,buffer);
                    }
                if (at_chooser_print_node)
                    {
                    (void) sprintf(buffer,at_chooser_print_node_fmt,nve->at_nve_node);
                    (void) fprintf(at_chooser_stdout,"%-*s ",maxlen_node,buffer);
                    }
                if (at_chooser_print_socket)
                    {
                    (void) sprintf(buffer,at_chooser_print_socket_fmt,nve->at_nve_socket);
                    (void) fprintf(at_chooser_stdout,"%-*s ",maxlen_socket,buffer);
                    }
                (void) fprintf(at_chooser_stdout,"\n");
                }
            (void) fprintf(at_chooser_stdout,
                    at_chooser_msg,at_chooser_msg_arg1,at_chooser_msg_arg2);
            (void) fflush(at_chooser_stdout);
            if (fgets(buffer,sizeof(buffer),at_chooser_stdin) != buffer)
                {
                at_error(1451,fn,"bad return or possible EOF from fgets()");
                return(-1);
                }
            if (buffer[0] == '\0')
                {
                continue;
                }
            item = atoi(buffer);
            if (item == 0)
                {
                return(0);
                }
            if (!(1 <= item  &&  item <= count))
                {
                continue;
                }
            item_idx = 1;
            for (nve = nve_head; nve != (at_nve *)NULL; nve = nve->at_nve_next)
                {
                if (item_idx++ == item)
                    {
                    (void)memcpy(at_chooser_object,nve->at_nve_object,
                                 AT_NBP_TUPLE_STRING_MAXLEN);
                    at_chooser_object[AT_NBP_TUPLE_STRING_MAXLEN] = '\0';

                    (void)memcpy(at_chooser_type,nve->at_nve_type,
                                 AT_NBP_TUPLE_STRING_MAXLEN);
                    at_chooser_type[AT_NBP_TUPLE_STRING_MAXLEN] = '\0';

                    (void)memcpy(at_chooser_zone,nve->at_nve_zone,
                                 AT_NBP_TUPLE_STRING_MAXLEN);
                    at_chooser_zone[AT_NBP_TUPLE_STRING_MAXLEN] = '\0';
                    at_chooser_net = nve->at_nve_net;
                    at_chooser_node = nve->at_nve_node;
                    at_chooser_socket = nve->at_nve_socket;
                    return(item);
                    }
                }
            at_error(1452,fn,"logic error - this should not happen");
            return(-1);
            }
        }
