#ifndef lint	/* .../appletalk/at/lib/at_ddp_rd.c */
#define _AC_NAME at_ddp_rd_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:21}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_ddp_rd.c on 87/11/11 20:58:21";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/errno.h>

extern int errno;

int at_read_datagram(fildes,buffer,length,
                     from_net_ptr,from_node_ptr,from_socket_ptr,type_ptr)
        int        fildes;
        char      *buffer;
        int        length;
        int       *from_net_ptr;
        int       *from_node_ptr;
        int       *from_socket_ptr;
        int       *type_ptr;
        {
        auto     char     *fn;
        register at_ddp_t *dg;
        auto     at_ddp_t  local_datagram;
        auto     int       read_length;
        auto     int       minimum_length;
        auto     int       data_length;
        auto     int       status;

        fn = "at_read_datagram()";
        at_errno = 0;
        if (buffer == NULL)
            {
            dg = &at_datagram;
            }
        else
            {
            dg = &local_datagram;
            }
        minimum_length = AT_LAP_HDR_SIZE + AT_DDP_HDR_SIZE;
        read_length = read(fildes,dg,sizeof(at_ddp_t));
        if (read_length == -1)
            {
            if (errno == EINTR || errno == ENETDOWN || errno == ENETRESET)
                {
                at_errno = errno;
                }
            return(-1);
            }
        else if (read_length < minimum_length)
            {
            at_error(1650,fn,"truncated datagram %d instead of %d",
                     read_length,minimum_length);
            return(-1);
            }
        else
            {
            minimum_length = AT_LAP_HDR_SIZE + AT_DDP_X_HDR_SIZE;
            if (read_length < minimum_length)
                {
                at_error(1651,fn,"truncated datagram %d instead of %d",
                             read_length,minimum_length);
                return(-1);
                }
            data_length = at_get_ddp_length(dg->length) - AT_DDP_X_HDR_SIZE;
            if (data_length > length)
                {
                data_length = length;
                }
            if (buffer != NULL)
                {
                (void) memcpy(buffer,(char *)dg->data,data_length);
                }
            if (from_net_ptr != NULL)
                {
                *from_net_ptr = R16(dg->src_net);
                }
            if (from_node_ptr != NULL)
                {
                *from_node_ptr = dg->src_node;
                }
            if (from_socket_ptr != NULL)
                {
                *from_socket_ptr = dg->src_socket;
                }
            if (type_ptr != NULL)
                {
                *type_ptr = dg->type;
                }
            return(data_length);
            }
        }
