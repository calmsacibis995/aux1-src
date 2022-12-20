#ifndef lint	/* .../appletalk/at/lib/at_pap_sts.c */
#define _AC_NAME at_pap_sts_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:57}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_sts.c on 87/11/11 20:59:57";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

int
at_pap_status_nve(object, object_len, type, type_len, zone, zone_len, trys, secs)
char *object;
char *type;
char *zone;
int object_len, type_len, zone_len;
int trys, secs;
{
	int i;
	at_nve *nvep;
	int fd;
	at_atpreq treq_buff;
	char rdata[100];

	at_pap_status[0] = '\0';
	i = at_lookup_nve(object, 	object_len,
			  type, 	type_len,
			  zone, 	zone_len,
			  trys,		secs);
	if (i <= 0)
		return(-1);

	nvep = at_nve_lkup_reply_head;
	fd = at_open_requesting_socket();
	if (fd < 0)
		return(-1);

	treq_buff.at_atpreq_type =		AT_ATP_CMD_TREQ;
	W16(treq_buff.at_atpreq_to_net,		nvep->at_nve_net);
	treq_buff.at_atpreq_to_node =		nvep->at_nve_node;
	treq_buff.at_atpreq_to_socket =		nvep->at_nve_socket;
	treq_buff.at_atpreq_treq_user_bytes[0] =0;
	treq_buff.at_atpreq_treq_user_bytes[1] =AT_PAP_TYPE_SEND_STATUS;
	treq_buff.at_atpreq_treq_user_bytes[2] =0;
	treq_buff.at_atpreq_treq_user_bytes[3] =0;
	treq_buff.at_atpreq_treq_length = 	0;
	treq_buff.at_atpreq_treq_bitmap = 	0x01;
	treq_buff.at_atpreq_xo = 		1;
	treq_buff.at_atpreq_retry_timeout = 	2;
	treq_buff.at_atpreq_maximum_retries = 	5;
        treq_buff.at_atpreq_tresp_data[0] =	(unsigned char *)rdata;
        treq_buff.at_atpreq_tresp_lengths[0] =	sizeof(rdata);
	
	for (;;) {
		if (at_send_request(fd, &treq_buff) < 0) {
			at_close_requesting_socket(fd);
			return(-1);
		}
		if (treq_buff.at_atpreq_tresp_user_bytes[0][1] !=
							AT_PAP_TYPE_SEND_STS_REPLY)
			continue;
		at_close_requesting_socket(fd);
		strncpy(at_pap_status, &rdata[5], rdata[4]&0xff);
		at_pap_status[rdata[4]&0xff] = '\0';
		return(0);
	}
}

