#ifndef lint	/* .../sys/NET/sys/af.c */
#define _AC_NAME af_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1980-87 The Regents of the University of California, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:16:37}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of af.c on 87/11/11 21:16:37";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	af.c	6.1	83/07/29	*/

#include "sys/param.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/mbuf.h"
#include "sys/protosw.h"
#include "net/af.h"

/*
 * Address family support routines
 */
int	null_hash(), null_netmatch();
#define	AFNULL \
	{ null_hash,	null_netmatch }

#ifdef INET
extern int inet_hash(), inet_netmatch();
#define	AFINET \
	{ inet_hash,	inet_netmatch }
#else
#define	AFINET	AFNULL
#endif

#ifdef PUP
extern int pup_hash(), pup_netmatch();
#define	AFPUP \
	{ pup_hash,	pup_netmatch }
#else
#define	AFPUP	AFNULL
#endif

struct afswitch afswitch[AF_MAX] = {
	AFNULL,	AFNULL,	AFINET,	AFINET,	AFPUP,
	AFNULL,	AFNULL,	AFNULL,	AFNULL,	AFNULL,
	AFNULL
};

/*ARGSUSED*/
null_hash(addr, hp)
	struct sockaddr *addr;
	struct afhash *hp;
{

	hp->afh_nethash = hp->afh_hosthash = 0;
}

/*ARGSUSED*/
null_netmatch(a1, a2)
	struct sockaddr *a1, *a2;
{

	return (0);
}
