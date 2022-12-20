#ifndef lint	/* .../sys/PAGING/os/exec.c */
#define _AC_NAME exec_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.5 87/11/11 21:24:06}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.5 of exec.c on 87/11/11 21:24:06";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)exec.c	UniPlus VVV.2.1.3	*/

#ifdef HOWFAR
extern int T_exec;
extern int T_clai;
#endif HOWFAR
#ifdef lint
#include "sys/sysinclude.h"
#include "a.out.h"
#else lint
#include "sys/types.h"
#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/reg.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/tuneable.h"

#include "a.out.h"		/* for compatiblity with old Uniplus+ a.out's */
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/wait.h"
#include "sys/pathname.h"
#include "sys/uio.h"
#endif lint

#include "sys/debug.h"


struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};


exece()
{
	register struct vnode *vp;
	extern struct vnode *gethead();
	struct vattr vattr;
	struct pathname pn;

	sysinfo.sysexec++;
	if ((vp = gethead(&vattr, &pn)) == (struct vnode *) NULL)
		return;
	execbld(vp, &vattr, &pn);
}

#define NCAPGS	btop(NCARGS + ((NCARGS + 2) * sizeof(int)))

#define GETB(V)				\
{					\
	na = &savecp[NCARGS] - cp;	\
	if (na <= 0) {			\
		up->u_error = E2BIG;	\
		goto bad;		\
	}				\
	na = bcopyin(V, cp, na);	\
	if (na == 0) {			\
		up->u_error = E2BIG;	\
		goto bad;		\
	}				\
	if (na < 0) {			\
		up->u_error = EFAULT;	\
		goto bad;		\
	}				\
}
#define GETW(V)						\
{							\
	na = wcopyin(V, cp, (NCARGS+1)*sizeof(int));	\
	if (na < 0) {					\
		up->u_error = EFAULT;			\
		goto bad;				\
	}						\
}
execbld(vp, vap, pnp)
register struct vnode *vp;
register struct vattr *vap;
register struct pathname *pnp;
{
	char *saveargs;
	uint	Savep_flag;
	register char *cp, *tcp, *savecp;
	register struct execa *uap;
	register struct user *up;
	register char *psap;
	int na, ap;
	int uid, gid;
	char **pp, *p;

	sysinfo.sysexec++;

	up = &u;
#ifdef NEW_PMMU
	availrmem -= NCAPGS;
	availsmem -= NCAPGS;

	if (availrmem < tune.t_minarmem || availsmem < tune.t_minasmem){
		up->u_error = EAGAIN;
		goto out2;
	}
#endif

	Savep_flag	= up->u_procp->p_flag;
	uid = up->u_uid;
	gid = up->u_gid;
	if ((vp->v_vfsp->vfs_flag & VFS_NOSUID) == 0) {
		if (vap->va_mode & VSUID)
			uid = vap->va_uid;
		if (vap->va_mode & VSGID)
			gid = vap->va_gid;
	} else {
		printf("%s: Setuid execution not allowed\n", pnp->pn_buf);
	}

	uap = (struct execa *)up->u_ap;
#ifdef NEW_PMMU
	while((saveargs = (char *)sptalloc(NCAPGS, PG_V|PG_RW, -1)) == NULL) {
		mapwant(sptmap)++;
		sleep(sptmap, PMEM);
	}
#else
	while((saveargs = (char *)sptalloc(NCAPGS, PG_V|PG_RW, 0)) == NULL) {
		mapwant(sptmap)++;
		sleep(sptmap, PMEM);
	}
#endif

	cp = saveargs;

	/* save room for argc */
	cp += sizeof(int);

	/* fetch argv */
	GETW(uap->argp);
	cp += na;

	/* set up argc */
	*(int *)saveargs = (cp - saveargs) / sizeof(int) - 2;

	/* fetch envp */
	GETW(uap->envp);
	cp += na;

	/* save start of args */
	savecp = cp;

	/* fetch args */
	psap = up->u_psargs;
	for (pp = (char **)(saveargs + 4); p = *pp; pp++) {
		GETB(p);
		tcp = cp + na;
		while (cp < tcp)
			if (psap < &up->u_psargs[PSARGSZ-1])
				*psap++ = *cp++;
			else
				break;
		psap[-1] = ' ';
		cp = tcp;
	}
	while (psap < &up->u_psargs[PSARGSZ])
		*psap++ = 0;

	/* fetch env */
	for (pp++; p = *pp; pp++) {
		GETB(p);
		cp += na;
	}


	/* leave int hole at end and round up to int size */
	tcp = (char *)((int)(cp + 2 * sizeof(int) - 1) & ~(sizeof(int) - 1));
/*debug TRACE(T_clai,("E4-%x-%x ",cp,tcp));*/
	while (cp < tcp)
		*cp++ = 0;

	getxfile(vp, cp - saveargs, uid, gid);
	if (up->u_error) {
		char *namep = pnp->pn_path;
		int pathlen = pnp->pn_pathlen;

		printf("exec error:  u_error %d  pn_path ", up->u_error);
		for (; namep && *namep && pathlen > 0; namep++, pathlen--)
			outchar(*namep);
		outchar('\n');
		psignal(up->u_procp, SIGKILL);
		goto bad;
	}

	/* reset argv pointers for new process */
	ap = v.v_uend - (cp - savecp);
	for (pp = (char **)saveargs + 1; *pp; pp++) {
		*pp = (char *)ap;
		na = strlen(savecp) + 1;
		ap += na;
		savecp += na;
	}

	/* reset envp pointers for new process */
	for (pp++ ; *pp; pp++) {
		*pp = (char *)ap;
		na = strlen(savecp) + 1;
		ap += na;
		savecp += na;
	}

	/* copy back arglist */
	ap = v.v_uend - (cp - saveargs);
	copyout(saveargs, ap, cp - saveargs);

	up->u_ar0[SP] = ap;
	TRACE(T_exec, ("Setting new stack pointer to 0x%x\n", ap));

	setregs(pnp);
	goto out1;
bad:
	up->u_procp->p_flag	= Savep_flag;
out1:
	sptfree((int)saveargs, NCAPGS, 1);
out2:
#ifdef NEW_PMMU
	availrmem += NCAPGS;
	availsmem += NCAPGS;
#endif NEW_PMMU
	pn_free(pnp);
	VN_RELE(vp);
	return;
}

