#ifndef lint	/* .../sys/NET/sys/if.c */
#define _AC_NAME if_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.3 87/11/11 21:16:49}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of if.c on 87/11/11 21:16:49";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)if.c 1.1 86/02/03 SMI; from UCB 6.2 83/09/27	*/
/*	@(#)if.c	2.3 86/04/24 NFSSRC */

#include "sys/param.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/mbuf.h"
#include "sys/protosw.h"
#include "net/if.h"
#include "net/af.h"
#include "net/route.h"
#include "net/raw_cb.h"
#ifdef	NIT
#include "net/nit.h"
#endif	NIT
#include "netinet/in.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/ioctl.h"

int	ifqmaxlen = IFQ_MAXLEN;

/*
 * Network interface utility routines.
 *
 * Routines with if_ifwith* names take sockaddr *'s as
 * parameters.  Other routines take value parameters,
 * e.g. if_ifonnetof takes the network number.
 */

ifinit()
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_init) {
			(*ifp->if_init)(ifp->if_unit);
			if (ifp->if_snd.ifq_maxlen == 0)
				ifp->if_snd.ifq_maxlen = ifqmaxlen;
		}
	if_slowtimo();
}

#ifdef vax
/*
 * Call each interface on a Unibus reset.
 */
ifubareset(uban)
	int uban;
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_reset)
			(*ifp->if_reset)(ifp->if_unit, uban);
}
#endif

/*
 * Attach an interface to the
 * list of "active" interfaces.
 */
if_attach(ifp)
	struct ifnet *ifp;
{
	register struct ifnet **p = &ifnet;

	while (*p)
		p = &((*p)->if_next);
	*p = ifp;
}

/*
 * Locate an interface based on a complete address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithaddr(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

#define	equal(a1, a2) \
	(bcmp((caddr_t)((a1)->sa_data), (caddr_t)((a2)->sa_data), 14) == 0)
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr.sa_family != addr->sa_family)
			continue;
		if (equal(&ifa->ifa_addr, addr))
			return (ifa);
		if ((ifp->if_flags & IFF_BROADCAST) &&
		    equal(&ifa->ifa_broadaddr, addr))
			return (ifa);
	}
	return ((struct ifaddr *)0);
}
/*
 * Locate the point to point interface with a given destination address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithdstaddr(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) 
	    if (ifp->if_flags & IFF_POINTOPOINT)
		for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr.sa_family != addr->sa_family)
				continue;
			if (equal(&ifa->ifa_dstaddr, addr))
				return (ifa);
	}
	return ((struct ifaddr *)0);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is first found.
 */
struct ifaddr *
ifa_ifwithnet(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	register u_int af = addr->sa_family;
	register int (*netmatch)();

	if (af >= AF_MAX)
		return (0);
	netmatch = afswitch[af].af_netmatch;
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr.sa_family != addr->sa_family)
			continue;
		if ((*netmatch)(&ifa->ifa_addr, addr))
			return (ifa);
	}
	return ((struct ifaddr *)0);
}

#ifdef notdef
/*
 * Find an interface using a specific address family
 */
struct ifaddr *
ifa_ifwithaf(af)
	register int af;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_addr.sa_family == af)
			return (ifa);
	return ((struct ifaddr *)0);
}
#endif

/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
if_down(ifp)
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;
	register int (*rtn)() = ifp->if_ioctl;
	int flags = ifp->if_flags;

	ifp->if_flags &= ~IFF_UP;

	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		pfctlinput(PRC_IFDOWN, &ifa->ifa_addr);
}

/*
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 */
if_slowtimo()
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_timer == 0 || --ifp->if_timer)
			continue;
		if (ifp->if_watchdog)
			(*ifp->if_watchdog)(ifp->if_unit);
	}
	timeout(if_slowtimo, (caddr_t)0, v.v_hz / IFNET_SLOWHZ);
}

/*
 * Map interface name to
 * interface structure pointer.
 */
struct ifnet *
ifunit(name, size)
	register char *name;
	register size;
{
	register char *cp;
	register struct ifnet *ifp;
	int unit;
	int namlen = 0;

	for (cp = name; cp < name + size && *cp; cp++) {
		if (*cp >= '0' && *cp <= '9')
			break;
		namlen++;
	}
	if (*cp == '\0' || cp == name + size || cp == name)
		return ((struct ifnet *)0);
	unit = 0;
	while (*cp && cp < name + IFNAMSIZ) 
		unit = 10*unit + (*cp++ - '0');
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (unit == ifp->if_unit && namlen == strlen(ifp->if_name) &&
		    bcmp(ifp->if_name, name, namlen) == 0)
			return (ifp);
	return ((struct ifnet *)0);
}

int     in_interfaces;          /* number of external internet interfaces */
extern  struct ifnet loif;

/*
 * Interface ioctls.
 */
