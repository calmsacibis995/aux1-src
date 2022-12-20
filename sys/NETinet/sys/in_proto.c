#ifndef lint	/* .../sys/NETinet/sys/in_proto.c */
#define _AC_NAME in_proto_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.5 87/11/11 21:19:31}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.5 of in_proto.c on 87/11/11 21:19:31";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*	@(#)in_proto.c 1.1 86/02/03 SMI; from UCB 6.2 83/12/15  */
/*	@(#)in_proto.c	2.1 86/04/15 NFSSRC */

#include "sys/errno.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/mbuf.h"
#include "sys/sysmacros.h"
#include "sys/socket.h"
#include "sys/protosw.h"
#include "sys/domain.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"

extern int m_freem();
/*
 * TCP/IP protocol family: IP, ICMP, UDP, TCP.
 */
int	ip_output();
int	ip_init(),ip_slowtimo(),ip_drain();
int	icmp_input();
int	udp_input(),udp_ctlinput();
int	udp_usrreq();
int	udp_init();
int	tcp_input(), tcp_ctlinput();
int	tcp_usrreq();
int	tcp_init(),tcp_fasttimo(),tcp_slowtimo(),tcp_drain();
int	rip_input(),rip_output();
int	raw_usrreq();
/*
 * IMP protocol family: raw interface.
 * Using the raw interface entry to get the timer routine
 * in is a kludge.
 */
#if NIMP > 0
int	rimp_output(), hostslowtimo();
#endif
/*
 * Network disk protocol: runs on top of IP
 */
#if NND > 0
int	nd_input(), nd_init();
#endif

/* 
 * Used to switch out the raw interface driver stuff, if no interface is
 * linked into the kernel.
 */


#ifdef ETHERLINK
#ifdef AUTOCONFIG
int nonet_output();
#define ren_output nonet_output
#else
int	ren_output();
#endif
extern struct domain endomain;
#endif

extern struct domain inetdomain;

struct protosw inetsw[] = {
{ 0,		&inetdomain,	0,		0,
  0,		ip_output,	0,		0,
  0,
  ip_init,	0,		ip_slowtimo,	ip_drain,
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_ICMP,	PR_ATOMIC|PR_ADDR,
  icmp_input,	rip_output,	0,		0,
  raw_usrreq,
  0,		0,		0,		0,
},
{ SOCK_DGRAM,	&inetdomain,	IPPROTO_UDP,	PR_ATOMIC|PR_ADDR,
  udp_input,	0,		udp_ctlinput,	0,
  udp_usrreq,
  udp_init,	0,		0,		0,
},
{ SOCK_STREAM,	&inetdomain,	IPPROTO_TCP,	PR_CONNREQUIRED|PR_WANTRCVD,
  tcp_input,	0,		tcp_ctlinput,	0,
  tcp_usrreq,
  tcp_init,	tcp_fasttimo,	tcp_slowtimo,	tcp_drain,
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_RAW,	PR_ATOMIC|PR_ADDR,
  rip_input,	rip_output,	0,	0,
  raw_usrreq,
  0,		0,		0,		0,
},
#if NND > 0
{ 0,		&inetdomain,	IPPROTO_ND,	0,
  nd_input,	0,		0,		0,
  0,
  nd_init,	0,		0,		0,
},
#endif
};

struct domain inetdomain =
    { AF_INET, "internet", 0, 0, 0,
    inetsw, &inetsw[sizeof(inetsw)/sizeof(inetsw[0])] };

#if NIMP > 0
struct protosw impsw[] = {
{ SOCK_RAW,	&impdomain,	0,		PR_ATOMIC|PR_ADDR,
  0,		rimp_output,	0,		0,
  raw_usrreq,
  0,		0,		hostslowtimo,	0,
},
};

struct domain impdomain =
    { AF_IMPLINK, "imp", 0, 0, 0,
    impsw, &impsw[sizeof (impsw)/sizeof(impsw[0])] };
#endif

#ifdef ETHERLINK
struct protosw ensw[] = {
{ SOCK_RAW,	&endomain,	0,		PR_ATOMIC|PR_ADDR,
  0,		ren_output,	0,		0,
  raw_usrreq,
  0,		0,		0,		0,
},
};

struct domain endomain =
    { AF_ETHERLINK, "etherlink", 0, 0, 0,
    ensw, &ensw[sizeof (ensw)/sizeof(ensw[0])] };

nonet_output(m0, so)
    	struct mbuf *m0;
	struct socket *so;
{
	if(m0 != 0)
	    m_freem(m0);
	return(ENETDOWN);
}
#endif