struct vnode *
gethead(vap, pnp)
struct vattr *vap;
struct pathname *pnp;
{
	struct vnode *vp;
	register unsigned tstart;
	register struct user *up;
	register struct execa *uap;
	register int i;
	struct execfile {
		struct filehdr filehdr;
		struct aouthdr aouthdr;
	} execfile;
	register struct execfile *ep;
	register struct aouthdr *hdr;
	int resid;

	struct bhdr {
		long	fmagic;
		long	tsize;
		long	dsize;
		long	bsize;
		long	ssize;
		long	rtsize;
		long	rdsize;
		long	entry;
	};

	short	Savep_flag;
	SCNHDR sbuf;

	up = &u;
	uap = (struct execa *)u.u_ap;
	up->u_error = pn_get(uap->fname, UIOSEG_USER, pnp);
	if (up->u_error)
		return((struct vnode *)NULL);
	up->u_error = lookuppn(pnp, FOLLOW_LINK, (struct vnode **)0, &vp);
	if (up->u_error) {
		pn_free(pnp);
		return((struct vnode *)NULL);
	}
	Savep_flag	= up->u_procp->p_flag;
	if (up->u_error = VOP_GETATTR(vp, vap, up->u_cred))
		goto bad;
	/*
	 * XXX should change VOP_ACCESS to not let super user always have it
	 * for exec permission on regular files.
	 */
	if (up->u_error = VOP_ACCESS(vp, VEXEC, up->u_cred))
		goto bad;
	if ((up->u_procp->p_flag&STRC)
	    && (up->u_error = VOP_ACCESS(vp, VREAD, up->u_cred)))
		goto bad;
	if (vp->v_type != VREG ||
	    (vap->va_mode & (VEXEC|(VEXEC>>3)|(VEXEC>>6))) == 0) {
		up->u_error = EACCES;
		goto bad;
	}
	/*
	 * read in first few bytes of file for segment sizes
	 * ux_mag = 407/410/411/520/570/575
	 *  407 is plain executable
	 *  410 is RO text
	 *  411 is separated ID
	 *  520 Motorola Common object
	 *  570 Common object
	 *  575 "
	 *  set ux_tstart to start of text portion
	 */
	ep = &execfile;
	hdr = &execfile.aouthdr;
	up->u_error =
	    vn_rdwr(UIO_READ, vp, (caddr_t)ep, sizeof (*ep),
		0, UIOSEG_KERNEL, IO_UNIT, &resid);
	if (up->u_error)
		goto bad;
	if (resid)
		hdr->magic = 0;
	if (((ufhd_t *)(ep))->ux_mag == 0520) {
		up->u_error =
		    vn_rdwr(UIO_READ, vp, (caddr_t) &sbuf, SCNHSZ,
			FILHSZ + sizeof(struct aouthdr),
			UIOSEG_KERNEL, IO_UNIT, (int *) 0);
		if (up->u_error)
			goto bad;
		if ((sbuf.s_flags & STYP_TEXT) == 0) {
			up->u_error = ENOEXEC;
			goto bad;
		}

		up->u_procp->p_flag	|= SCOFF;
#ifdef	HOWFAR
		TRACE(T_exec, ("gethead:  COFF binary\n"));
#endif

		i = hdr->text_start - (hdr->text_start & ~POFFMASK);
		if (i) {
			hdr->tsize += i;
			hdr->text_start -= i;
			tstart = 0;
		} else 
			tstart = sbuf.s_scnptr;
	} else {
		up->u_procp->p_flag	&= ~SCOFF;
		/* reverse aouthdr assignments to avoid trashing bhdr */
#ifdef	HOWFAR
		TRACE(T_exec, ("gethead:  UniPlus+ binary\n"));
#endif
		((ufhd_t *)(hdr))->ux_datorg = ((struct bhdr *) ep)->entry +
			((struct bhdr *) ep)->tsize;
		((ufhd_t *)(hdr))->ux_txtorg = v.v_ustart;
		((ufhd_t *)(hdr))->ux_entloc = ((struct bhdr *) ep)->entry;
		((ufhd_t *)(hdr))->ux_bsize = ((struct bhdr *) ep)->bsize;
		((ufhd_t *)(hdr))->ux_dsize = ((struct bhdr *) ep)->dsize;
		((ufhd_t *)(hdr))->ux_tsize = ((struct bhdr *) ep)->tsize;
		((ufhd_t *)(hdr))->ux_mag = ((struct bhdr *) ep)->fmagic;
		if (hdr->magic == 0410)
			((ufhd_t *)(hdr))->ux_datorg = 
				(((ufhd_t *)(hdr))->ux_txtorg +
				 ((ufhd_t *)(hdr))->ux_tsize + v.v_txtrnd - 1)
					& (-v.v_txtrnd);
		tstart = sizeof(struct bhdr);
	}