ifioctl(so, cmd, data)
	struct socket *so;
	int cmd;
	caddr_t data;
{
	register struct ifnet *ifp, *oifp;
	register struct ifreq *ifr;
	int error = 0;

	switch (cmd) {

	case SIOCGIFCONF:
		return (ifconf(cmd, data));

	case SIOCSARP:
	case SIOCDARP:
		if (!suser())
			return (u.u_error);
		/* fall through */
	case SIOCGARP:
		return (arpioctl(cmd, data));

	case SIOCSIFADDR:
	case SIOCSIFFLAGS:
	case SIOCSIFDSTADDR:
	case SIOCSIFMTU:
		if (!suser())
			return (u.u_error);
		break;
	}

	ifr = (struct ifreq *)data;
	ifp = ifunit(ifr->ifr_name, IFNAMSIZ);
#ifndef NIT
	if (ifp == 0)
		return (ENXIO);
#else NIT
	if (ifp == 0 || (so->so_proto->pr_domain->dom_family == PF_NIT &&
		    so->so_proto->pr_type == SOCK_RAW)) {
		/* handle direct connects of ioctl's to interfaces */
		/* XXX consider attempting other paths if this one fails. */
		register struct rawcb *rcb = (struct rawcb *)so->so_pcb;

		if (ifp == 0 && rcb->rcb_flags & RAW_LADDR &&
		    rcb->rcb_laddr.sa_family == AF_NIT)
			ifp = nit_ifwithaddr(&rcb->rcb_laddr);
		if (ifp == 0)
			return (ENXIO);
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		if (!suser())		/* XXX redundant */
			return (u.u_error);
		return ((*ifp->if_ioctl)(ifp, cmd, data));
	}
#endif NIT

	switch (cmd) {

	case SIOCGIFMTU:
		*(int *)ifr->ifr_data = ifp->if_mtu;
		break;

	case SIOCGIFFLAGS:
		ifr->ifr_flags = ifp->if_flags;
		break;

	case SIOCGIFMETRIC:
		ifr->ifr_metric = ifp->if_metric;
		break;

	case SIOCSIFFLAGS:
		if ((ifp->if_flags&IFF_UP) &&
		    (ifr->ifr_flags&IFF_UP) == 0) {
			int s = splimp();
			if_down(ifp);
			(void) splx(s);
		}
		ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
			(ifr->ifr_flags &~ IFF_CANTCHANGE);
		if (ifp->if_ioctl)
		    (*ifp->if_ioctl)(ifp, cmd, data);
		break;

	case SIOCSIFMETRIC:
		if (!suser())
			return (u.u_error);
		ifp->if_metric = ifr->ifr_metric;
		break;

	case SIOCUPPER:
		oifp = ifunit(ifr->ifr_oname, IFNAMSIZ);
		if (oifp == 0)
			return (ENXIO);
		if (oifp->if_input == 0)
			return (EINVAL);
		ifp->if_upper = oifp;
		break;

	case SIOCLOWER:
		oifp = ifunit(ifr->ifr_oname, IFNAMSIZ);
		if (oifp == 0)
			return (ENXIO);
		if (oifp->if_output == 0)
			return (EINVAL);
		ifp->if_lower = oifp;
		break;

	default:
		if (so->so_proto == 0)
			return (EOPNOTSUPP);
		return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
			cmd, data, ifp));
	}
	return (0);
}    

/*
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 */
/*ARGSUSED*/
ifconf(cmd, data)
	int cmd;
	caddr_t data;
{
	register struct ifconf *ifc = (struct ifconf *)data;
	register struct ifnet *ifp = ifnet;
	register struct ifaddr *ifa;
	register char *cp, *ep;
	struct ifreq ifr, *ifrp;
	int space = ifc->ifc_len, error = 0;

	ifrp = ifc->ifc_req;
	ep = ifr.ifr_name + sizeof (ifr.ifr_name) - 2;
	for (; space > sizeof (ifr) && ifp; ifp = ifp->if_next) {
		bcopy(ifp->if_name, ifr.ifr_name, sizeof (ifr.ifr_name) - 2);
		for (cp = ifr.ifr_name; cp < ep && *cp; cp++)
			;
		*cp++ = '0' + ifp->if_unit; *cp = '\0';
		if ((ifa = ifp->if_addrlist) == 0) {
			bzero((caddr_t)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			error = copyout((caddr_t)&ifr, (caddr_t)ifrp, sizeof (ifr));
			if (error)
				break;
			space -= sizeof (ifr), ifrp++;
		} else 
		    for ( ; space > sizeof (ifr) && ifa; ifa = ifa->ifa_next) {
			ifr.ifr_addr = ifa->ifa_addr;
			error = copyout((caddr_t)&ifr, (caddr_t)ifrp, sizeof (ifr));
			if (error)
				break;
			space -= sizeof (ifr), ifrp++;
		}
	}
	ifc->ifc_len -= space;
	return (error);
}
