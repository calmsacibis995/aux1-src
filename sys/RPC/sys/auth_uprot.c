#ifndef lint	/* .../sys/RPC/sys/auth_uprot.c */
#define _AC_NAME auth_uprot_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:30:16}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of auth_uprot.c on 87/11/11 21:30:16 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/* NFSSRC @(#)authunix_prot.c	2.2 86/04/14 */
#ifndef lint
#define _AC_MODS
#endif

/*
 * authunix_prot.c
 * XDR for UNIX style authentication parameters for RPC
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef KERNEL
#include "sys/param.h"
#include "sys/signal.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#include "sys/region.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "rpc/types.h"
#include "rpc/xdr.h"
#include "rpc/auth.h"
#include "rpc/auth_unix.h"
#else
#include "types.h"	/* <> */
#include "xdr.h"	/* <> */
#include "auth.h"	/* <> */
#include "auth_unix.h"	/* <> */
#endif

/*
 * XDR for unix authentication parameters.
 */
bool_t
xdr_authunix_parms(xdrs, p)
	register XDR *xdrs;
	register struct authunix_parms *p;
{

	if (xdr_u_long(xdrs, &(p->aup_time))
	    && xdr_string(xdrs, &(p->aup_machname), MAX_MACHINE_NAME)
	    && xdr_int(xdrs, &(p->aup_uid))
	    && xdr_int(xdrs, &(p->aup_gid))
	    && xdr_array(xdrs, (caddr_t *)&(p->aup_gids),
		    &(p->aup_len), NGRPS, sizeof(int), xdr_int) ) {
		return (TRUE);
	}
	return (FALSE);
}

#ifdef KERNEL
/*
 * XDR kernel unix auth parameters.
 * Goes out of the u struct directly.
 * NOTE: this is an XDR_ENCODE only routine.
 */
xdr_authkern(xdrs)
	register XDR *xdrs;
{
	int	*gp;
	int	 uid = u.u_uid;
	int	 gid = u.u_gid;
	int	 len;
	caddr_t	groups;
	char	*name = hostname;

	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}

	for (gp = &u.u_groups[NGRPS]; gp > u.u_groups; gp--) {
		if (gp[-1] >= 0) {
			break;
		}
	}
	len = gp - u.u_groups;
	groups = (caddr_t)u.u_groups;
        if (xdr_u_long(xdrs, (u_long *)&time.tv_sec)
            && xdr_string(xdrs, &name, MAX_MACHINE_NAME)
            && xdr_int(xdrs, &uid)
            && xdr_int(xdrs, &gid)
	    && xdr_array(xdrs, &groups, (u_int *)&len, NGRPS, sizeof (int), xdr_int) ) {
                return (TRUE);
	}
	return (FALSE);
}
#endif
