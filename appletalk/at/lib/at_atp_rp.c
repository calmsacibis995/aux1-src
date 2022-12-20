#ifndef lint	/* .../appletalk/at/lib/at_atp_rp.c */
#define _AC_NAME at_atp_rp_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:49}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_atp_rp.c on 87/11/11 20:57:49";
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

int at_send_response(socket,treq_buff)
int        socket;
at_atpreq *treq_buff;
{
        char       *fn;
        int        status;
	int	   loop;
	int	   i, j;
	char	   buff[650];
	at_atp	   *atp;
	at_ddp_t   *ddp;
 
        fn = "at_send_response";
        if ((*treq_buff).at_atpreq_type != AT_ATP_CMD_TRESP) {
            	at_error(1150,fn,"illegal command type\n");
            	return(-1);
        }
        if (!(1 < (*treq_buff).at_atpreq_to_socket && 
              (*treq_buff).at_atpreq_to_socket <= 254)) {
            	at_error(1151,fn,"socket number out of range (%d)",
                     (*treq_buff).at_atpreq_to_socket);
            	return(-1);
        }
        for (loop=0;loop<AT_ATP_TRESP_MAX;loop++) {
            if (!(0 <= (*treq_buff).at_atpreq_tresp_lengths[loop] && 
                (*treq_buff).at_atpreq_tresp_lengths[loop] <= AT_ATP_DATA_SIZE)) {
                	at_error(1152,fn,"data buffer length %d out of range (%d)",loop,
                              (*treq_buff).at_atpreq_tresp_lengths[loop]);
                	return(-1);
        	}
        }
        if (!(0 <= (*treq_buff).at_atpreq_tresp_eom_seqno &&
            (*treq_buff).at_atpreq_tresp_eom_seqno <= AT_ATP_TRESP_MAX)) {
            	at_error(1153,fn,"eom sequence number out of range (%d)",
                     (*treq_buff).at_atpreq_tresp_eom_seqno);
            	return(-1);
        }

	ddp = ATP_DDP_HDR(buff);
	C16(ddp->dst_net, treq_buff->at_atpreq_to_net);
	ddp->dst_node = treq_buff->at_atpreq_to_node;
	ddp->dst_socket = treq_buff->at_atpreq_to_socket;
	atp = ATP_ATP_HDR(buff);
	atp->at_atp_transaction_id[0] = treq_buff->at_atpreq_tid >> 8;
	atp->at_atp_transaction_id[1] = treq_buff->at_atpreq_tid;
	for (i = 0; i < 8; i++)
	if ((1<<i)&treq_buff->at_atpreq_tresp_bitmap) {
		for (j = 0; j < sizeof(at_4bytes); j++)
			atp->at_atp_user_bytes[j] =
			treq_buff->at_atpreq_tresp_user_bytes[i][j];
		j = treq_buff->at_atpreq_tresp_lengths[i];
		memcpy((char *)atp->at_atp_data,
		       (char *)treq_buff->at_atpreq_tresp_data[i],
	               j);
		if (i == treq_buff->at_atpreq_tresp_eom_seqno) {
			status = at_atp_send_response_eof(socket,i,buff, j+ATP_HDR_SIZE);
       			if (status == -1) {
          			at_error(1154,fn,
					"bad return from at_atp_send_response_eof()");
            			return(-1);
			}
		} else {
			status = at_atp_send_response(socket, i, buff, j + ATP_HDR_SIZE);
        		if (status == -1) {
            			at_error(1155,fn,
					"bad return from at_atp_send_response()");
            			return(-1);
			}
		}
        }
        return(sizeof(*treq_buff));
}
