#ifndef lint	/* .../sys/PAGING/os/blklist.c */
#define _AC_NAME blklist_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:23:39}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of blklist.c on 87/11/11 21:23:39";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)blklist.c	UniPlus VVV.2.1.2	*/

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include	"sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include	"sys/param.h"
#include	"sys/sysmacros.h"
#include	"sys/page.h"
#include	"sys/dir.h"
#include	"sys/signal.h"
#include	"sys/time.h"
#include	"sys/vnode.h"
#include	"sys/user.h"
#include	"sys/buf.h"
#include	"sys/region.h"
#include 	"sys/proc.h"
#include	"sys/pfdat.h"
#include	"sys/debug.h"
#include	"sys/vfs.h"
#endif lint


/*	Build the list of block numbers for a file.  This is used
 *	for mapped files.
 */

bldblklst(lp, vp, nblks)
register int		*lp;
register struct vnode	*vp;
register int		nblks;
{
	register daddr_t	bfirst;
	struct vnode	*mapped_vp;
	daddr_t		mapped_bn;

	for (bfirst = (daddr_t) 0; bfirst < (daddr_t) nblks; bfirst++) {
		VOP_BMAP(vp, bfirst, &mapped_vp, &mapped_bn);
#ifdef NOTDEFASA
		if (mapped_bn == (daddr_t) -1)
			*lp++ = 0;
		else
#endif NOTDEFASA
			*lp++ = mapped_bn;
	}
	vp->v_mappedvp = mapped_vp;
}

/*	Free the block list attached to an vnode.
 */

freeblklst(vp)
register struct vnode	*vp;
{
	register int	nblks;
	register int	blkspp;
	register int	i;
	dbd_t		dbd;
	reg_t		reg;
	u_int		bsize;
	struct vattr	vattr;

	dbd.dbd_type = DBD_FILE;
	reg.r_vptr = vp;

	VOP_GETATTR(vp, &vattr, u.u_cred);
	bsize = vp->v_vfsp->vfs_bsize;
	nblks = (vattr.va_size + bsize - 1)/bsize;
	blkspp = NBPP/bsize;
	nblks = ((nblks + blkspp - 1) / blkspp) * blkspp;
	memlock();
	for(i = 0  ;  i < nblks  ;  i += blkspp){

		/*	Note the following grossness.  When we
		**	inserted these pages, we used either
		**	the first or the second block number
		**	to hash on depending on whether the
		**	page was private or shared.  Now we
		**	don't know which it is so we must do
		**	the pbremove twice to be sure we get
		**	the page.  Note that the page which
		**	contains both text and data could
		**	be in the table twice so we must do
		**	both pbremove's even if the first
		**	one succeeds.
		*/
		
		dbd.dbd_blkno = i;
		reg.r_type = RT_PRIVATE;
		(void) pbremove(&reg, &dbd);
		reg.r_type = RT_STEXT;
		(void) pbremove(&reg, &dbd);
	}
	memunlock();
		/*uptfree((int)vp->v_map, btos(4*nblks));*/
	uptfree((int)vp->v_map, ptos(nblks));
	vp->v_map = NULL;
}

/* <@(#)blklist.c	1.3> */
