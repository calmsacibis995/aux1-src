#ifndef lint	/* .../sys/PAGING/os/usyslocal.c */
#define _AC_NAME usyslocal_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:28:42}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of usyslocal.c on 87/11/11 21:28:42";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)usyslocal.c	UniPlus VVV.2.1.1	*/

#ifdef HOWFAR
extern int T_usyslocal;
#endif HOWFAR
#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/uconfig.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/time.h"
#include "sys/proc.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/map.h"
#include "sys/var.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "svfs/inode.h"

#include "sys/reg.h"
#include "sys/sysinfo.h"
#include "sys/swap.h"
#include "sys/debug.h"
#include "sys/buserr.h"
#include "sys/tuneable.h"

#endif lint

/*	Sys3b function 3 - manipulate swap files.
 */

swapfunc(si)
register swpi_t	*si;
{
	register int		i;
	register struct inode	*ip;
	struct vnode		*vp;
	swpi_t			swpbuf;
	int			error;
	struct pathname		pn;

	error = copyin(si, &swpbuf, sizeof(swpi_t));
	if (error)
		return (error);

	si = &swpbuf;

	switch(si->si_cmd) {
		case SI_LIST:
			i = sizeof(swpt_t) * MSFILES;
			error = copyout((caddr_t)swaptab, si->si_buf, i);
			if (error)
				return (error);
			break;

		case SI_ADD:
		case SI_DEL:
			if(!suser())
				return(EPERM);
			error = pn_get(si->si_buf, UIOSEG_USER, &pn);
			if (error)
				return(error);
			error = lookuppn(&pn, FOLLOW_LINK, (struct vnode **)0, &vp);
			pn_free(&pn);
			if (error)
				return(error);
			if (vp->v_op != &svfs_vnodeops) {
				VN_RELE(vp);
				return (EINVAL);
			}
			else
				ip = (struct inode *) vp->v_data;
			if((ip->i_mode & IFMT)  !=  IFBLK)
				return(ENOTBLK);

			if(si->si_cmd == SI_DEL)
				error = swapdel((dev_t)ip->i_rdev, si->si_swplo);
			else
				error = swapadd((dev_t)ip->i_rdev, si->si_swplo,
					si->si_nblks);
			VN_RELE(vp);
			return(error);
	}
	return(0);
}

/*
 * Profiling
 */

profil()
{
	register struct a {
		short	*bufbase;
		unsigned bufsize;
		unsigned pcoffset;
		unsigned pcscale;
	} *uap;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	up->u_prof.pr_base = uap->bufbase;
	up->u_prof.pr_size = uap->bufsize;
	up->u_prof.pr_off = uap->pcoffset;
	up->u_prof.pr_scale = uap->pcscale;
}

ulimit()
{
	register struct user *up;
	register struct a {
		int	cmd;
		long	arg;
	} *uap;

	up = &u;
	uap = (struct a *)up->u_ap;
	switch(uap->cmd) {
	case 2:
		if (uap->arg > up->u_limit && !suser())
			return;
		up->u_limit = uap->arg;
	case 1:
		up->u_roff = up->u_limit;
		break;

	case 3:{
		register preg_t	*prp, *dprp, *prp2;
		register reg_t *rp;
		register size = 0;

		/*	Find the data region
		 */

		dprp = findpreg(up->u_procp, PT_DATA);
		if(dprp == NULL)
			up->u_roff = 0;
		else {
			/*	Now find the region with a virtual
			 *	address greater than the data region
			 *	but lower than any other region
			 */
			prp2 = NULL;
			for(prp = up->u_procp->p_region; prp->p_reg; prp++) {
				if(prp->p_regva <= dprp->p_regva)
					continue;
				if(prp2==NULL || prp->p_regva < prp2->p_regva)
					prp2 = prp;
			}
			if(prp2 == NULL)
				up->u_roff = stob(NSEGP);
			else if (prp2->p_reg->r_stack)
				up->u_roff = (off_t) (USRSTACK - 
					stob(ptos(prp2->p_reg->r_pgsz)));
			else
				up->u_roff = (off_t)prp2->p_regva;
			prp = up->u_procp->p_region;
			while(rp = prp->p_reg){
				if (prp == dprp)
					size += btop((caddr_t)up->u_roff - 
							prp->p_regva);
				else
					size += rp->r_pgsz;
				prp++;
			}
			if(size > tune.t_maxumem)
				up->u_roff -= (off_t) ptob(size - tune.t_maxumem);
		}
		break;
	}

	default:
		up->u_error = EINVAL;
	}
}
