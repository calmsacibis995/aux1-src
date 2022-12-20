#ifndef lint	/* .../sys/NET/sys/raw_usrreq.c */
#define _AC_NAME raw_usrreq_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.4 87/11/11 21:17:17}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of raw_usrreq.c on 87/11/11 21:17:17";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)raw_usrreq.c 1.1 86/02/03 SMI; from UCB 6.1 83/07/29	*/
/*	@(#)raw_usrreq.c	2.1 86/04/15 NFSSRC */

#include "sys/param.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"
#include "sys/errno.h"

#include "net/if.h"
#include "net/af.h"
#include "net/netisr.h"
#include "net/route.h"
#include "net/raw_cb.h"

#include "netinet/in.h"

/*
 * Initialize raw connection block q.
 */
raw_init()
{

	rawcb.rcb_next = rawcb.rcb_prev = &rawcb;
	rawintrq.ifq_maxlen = IFQ_MAXLEN;
}

/*
 * Raw protocol interface.
 * XXX Should return an error.
 */
raw_input(m0, proto, src, dst)
	struct mbuf *m0;
	struct sockproto *proto;
	struct sockaddr *src, *dst;
{
	register struct mbuf *m;
	struct raw_header *rh;
	int s;

	/*
	 * Rip off an mbuf for a generic header.
	 */
	m = m_get(M_DONTWAIT, MT_HEADER);
	if (m == 0) {
		m_freem(m0);
		return;
	}
	m->m_next = m0;
	m->m_len = sizeof(struct raw_header);
	rh = mtod(m, struct raw_header *);
	rh->raw_dst = *dst;
	rh->raw_src = *src;
	rh->raw_proto = *proto;

	/*
	 * Header now contains enough info to decide
	 * which socket to place packet in (if any).
	 * Queue it up for the raw protocol process
	 * running at software interrupt level.
	 */
	s = splimp();
	if (IF_QFULL(&rawintrq)) {
		IF_DROP(&rawintrq);
		m_freem(m);
	} else
		IF_ENQUEUE(&rawintrq, m);
	(void) splx(s);
	schednetisr(NETISR_RAW);
}

/*
 * Raw protocol input routine.  Process packets entered
 * into the queue at interrupt time.  Find the socket
 * associated with the packet(s) and move them over.  If
 * nothing exists for this packet, drop it.
 * Use sbappendaddr only if PR_ADDR is requested, else sbappend;
 * note that sbappend does NOT check for space in the sockbuf.
 * The convoluted replicated code is to avoid unnecessary m_copies.
 */
rawintr()
{
	int s;
	struct mbuf *m;
	register struct rawcb *rp;
	register struct raw_header *rh;
	struct socket *lastso;
	struct rawcb *lastrcb;

next:
	s = splimp();
	IF_DEQUEUE(&rawintrq, m);
	(void) splx(s);
	if (m == 0)
		return;
	rh = mtod(m, struct raw_header *);
	lastso = 0;
	for (rp = rawcb.rcb_next; rp != &rawcb; rp = rp->rcb_next) {
		if (rp->rcb_proto.sp_family != rh->raw_proto.sp_family)
			continue;
		if (rp->rcb_proto.sp_protocol &&
		    rp->rcb_proto.sp_protocol != rh->raw_proto.sp_protocol)
			continue;
		/*
		 * We assume the lower level routines have
		 * placed the address in a canonical format
		 * suitable for a structure comparison.
		 */
#define equal(a1, a2) \
	(bcmp((caddr_t)&(a1), (caddr_t)&(a2), sizeof (struct sockaddr)) == 0)
		if ((rp->rcb_flags & RAW_LADDR) &&
		    !equal(rp->rcb_laddr, rh->raw_dst))
			continue;
		if ((rp->rcb_flags & RAW_FADDR) &&
		    !equal(rp->rcb_faddr, rh->raw_src))
			continue;
		if (lastso) {
			struct mbuf *n;
			int result;

			if ((n = m_copy(m->m_next, 0, (int)M_COPYALL)) == 0)
				goto nospace;
			if (lastrcb->rcb_flags & RAW_TALLY) {
				s = splimp();
				m_tally(n, -1,
				    &lastrcb->rcb_cc, &lastrcb->rcb_mbcnt);
				(void) splx(s);
			}
			if (lastso->so_proto->pr_flags & PR_ADDR)
				result = sbappendaddr(&lastso->so_rcv,
				    &rh->raw_src, n, (struct mbuf *)0,
				    lastso->so_proto->pr_flags & PR_RIGHTS);
			else
				result = 1, sbappend(&lastso->so_rcv, n);
			if (result == 0) {
				/* should notify about lost packet */
				m_freem(n);
				goto nospace;
			}
			sorwakeup(lastso);
		}
nospace:
		lastso = rp->rcb_socket;
		lastrcb = rp;
	}
	if (lastso) {
		int result;

		if (lastrcb->rcb_flags & RAW_TALLY) {
			s = splimp();
			m_tally(m->m_next, -1,
			    &lastrcb->rcb_cc, &lastrcb->rcb_mbcnt);
			(void) splx(s);
		}
		if (lastso->so_proto->pr_flags & PR_ADDR)
			result = sbappendaddr(&lastso->so_rcv,
			    &rh->raw_src, m->m_next, (struct mbuf *)0,
			    lastso->so_proto->pr_flags & PR_RIGHTS);
		else
			result = 1, sbappend(&lastso->so_rcv, m->m_next);
		if (result == 0)
			goto drop;
		m = m_free(m);		/* header */
		sorwakeup(lastso);
		goto next;
	}
drop:
	m_freem(m);
	goto next;
}

