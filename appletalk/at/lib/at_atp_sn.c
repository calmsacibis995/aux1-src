#ifndef lint	/* .../appletalk/at/lib/at_atp_sn.c */
#define _AC_NAME at_atp_sn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:59}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_atp_sn.c on 87/11/11 20:57:59";
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
#include <sys/stropts.h>

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();

int at_send_request(socket,treq_buff)
int        socket;
at_atpreq *treq_buff;
{
        char      *fn;
        int        status;
	struct atp_result *rp;
	char	   buff[640*8];
	int i, j;
	struct strioctl s;
	struct atp_set_default *pp;
	at_atp *atp;
	at_ddp_t *ddp;
 
        fn = "at_send_request";
        if ((*treq_buff).at_atpreq_type != AT_ATP_CMD_TREQ) {
            	at_error(1129,fn,"illegal command type\n");
            	return(-1);
        }
        if (!(1 < (*treq_buff).at_atpreq_to_socket && 
              (*treq_buff).at_atpreq_to_socket <= 254)) {
            	at_error(1130,fn,"socket number out of range (%d)",
                     (*treq_buff).at_atpreq_to_socket);
            	return(-1);
        }
        if (!(0 <= (*treq_buff).at_atpreq_treq_length && 
              (*treq_buff).at_atpreq_treq_length <= AT_ATP_DATA_SIZE)) {
            	at_error(1131,fn,"data buffer length out of range (%d)",
                          (*treq_buff).at_atpreq_treq_length);
            	return(-1);
        }

	pp = (struct atp_set_default *)buff;
	pp->def_retries = (int)treq_buff->at_atpreq_maximum_retries;
	pp->def_rate = (int)treq_buff->at_atpreq_retry_timeout*100;
	ddp = ATP_DDP_HDR(&buff[sizeof(struct atp_set_default)]);
	C16(ddp->dst_net, treq_buff->at_atpreq_to_net);
	ddp->dst_node = treq_buff->at_atpreq_to_node;
	ddp->dst_socket = treq_buff->at_atpreq_to_socket;
	atp = ATP_ATP_HDR(&buff[sizeof(struct atp_set_default)]);
	atp->at_atp_xo = treq_buff->at_atpreq_xo;
	atp->at_atp_bitmap_seqno = treq_buff->at_atpreq_treq_bitmap;
	for (i = 0; i < sizeof(at_4bytes); i++)
		atp->at_atp_user_bytes[i] = treq_buff->at_atpreq_treq_user_bytes[i];
	
        s.ic_cmd = AT_ATP_ISSUE_REQUEST_DEF;
	s.ic_timout = -1;
	s.ic_dp = buff;
	s.ic_len = treq_buff->at_atpreq_treq_length +
		   sizeof(struct atp_set_default) +
		   ATP_HDR_SIZE;
	(void) memcpy((char *)atp->at_atp_data,
	       	      (char *)treq_buff->at_atpreq_treq_data,
	       	      treq_buff->at_atpreq_treq_length);
	status = ioctl(socket, I_STR, &s);
        if (status == -1) {
            	at_error(1132,fn,"bad return from at_atp_issue_request()");
            	return(-1);
        }
	rp = (struct atp_result *)buff;
        if (rp->count <= 0) {
            	at_error(1132,fn,"bad return from at_atp_issue_request()");
            	return(-1);
        }
	atp = (at_atp *)&buff[rp->offset[0]];
	treq_buff->at_atpreq_tid = (atp->at_atp_transaction_id[0] << 8) |
				    (atp->at_atp_transaction_id[1] & 0xff);
	treq_buff->at_atpreq_tresp_bitmap  = (1 << rp->count) - 1;
	treq_buff->at_atpreq_tresp_eom_seqno = rp->count-1;
	for (i = 0; i < rp->count; i++) {
		atp = (at_atp *)&buff[rp->offset[i]];
		for (j = 0; j < sizeof(at_4bytes); j++)
			treq_buff->at_atpreq_tresp_user_bytes[i][j] = 
				atp->at_atp_user_bytes[j];
		j = rp->len[i] - AT_ATP_HDR_SIZE;
		treq_buff->at_atpreq_tresp_lengths[i] = j;
		(void) memcpy((char *)treq_buff->at_atpreq_tresp_data[i],
		       	      (char *)atp->at_atp_data,
	               	      j);
	}
	return(sizeof(at_atpreq));
}
