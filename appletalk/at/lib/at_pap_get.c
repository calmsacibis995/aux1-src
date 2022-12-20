#ifndef lint	/* .../appletalk/at/lib/at_pap_get.c */
#define _AC_NAME at_pap_get_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap_get.c on 87/11/11 20:59:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <fcntl.h>

int
at_papsl_get_next_job(fd)
int fd;
{
	struct strioctl s;
	char c[AT_PAP_HDR_SIZE+sizeof(struct atp_set_default)];
	int fd2;
	at_ddp_t *ddp;
	at_atp *atp;
	int socket;
	extern long time();
	long tm;
	int reqid;
	char ch;
	struct atp_set_default *rp;

	fd2 = at_open_dynamic_socket(O_RDWR, &socket);
	if (fd2 < 0) {
		return(-1);
	}
	if (ioctl(fd2, I_PUSH, "at_atp") < 0) {
		at_close_dynamic_socket(fd2);
		return(-1);
	}
	if (ioctl(fd2, I_PUSH, "at_pap") < 0) {
		at_close_dynamic_socket(fd2);
		return(-1);
	}

	rp = (struct atp_set_default *)c;
	rp->def_retries = ATP_INFINITE_RETRIES;
	rp->def_rate = 15*100;;
	ddp = ATP_DDP_HDR(&c[sizeof(struct atp_set_default)]);
	W16(ddp->src_net, at_get_net_number());
	if (R16(ddp->src_net) < 0)
		return(-1);
	ddp->src_node = at_get_node_number();
	ddp->src_socket = socket;
	atp = ATP_ATP_HDR(&c[sizeof(struct atp_set_default)]);
	s.ic_dp = (char *)c;
	s.ic_len = sizeof(c);
	s.ic_timout = -1;
	s.ic_cmd = AT_PAPD_GET_NEXT_JOB;
	if (ioctl(fd, I_STR, &s) < 0) {
		at_close_dynamic_socket(fd2);
		return(-1);
	}
       	ddp->type = 			3;
       	atp->at_atp_cmd = 		AT_ATP_CMD_TREQ;
        atp->at_atp_xo = 		1;
        atp->at_atp_eom =		0;
        atp->at_atp_sts =		0;
        atp->at_atp_unused =		0;
        atp->at_atp_user_bytes[1] = 	0;
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
