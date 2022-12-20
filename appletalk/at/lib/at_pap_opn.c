#ifndef lint	/* .../appletalk/at/lib/at_pap_opn.c */
#define _AC_NAME at_pap_opn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:42}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_opn.c on 87/11/11 20:59:42";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>
char at_pap_status[600];

int
at_pap_open_nve(object, object_len, type, type_len, zone, zone_len, trys, secs, retry, name)
char *object;
char *type;
char *zone;
int trys, secs, retry;
char *name;
{
	int fd, fd2;
	int i, first;
	long tm;
	extern long time();
	char c[AT_PAP_HDR_SIZE + sizeof(struct atp_set_default)];
	at_nve *nvep;
	int reqid;
	struct strioctl s;
	unsigned char data[10], rdata[640];
	at_atpreq treq_buff;
	int socket;
	at_ddp_t *ddp;
	at_atp *atp;
	struct atp_set_default *rp;

	at_pap_status[0] = '\0';
	fd = at_open_requesting_socket();
	if (fd < 0)
		return(-1);

	fd2 = at_open_dynamic_socket(O_RDWR, &socket);
	if (fd2 < 0) {
		at_close_requesting_socket(fd);
		return(-1);
	}
	if (ioctl(fd2, I_PUSH, "at_atp") < 0) {
		at_close_requesting_socket(fd);
		at_close_dynamic_socket(fd2);
		return(-1);
	}
	if (ioctl(fd2, I_PUSH, "at_pap") < 0) {
		at_close_requesting_socket(fd);
		at_close_dynamic_socket(fd2);
		return(-1);
	}
	tm = time(NULL);
	srand(tm);

	rp = (struct atp_set_default *)c;
	rp->def_retries = ATP_INFINITE_RETRIES;
	rp->def_rate = 15*100;;
	ddp = ATP_DDP_HDR(&c[sizeof(struct atp_set_default)]);
        W16(ddp->dst_net, 	0);
        ddp->dst_node = 	0;
        ddp->dst_socket = 	0;
        ddp->type = 		3;
	atp = ATP_ATP_HDR(&c[sizeof(struct atp_set_default)]);
        atp->at_atp_cmd = 	AT_ATP_CMD_TREQ;
        atp->at_atp_xo = 	1;
        atp->at_atp_eom =	0;
        atp->at_atp_sts =	0;
        atp->at_atp_unused =	0;
        atp->at_atp_bitmap_seqno =	1;
        atp->at_atp_user_bytes[0] = 	0;
        atp->at_atp_user_bytes[1] = 	0;
        atp->at_atp_user_bytes[2] = 	0;
        atp->at_atp_user_bytes[3] = 	0;
	s.ic_timout = -1;
	s.ic_len = sizeof(c);
	s.ic_dp = (char *)c;
	s.ic_cmd = AT_PAP_SETHDR;
	if (ioctl(fd2, I_STR, &s) < 0) {
		at_close_requesting_socket(fd);
		at_close_dynamic_socket(fd2);
		return(-1);
	}

	strcpy(&rdata[5], "No Printers");
	rdata[4] = strlen("No Printers");
	nvep = NULL;	
	first = 1;
	for (;;) {
		while (nvep == NULL) {
			if (first) {
				first = 0;
			} else {
				sleep(120);
			}
			i = at_lookup_nve(object, 	object_len,
					  type, 	type_len,
					  zone, 	zone_len,
					  trys,		secs);
			if (i <= 0 ||
			    (nvep = at_nve_lkup_reply_head) == NULL) {
				if (retry == 0) {
					strncpy(at_pap_status, &rdata[5], rdata[4]&0xff);
					at_pap_status[rdata[4]&0xff] = '\0';
					at_close_requesting_socket(fd);
					at_close_dynamic_socket(fd2);
					return(-1);
				}
				if (retry > 0)
					retry--;
			}
		}
		reqid = rand()&0xff;
		W16(treq_buff.at_atpreq_to_net,		nvep->at_nve_net);
		treq_buff.at_atpreq_to_node =		nvep->at_nve_node;
		treq_buff.at_atpreq_to_socket =		nvep->at_nve_socket;
		treq_buff.at_atpreq_type =		AT_ATP_CMD_TREQ;
		treq_buff.at_atpreq_to_node =		nvep->at_nve_node;
		treq_buff.at_atpreq_to_socket =		nvep->at_nve_socket;
		treq_buff.at_atpreq_treq_user_bytes[0] =reqid;
		treq_buff.at_atpreq_treq_user_bytes[1] =AT_PAP_TYPE_OPEN_CONN;
		treq_buff.at_atpreq_treq_user_bytes[2] =0;
		treq_buff.at_atpreq_treq_user_bytes[3] =0;
		treq_buff.at_atpreq_treq_data = data;
		data[0] = 				socket;
		data[1] = 				8;
		treq_buff.at_atpreq_treq_length = 	4;
		treq_buff.at_atpreq_treq_bitmap = 	0x01;
		treq_buff.at_atpreq_xo = 		1;
		treq_buff.at_atpreq_retry_timeout = 	2;
		treq_buff.at_atpreq_maximum_retries = 	5;
        	treq_buff.at_atpreq_tresp_data[0] =	rdata;
        	treq_buff.at_atpreq_tresp_lengths[0] =	sizeof(rdata);
		i = time(NULL) - tm;
		data[2] = i>>8;
		data[3] = i&0xff;
		if (at_send_request(fd, &treq_buff) >= 0) {
			if ((treq_buff.at_atpreq_tresp_user_bytes[0][0]&0xff) != reqid)
				continue;
			if (treq_buff.at_atpreq_tresp_user_bytes[0][1] !=
							AT_PAP_TYPE_OPEN_CONN_REPLY)
				continue;
			if (rdata[2] == 0 && rdata[3] == 0)
				break;
		} else {
			strcpy(&rdata[5], "Unreachable");
			rdata[4] = strlen("Unreachable");
		}
		nvep = nvep->at_nve_next;
	}
	at_close_requesting_socket(fd);
	strncpy(at_pap_status, &rdata[5], rdata[4]&0xff);
	at_pap_status[rdata[4]&0xff] = '\0';
	if (name) {
		strcpy(name, nvep->at_nve_object);
	}
        W16(ddp->dst_net, 		nvep->at_nve_net);
        ddp->dst_node = 		nvep->at_nve_node;
        ddp->dst_socket = 		rdata[0];
        ddp->type = 			3;
        atp->at_atp_cmd = 		AT_ATP_CMD_TREQ;
        atp->at_atp_xo = 		1;
        atp->at_atp_eom =		0;
        atp->at_atp_sts =		0;
        atp->at_atp_unused =		0;
        atp->at_atp_bitmap_seqno =	(1<<rdata[1])-1;
        atp->at_atp_user_bytes[0] = 	reqid;
        atp->at_atp_user_bytes[1] = 	rdata[1];
        atp->at_atp_user_bytes[2] = 	0;
        atp->at_atp_user_bytes[3] = 	0;
	s.ic_timout = -1;
	s.ic_len = sizeof(c);
	s.ic_dp = (char *)c;
	s.ic_cmd = AT_PAP_SETHDR;
	if (ioctl(fd2, I_STR, &s) < 0) {
		at_close_dynamic_socket(fd2);
		return(-1);
	}

	return(fd2);
}