	/*	407 is RW nonshared text
	 *	410 is RO shared text
	 *	413 is demand fill RO shared text.
	 */

	if (hdr->magic == 0410  || hdr->magic == 0413) {
		/*
		 * Check to make sure nobody is modifying the text right now
		 */
		if ((vp->v_flag & VTEXTMOD) != 0) {
			up->u_error  = ETXTBSY;
			goto bad;
		}
		if (((vp->v_flag & VTEXT) == 0) && vp->v_count != 1) {
			register struct file *fp;

			for (fp = file; fp < (struct file *)v.ve_file; fp++)
				if (fp->f_type == DTYPE_VNODE &&
				    fp->f_count > 0 &&
				    (struct vnode *)fp->f_data == vp &&
				    (fp->f_flag & FWRITE)) {
					up->u_error = ETXTBSY;
					goto bad;
				}
		}

		vp->v_flag |= VTEXT;
	} else if (hdr->magic == 0407) {
		hdr->dsize += hdr->tsize;
		hdr->tsize = 0;
		hdr->data_start = hdr->text_start;

		/*	The following is needed to prevent
		**	chksize from failing certain 407's.
		*/

		hdr->text_start = 0;
	} else {
		up->u_error = ENOEXEC;
		goto bad;
	}
	chksize(hdr);
bad:
	if (up->u_error) {
		if (vp->v_count == 1)
			vp->v_flag &= ~VTEXT;
		pn_free(pnp);
		VN_RELE(vp);
		vp = NULL;
		up->u_procp->p_flag	= Savep_flag;
	} else {
		up->u_exdata = *((ufhd_t *) hdr);
		up->u_exdata.ux_tstart = tstart;
	}
	return(vp);
}