/*ARGSUSED*/
raw_ctlinput(cmd, arg)
	int cmd;
	caddr_t arg;
{

	if (cmd < 0 || cmd > PRC_NCMDS)
		return;
	/* INCOMPLETE */
}

/*ARGSUSED*/
raw_usrreq(so, req, m, nam, rights)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *rights;
{
	register struct rawcb *rp = sotorawcb(so);
	register int error = 0;
/*	int (*ctlout)() = so->so_proto->pr_ctloutput; */

	if(req == PRU_CONTROL)
	    return(EOPNOTSUPP);
	if (rights && rights->m_len) {
		error = EOPNOTSUPP;
		goto release;
	}
	if (rp == 0 && req != PRU_ATTACH) {
		error = EINVAL;
		goto release;
	}
	switch (req) {

	/*
	 * Allocate a raw control block and fill in the
	 * necessary info to allow packets to be routed to
	 * the appropriate raw interface routine.
	 */
	case PRU_ATTACH:
		if ((so->so_state & SS_PRIV) == 0) {
			error = EACCES;
			break;
		}
		if (rp) {
			error = EINVAL;
			break;
		}
		error = raw_attach(so, (int)nam);
		break;

	/*
	 * Destroy state just before socket deallocation.
	 * Flush data or not depending on the options.
	 */
	case PRU_DETACH:
		if (rp == 0) {
			error = ENOTCONN;
			break;
		}
/*		if (ctlout)
			error = (*ctlout)(rp, req, m, nam); */
		raw_detach(rp);
		break;

	/*
	 * If a socket isn't bound to a single address,
	 * the raw input routine will hand it anything
	 * within that protocol family (assuming there's
	 * nothing else around it should go to). 
	 */
	case PRU_CONNECT:
		if (rp->rcb_flags & RAW_FADDR) {
			error = EISCONN;
			break;
		}
		raw_connaddr(rp, nam);
		soisconnected(so);
		break;

	case PRU_CONNECT2:
		error = EOPNOTSUPP;
		goto release;

	case PRU_BIND:
		if (rp->rcb_flags & RAW_LADDR) {
			error = EINVAL;			/* XXX */
			break;
		}
		error = raw_bind(so, nam);
/*		if (error == 0 && ctlout)
                        error = (*ctlout)(rp, req, m, nam); */
		break;

	case PRU_DISCONNECT:
		if ((rp->rcb_flags & RAW_FADDR) == 0) {
			error = ENOTCONN;
			break;
		}
/*		if (rp->rcb_route.ro_rt)
			rtfree(rp->rcb_route.ro_rt); */
		raw_disconnect(rp);
		soisdisconnected(so);
		break;

	/*
	 * Mark the connection as being incapable of further input.
	 */
	case PRU_SHUTDOWN:
		socantsendmore(so);
		break;

	/*
	 * Ship a packet out.  The appropriate raw output
	 * routine handles any massaging necessary.
	 */
	case PRU_SEND:
		if (nam) {
			if (rp->rcb_flags & RAW_FADDR) {
				error = EISCONN;
				break;
			}
			raw_connaddr(rp, nam);
			/*
			 * clear RAW_FADDR to enable raw receive
			 * semantics by avoiding race between
			 * rawintr inception and pr_output completion
			 */
			rp->rcb_flags &= ~RAW_FADDR;
		} else if ((rp->rcb_flags & RAW_FADDR) == 0) {
			error = ENOTCONN;
			break;
		}
		error = (*so->so_proto->pr_output)(m, so);
		m = NULL;
		break;

	case PRU_ABORT:
		raw_disconnect(rp);
		sofree(so);
		soisdisconnected(so);
		break;

	case PRU_SENSE:
		return(0);

	case PRU_RCVOOB:
	case PRU_RCVD:
		return(EOPNOTSUPP);
	/*
	 * Not supported.
	 */
	case PRU_ACCEPT:
	case PRU_SENDOOB:
	case PRU_LISTEN:
		error = EOPNOTSUPP;
		break;

	case PRU_SOCKADDR:
		bcopy((caddr_t)&rp->rcb_laddr, mtod(nam, caddr_t),
		    sizeof (struct sockaddr));
		nam->m_len = sizeof (struct sockaddr);
		break;

	case PRU_PEERADDR:
		bcopy((caddr_t)&rp->rcb_faddr, mtod(nam, caddr_t),
		    sizeof (struct sockaddr));
		nam->m_len = sizeof (struct sockaddr);
		break;

	default:
		panic("raw_usrreq");
	}
release:
	if (m != NULL)
		m_freem(m);
	return (error);
}
