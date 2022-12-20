#ifndef lint	/* .../appletalk/at/lib/atp_st_hdr.c */
#define _AC_NAME atp_st_hdr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:01:31}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_st_hdr.c on 87/11/11 21:01:31";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"

int
at_atp_set_hdr(req, resp)
register at_ddp_t *req, *resp;
{
	register at_atp *areq, *aresp;

	C16(resp->dst_net, req->src_net);
	resp->dst_node = req->src_node;
	resp->dst_socket = req->src_socket;
        areq = ATP_ATP_HDR(req);
        aresp = ATP_ATP_HDR(resp);
        aresp->at_atp_transaction_id[0] = areq->at_atp_transaction_id[0];
        aresp->at_atp_transaction_id[1] = areq->at_atp_transaction_id[1];
}

