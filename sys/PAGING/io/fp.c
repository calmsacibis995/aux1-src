#ifndef lint	/* .../sys/PAGING/io/fp.c */
#define _AC_NAME fp_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 21:22:50}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of fp.c on 87/11/11 21:22:50";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fp.c	UniPlus VVV.2.1.2	*/

/*
 * MC68881 Floating-Point Coprocessor
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#ifdef lint
#include <sys/sysinclude.h>
#include <sys/fpioctl.h>
#else lint
#include <sys/param.h>
#include <sys/uconfig.h>
#include <sys/types.h>
#include <sys/mmu.h>
#include <sys/seg.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/buf.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/uioctl.h>
#include <sys/fpioctl.h>
#endif lint

/*ARGSUSED*/
fpioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	register struct user *up;

	up = &u;
	switch (cmd) {
#ifdef mc68881		/* MC68881 floating-point coprocessor */
		case FPIOCEXC:	/* return reason for last 881 SIGFPE */
			if (fubyte(addr) == -1) {
				up->u_error = EFAULT;
				return;
			}
			/* u_fpexc is saved in trap.c */
			if (subyte(addr, up->u_fpexc) == -1) {
				up->u_error = EFAULT;
				return;
			}
			break;
#endif mc68881
		default:
			up->u_error = EINVAL;
	}
}

#ifdef mc68881		/* MC68881 floating-point coprocessor */
/*
 *	Save the internal state and programmer's model of the 68881.
 *	This is called from psig(), core(), swtch(), or procdup().
 */
fpsave()
{
	/* warning: asm calls depend on the order of declaration */
	register char *reg;		/* a2 */
	register char *istate;		/* a3 */
	extern short fp881;		/* is there an MC68881? */

	if (fp881 && u.u_fpsaved == 0) {
		istate = &u.u_fpstate[0];
		/* halt processing and save internal state of coprocessor */
		asm("	short	0xf313		# fsave (a3)");
		/* check for non-null saved state (version # byte) */
		if (*((short *)istate)) {
			/* save system registers CONTROL/STATUS/IADDR */
			reg = (char *)&u.u_fpsysreg[0];
			asm("	short	0xf212		# fmovem (a2)");
			asm("	short	0xbc00		# save system regs");
			/* save data registers FP0,FP1,...,FP7 */
			reg = &u.u_fpdreg[0][0];
			asm("	short	0xf212		# fmovem (a2)");
			asm("	short	0xf0ff		# save data registers");
#ifdef lint
			*reg = *istate;
#endif lint
		}
		u.u_fpsaved = 1;
	}
}

/*
 *	Restore the internal state and programmer's model of the 68881.
 *	This is called from trap() or syscall().
 */
fprest()
{
	/* warning: asm calls depend on the order of declaration */
	register char *reg;		/* a2 */
	register char *istate;		/* a3 */

	istate = &u.u_fpstate[0];
	/* check for non-null saved state (version # byte) */
	if (*((short *)istate)) {
		/* restore system registers CONTROL/STATUS/IADDR */
		reg = (char *)&u.u_fpsysreg[0];
		asm("	short	0xf212		# fmovem (a2)");
		asm("	short	0x9c00		# restore system registers");
		/* restore data registers FP0,FP1,...,FP7 */
		reg = &u.u_fpdreg[0][0];
		asm("	short	0xf212		# fmovem (a2)");
		asm("	short	0xd0ff		# restore data registers");
#ifdef lint
		*reg = *istate;
#endif lint
	}
	/* load internal state of coprocessor */
	asm("	short	0xf353		# frestore (a3)");
	u.u_fpsaved = 0;
}