chksize(hdr)
register struct aouthdr *hdr;
{

	/*	Check that the text, data, and stack segments
	 *	are all non-overlapping.
	 */

	if ((hdr->text_start + hdr->tsize) > (hdr->data_start & ~SOFFMASK)  ||
	   (hdr->data_start + hdr->dsize + hdr->bsize) > (unsigned) USRSTACK) {
		u.u_error = ENOMEM;
		return;
	}

	if (btop(hdr->tsize) + btop(hdr->dsize + hdr->bsize) + NCAPGS >
	   tune.t_maxumem)
		u.u_error = ENOMEM;
}

/*
 * Read in and set up memory for executed file.
 */
getxfile(vp, nargc, uid, gid)
	register struct vnode *vp;
	int nargc, uid, gid;
{
	register	size, npgs, base;
	register reg_t	*rp;
	register preg_t	*prp;
	register struct user *up;
	struct proc	*p;
	int		rgva;
	int		offset;
	int		(**fptr)();
	extern int	(*execfunc[])();
	int		execself;

	up = &u;
	for (fptr = execfunc; *fptr; fptr++)
		(**fptr)(up);
	up->u_prof.pr_scale = 0;
	p = up->u_procp;

	/*	We must check for the special case of a process
	**	exec'ing itself.  In this case, we skip
	**	detaching the regions and then attaching them
	**	again because it causes deadlock problems.
	*/

	prp = findpreg(p, PT_TEXT);

	if (prp  &&  prp->p_reg->r_vptr == vp)
		execself = 1;
	else {
		execself = 0;

		/*	We must unlock the vnode for the
		**	file we are about to execute before
		**	detaching the regions of the current
		**	file.  If we don't, we could get an
		**	a-b, b-a deadlock problem.
		*/

		vp->v_flag |= VTEXT;
	}

	/*	Loop through all of the regions but
	**	handle the text and data specially
	**	if we are exec'ing ourselves.
	*/

	prp = p->p_region;
	while(rp = prp->p_reg) {

		/*	Just skip the text region if
		**	we are exec'ing ourselves.
		*/

		if (execself  &&  prp->p_type == PT_TEXT) {
			prp++;
			continue;
		}

		/*	If we are a exec'ing ourselves, then
		**	give up all of the data space since
		**	it may have been modified.  We cannot
		**	detach from the data region because
		**	this will unlock the vnode.
		*/

		if (execself  &&  prp->p_type == PT_DATA) {

			/*	Give up all of the data space since
			**	it may have been modified.  We cannot
			**	detach from the data region because
			**	this will unlock the vnode.
			*/

			reglock(rp);
			(void) growreg(prp, - rp->r_pgsz, DBD_DFILL);
			regrele(rp);
			prp++;
			continue;
		}
		reglock(rp);
		detachreg(prp, &u);
	}

	bzero((caddr_t) u.u_phys, v.v_phys * sizeof(struct phys));
	
	if (!execself) {
		if (up->u_exdata.ux_mag == 0407)
			vp->v_flag &= ~VTEXT;
	}

	clratb(USRATB);

	offset = up->u_exdata.ux_tstart + up->u_exdata.ux_tsize;

	/*	Load text region.  Note that if xalloc
	**	returns an error, then it has already
	**	done an pn_free.
	**/

	if (!execself)
		if (up->u_error = xalloc(vp))
			goto out;

	/*	Allocate the data region.
	 */


	base = up->u_exdata.ux_datorg;
	size = up->u_exdata.ux_dsize;
	rgva = base & ~SOFFMASK;

	if (execself) {
		prp = findpreg(p, PT_DATA);
		rp = prp->p_reg;
		reglock(rp);
	} else if ((rp = allocreg(vp, RT_PRIVATE)) == NULL) {
		goto out;
	}

	/*	Attach the data region to this process.
	 */
	
	if (!execself  &&  
	   (prp = attachreg(rp, &u,  (caddr_t)rgva, PT_DATA, SEG_RW)) == NULL) {
		freereg(rp);
		goto out;
	}

	/*
	 * Load data region
	 */

	if (size) {
		if (up->u_exdata.ux_mag == 0413) {
			if (mapreg(prp, (caddr_t)base, vp, offset, size) < 0) {
				detachreg(prp, &u);
				goto out;
			}
		} else {
			if (loadreg(prp, (caddr_t)base, vp, offset, size) < 0) {
				detachreg(prp, &u);
				goto out;
			}
		}
#if ! defined(lint) && ! defined(NEW_PMMU)
	/* "pointer alignment problem" when OSDEBUG turned on */
		ASSERT(rp->r_list[0] >= (pte_t *)uptbase);
#endif lint
	}

	/*
	 * Allocate bss as demand zero
	 */
	npgs = btop(base + size + up->u_exdata.ux_bsize) - btop(base + size);
	if (npgs) {
		if (growreg(prp, npgs, DBD_DZERO) < 0) {
			detachreg(prp, &u);
			goto out;
		}
#if ! defined(lint) && ! defined(NEW_PMMU)
	/* "pointer alignment problem" when OSDEBUG turned on */
		ASSERT(rp->r_list[0] >= (pte_t *)uptbase);
#endif lint
	}
	regrele(rp);

	/*	Allocate a region for the stack and attach it to
	 *	the process.
	 */

	if ((rp = allocreg((struct vnode *)NULL, RT_PRIVATE)) == NULL)
		goto out;

	if ((prp = attachreg(rp, &u, USRSTACK, PT_STACK, SEG_RW)) == NULL) {
		freereg(rp);
		goto out;
	}
	
	/*	Grow the stack but don't actually allocate
	 *	any pages.
	 */
	
	npgs = SSIZE + btop(nargc);
	if (growreg(prp, npgs, DBD_DZERO) < 0) {
		detachreg(prp, &u);
		goto out;
	}
	regrele(rp);

	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((p->p_flag&STRC)==0) {
		if (uid != up->u_uid || gid != up->u_gid)
			up->u_cred = crcopy(up->u_cred);
		up->u_uid = uid;
		up->u_gid = gid;
		p->p_suid = up->u_uid;
	} else
		psignal(up->u_procp, SIGTRAP);

	return;

out:
	/*	We get here only for an error.  The vnode
	**	ip is unlocked.  We may have regions attached
	**	which we must detach.  Note that we again
	**	rely on the compacting of detachreg.
	**/

	prp = p->p_region;
	while(rp = prp->p_reg) {
		reglock(rp);
		detachreg(prp, &u);
	}

	up->u_error = ENOEXEC;
}

/* <@(#)exec.c	1.5> */
