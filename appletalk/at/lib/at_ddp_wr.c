#ifndef lint	/* .../appletalk/at/lib/at_ddp_wr.c */
#define _AC_NAME at_ddp_wr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_ddp_wr.c on 87/11/11 20:58:24";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/errno.h>

extern int errno;

int at_write_datagram(fildes,buffer,length,net,node,socket,type)
        int        fildes;
        char      *buffer;
        int        length;
        int        net;
        int        node;
        int        socket;
        int        type;
        {
        auto      char      *fn;
        auto      at_ddp_t   local_datagram;   
        register  at_ddp_t  *dgx;
        auto      unsigned   write_length;
        auto      int        status;

        fn = "at_write_datagram()";
        if (buffer == NULL)
            {
            dgx = &at_datagram;
            }
        else
            {
            dgx = &local_datagram;
            }

        if (!(0 <= net && net <= 65535))
            {
            at_error(1700,fn,"net number out of range (%d)",net);
            return (-1);
            }

        if (!(0 < node && node <= 255))
            {
            at_error(1701,fn,"node number out of range (%d)",node);
            return (-1);
            }

        if (!(1 <= socket && socket <= 254))
            {
            at_error(1702,fn,"socket number out of range (%d)",socket);
            return (-1);
            }

        W16(dgx->dst_net,    net);
/*
        dgx->dst_net[1] = net & 0xff;
*/
        dgx->dst_node      = node;
        dgx->dst_socket    = socket;
        dgx->type          = type;
        dgx->type          = AT_LAP_TYPE_DDP_X;
        if (length <= AT_DDP_DATA_SIZE)
            {
            at_set_ddp_length(dgx->length,length+AT_DDP_X_HDR_SIZE);
            if (buffer != NULL)
                {
                (void) memcpy((char *)dgx->data,buffer,length);
                }
            write_length = AT_LAP_HDR_SIZE + AT_DDP_X_HDR_SIZE + length;
            status = write(fildes,(char *)dgx,write_length);
            if (status == -1)
                {
                if (errno == EHOSTDOWN)
                    {
                    at_errno = errno;
                    return(-1);
                    }
                else
                    {
                    at_error(1703,fn,"bad return from write()");
                    return(-1);
                    }
                }
            else if (status != write_length)
                {
                at_error(1704,fn,"datagram write returned %d instead of %d",
                         status,write_length);
                return(-1);
                }
            else
                {
                return(length);
                }
            }
        else
            {
            at_error(1705,fn,"buffer length %d too large for datagram",length);
            return(-1);
            }
        }