/*	fp_copyout -- copy floating point status.
 *	    The floating point state, if any, is saved to the user stack
 *	area.  The current format is:
 *	4 byte	checksum
 *	var sz	FPU state info
 *	24 byte	fp0 fp1
 *	12 byte	system and status registers
 *
 *	If information is saved, the u_traptype word is set to reflect what's
 *	saved.  u_traptype will later be used to indicate what needs to
 *	be restored.  By intent, only code in this file knows the precise
 *	contents of the save area.  Since none of the save area is publicly
 *	known, future enhancements need not be tied in any way to this
 *	precise format.
 */

fp_copyout(usp)

unsigned usp;
{
	struct stateinfo {
	u_char ver;
	u_char size;
	short	rsvd;
	} *sp;
	register struct user *up;
	register u_char *cp;
	register sum, size;

	up = &u;
	sp = (struct stateinfo *) up->u_fpstate;
	if((size = sp->size) == 0) {
		up->u_traptype &= ~TRAPSAVFP;
		return((int)usp);
	}
	size += sizeof(int);	/* include format word */
	grow(usp - size - sizeof(up->u_fpsysreg) - 2 * FPDSZ - sizeof(int));
	sum = 0;
	for(cp = &((u_char *)up->u_fpstate)[size-1]; 
				cp >= (u_char *)u.u_fpstate; --cp)
		sum += *cp;
	if(copyout(up->u_fpsysreg, usp -= sizeof(up->u_fpsysreg), sizeof(up->u_fpsysreg))
	  || copyout(up->u_fpdreg, usp -= 2 * FPDSZ, 2 * FPDSZ)
	  || copyout(up->u_fpstate, usp -= size, size)
	  || suword(usp -= sizeof(int), sum) < 0) {
		return(0);
	}
	up->u_traptype |= TRAPSAVFP;
	return((int)usp);
}

/*	fp_copyin -- copyin floating point state from user stack.
 *	     fp_copyout saved floating point information, and set
 *	the u_traptype word to reflect what had been saved.  Now,
 *	we copy the floating point state info back from user space.
 */

fp_copyin(usp)

register unsigned usp;
{
	int state;
	register struct user *up;
	register u_char *cp;
	register sum, size;
	int	checksum;

	up = &u;
	up->u_fpsaved = 1;
	*((int *)up->u_fpstate) = 0;
	/* There is a chance that a page in will occur in the copyins.
	 * In this case we may be rescheduled, and the undefined floating 
	 * point registers will be restored to the chip.  Nulling out the 
	 * state should handle this.
	 */
	checksum = fuword(usp);
	usp += sizeof(int);
	if((state = fuword(usp)) == -1)
		return(0);
	usp += sizeof(int);
	size = (state >> 16) & 0xFF;
	if(size > sizeof(u.u_fpstate) - sizeof(int))
		return(0);
	if(copyin(usp, (caddr_t)u.u_fpstate + sizeof(int), size))
		return(0);
	usp += size;
	size += sizeof(int);	/* 1st word of frame not included in count */
	if(copyin(usp, up->u_fpdreg, FPDSZ * 2)
	  || copyin(usp += FPDSZ * 2, up->u_fpsysreg, sizeof(up->u_fpsysreg)))
		return(0);
	usp += sizeof(up->u_fpsysreg);
	*((int *)up->u_fpstate) = state;
	sum = 0;
	for(cp = &((u_char *)up->u_fpstate)[size-1]; 
				cp >= (u_char *)u.u_fpstate; --cp)
		sum += *cp;
	if(checksum != sum) {
		return(0);
	}
	up->u_fpsaved = 1;
	up->u_traptype &= ~TRAPSAVFP;
	return(usp);
}


/*	fpnull -- zero floating point system.
 *	     All state variables, and all register values are made null.
 *	If a coprocessor format error has occured, this routine must recover
 *	from it.
 */

fpnull()

{
	*((int *)u.u_fpstate) = 0;
	u.u_fpsaved = 1;
}


fp_status()
{
	asm("	short	0xf200		# fmove STATUS,d0");
	asm("	short	0xa800");
}
#endif mc68881
