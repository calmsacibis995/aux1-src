#ifndef lint	/* .../sys/COMMON/os/cmachdep.c */
#define _AC_NAME cmachdep_c
#define _AC_NO_MAIN "%Z% Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version %I% %E% %U%}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/* 	%Z%Copyright Apple Computer 1987	Version %I% of %M% on %E% %U% */

/*	@(#)cmachdep.c UniPlus VVV.2.1.7	*/

#ifdef HOWFAR
extern int T_sendsig;
#endif HOWFAR

#include "sys/types.h"
#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/reg.h"
#include "sys/acct.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/vnode.h"
#include "sys/debug.h"

#include "sys/var.h"
#include "sys/psl.h"
#include "sys/pmmu.h"
#include "sys/buserr.h"

#include "sys/pathname.h"
#include "sys/uio.h"
#include "sys/trace.h"
#include "sys/clock.h"
#include "compat.h"

static	badstack();

static struct sigframe {
	char 	*sf_retaddr;		/* 0x0: return address from handler */
	int	sf_signum;		/* 0x4 */
	int	sf_code;		/* 0x8 */
	struct sigcontext *sf_scp;	/* 0xC */
	int	sf_kind;		/* 0x10: u_traptype */
	int	sf_regs[4];		/* 0x14: saved user registers */
	char	sf_retasm[8];		/* 0x24: return to kernel */
};

/*	This code is processor and assembler specific.  It is copied onto
 *	the user stack.  After a user signal handler exits, this code is
 *	executed.
 */
#ifndef	lint
extern  char	siglude[];
	asm("siglude:");		
	asm("	mov.l	&150,%d0");	/* system call number for sysreturn */
	asm("	trap &15");		/* make a system call */
#else	lint
	char	siglude[1];
#endif	lint

/*
 * Send a signal to a process - simulate an interrupt.
 */
/*ARGSUSED*/
sendsig(hdlr, signo, arg)
	int	(*hdlr) ();
{
	register int *regs;
	register struct user *up;
	register struct proc *p;
	struct sigcontext lscp, *uscp;
	struct sigframe lfp, *ufp;
	int	oonstack;

#ifdef	HOWFAR
	extern char	*signame();

	TRACE(T_sendsig,
		("<%x,%s,%x> ", hdlr, signame(signo), arg));
#endif	HOWFAR
	up = &u;
	p = up->u_procp;
	regs = up->u_ar0;
	if(signo == SIGSEGV || signo == SIGBUS) {
		up->u_traptype &= ~(TRAPBUS | TRAPADDR);
		if(signo == SIGBUS)	/* will never restart ADDRERR */
			up->u_traptype &= ~TRAPTMASK;
	}
    if ((p->p_compatflags & COMPAT_BSDSIGNALS) == 0) {
	if (p->p_flag & SCOFF) {
		/*
		 *	We infer from the fact that the binary was in
		 *	Common Object File Format that the signal interface
		 *	is the new one.
		 */
		/* We need to save more state.  Floating point and
		 * bus error info are now saved in the user space.
		 * The precise form is subject to change.  In general, there
		 * may be a varying number of bytes of state info between
		 * the user's old sp, and the signal interface frame known
		 * to application routines.  If traptype is nonzero, then
		 * the kernel "must" be called to clean this info.
		 */     
		register int	*usp;

		if((usp = (int *)copyframe(YES, regs, (caddr_t)regs[SP])) == 0)
			return;
		(void) grow((unsigned) (usp - 6));

		/* New rule for COFF binaries:
		 * If traptype is nonzero, the old user stack pointer is
		 * pushed below the state info.  Custom user signal handlers 
		 * may take advantage of this to pop off the state info.
		 */
		if(up->u_traptype)
			(void) suword((caddr_t) --usp, regs[SP]);
			
		/* The following code is the "traditional" Coff format */
		/* simulate an interrupt on the user's stack */
		(void) suword((caddr_t) --usp, regs[PC]);
		(void) suword((caddr_t) --usp,
			(regs[RPS] & 0xffff) | (up->u_traptype << 16));
		(void) suword((caddr_t) --usp, arg);
		(void) suword((caddr_t) --usp, signo);
		regs[SP]	= (int) usp;
	} else {
		register unsigned long	usp;
		short	ps;

		/* When the old interface is used, return will be via
		 * rts, there is no way to properly handle bus errors,
		 * or save fp info.  Users will have to recompile for these
		 * features.
		 */
		usp	= regs[SP] - 6;
		ps	= (short) regs[RPS];
		(void) grow((unsigned) (usp - 3));
		(void) subyte((caddr_t) usp, ps >> 8);	/* high byte of ps */
		(void) subyte((caddr_t) (usp + 1), ps); /* low byte of ps */
		(void) suword((caddr_t) (usp + 2), regs[PC]);
		regs[SP]	= (int) usp;
		regs[RPS]	&= ~PS_T;
	}
	regs[PC]	= (int) hdlr;
	if ((cputype == 68020) || (cputype == 68010)) {	 
		/* 
		 *	make sure looks like short format - clai 
		 *	020 still needs to store actual format somewhere - trip
		 *	ah - but what about type 2 stack frames???? - paul
		 *	Type 2 "stack creep" handled in mch.s - RZ
		 */
		((struct buserr *)regs)->ber_format = 0;
		up->u_traptype = 0;
	}
    } else {
	oonstack = up->u_onstack;
	if (!up->u_onstack && (up->u_sigonstack & sigmask(signo))) {
		uscp = (struct sigcontext *)copyframe(YES, regs, up->u_sigsp);
		up->u_onstack = 1;
	} else
		uscp = (struct sigcontext *)copyframe(YES, regs, regs[SP]);
	if(uscp-- == 0)
		return;
	ufp = (struct sigframe *)uscp - 1;
	/*
	 * Must build signal handler context on stack to be returned to
	 * so that rei instruction in sigcode will pop ps and pc
	 * off correct stack.  The remainder of the signal state
	 * used in calling the handler must be placed on the stack
	 * on which the handler is to operate so that the calls
	 * in sigcode will save the registers and such correctly.
	 */
	if (!up->u_onstack)
		grow((unsigned)ufp);
	lfp.sf_signum = signo;
	if (signo == SIGILL || signo == SIGFPE) {
#ifdef NOTDEFASA
THIS IS AN ATTEMPT TO PASS THE FPE CODE TO THE USER
NO ONE EVER SETS IT UP
probably psig should put its arg in u_code
for 4.3 signals
and someone later should restore it?
u_code does not exist currently in the swap user.h
		lfp.sf_code = up->u_code;
		up->u_code = 0;
#endif NOTDEFASA
	} else
		lfp.sf_code = 0;
	lfp.sf_scp = uscp;
	lfp.sf_regs[0] = regs[R0];
	lfp.sf_regs[1] = regs[R1];
	lfp.sf_regs[2] = regs[AR0];
	lfp.sf_regs[3] = regs[AR1];
	lfp.sf_retaddr = ufp->sf_retasm;
	bcopy(siglude, lfp.sf_retasm, sizeof(lfp.sf_retasm));
	lfp.sf_kind = up->u_traptype;

	/* sigcontext goes on previous stack */
	lscp.sc_onstack = oonstack;
	lscp.sc_mask = arg;
	/* setup rei */
	lscp.sc_sp = regs[SP];
	lscp.sc_pc = regs[PC];
	lscp.sc_ps = regs[RPS];
	regs[SP] = (int)ufp;
	regs[RPS] &= ~PS_T;
	regs[PC] = (int)hdlr;
	if (copyout(&lfp, ufp, sizeof(lfp)))
		goto bad;
	if (copyout(&lscp, uscp, sizeof(lscp)))
		goto bad;
	if ((cputype == 68020) || (cputype == 68010)) {
		((struct buserr *)regs)->ber_format = 0;
		up->u_traptype = 0;
	}
	return;

bad:
	badstack();
	return;
    }
}

/*	copyframe -- copy CPU state to user area.
 *	    Relying on the u_traptype word, this routine will copyin
 *	or copyout the CPU specific bus error information to the user stack.
 *	The format is:
 *		checksum prior to encryption
 *		stack info x'ord with address of proc struct
 *		floating point info, if any
 *	returns the new value for the user stack pointer.
 */

copyframe(out, regs, usp)

int	out;	/* copyout if true, else copyin */
int	*regs;	/* kernel stack regs structure */
caddr_t	usp;	/* user's stack pointer */
{
	int	i;
	register u_char *data;
	register u_char *cp;
	register n;
	int	checksum;

	/* Determine number of bytes of status to copy to/from user space */
	TRACE(T_sendsig, ("copyframe %s kern = 0x%x usp = 0x%x\n", 
		out ? "out" : "in", regs, usp));
#ifdef	mc68881
	if((u.u_traptype & TRAPSAVFP) && !out) {
		if((usp = (caddr_t)fp_copyin((unsigned)usp)) == 0) {
			badstack();
			return(0);
		}
	}
#endif	mc68881
	switch(u.u_traptype & TRAPTMASK) {
	case TRAPLONG:
		ASSERT(out ? ((struct buserr *)regs)->ber_format == FMT_LONG : 1);
		ASSERT(out ? (((struct buserr *)regs)->ber_sstat & 0x4) == 0
			: 1);
		i = 92 - 2;
		break;
	case TRAPSHORT:
		ASSERT(out ? ((struct buserr *)regs)->ber_format == FMT_SHORT : 1);
		ASSERT(out ? (((struct buserr *)regs)->ber_sstat & 0x4) == 0
			: 1);
		i = 32 - 2;
		break;
	case TRAPLONG10:
		ASSERT(out ? ((struct buserr *)regs)->ber_format == FMT_LONG10 : 1);
		ASSERT(out ? (((struct buserr *)regs)->ber_sstat & 0x4) == 0
			: 1);
		i = 58 - 2;
		break;
	default:
		i = 0;
		break;
	}
	data = (u_char *) &regs[PC];
	ASSERT(((int)data & 1) == 0 && (i & 1) == 0);	/* All even */
	if(i) {
		if(!out) {
			register short *sp;

			checksum = fuword(usp);
			TRACE(T_sendsig, ("copyin frame from 0x%x to 0x%x\n",
				usp+4, data));
			if(copyin(usp+4, data, i)) {
				badstack();
				return(0);
			}
			n = (int)u.u_procp;
			for(sp = (short *)(&data[i]); sp >= (short *)data; --sp) {
				*sp ^= n;
			}

		}
		/* caluclate checksum */
		n = 0;
		for(cp = (u_char *)(&data[i]); cp >= data; --cp) {
			n += *cp;
		}
		if(out) {
			register short *sp;
			register short x;

			x = (short)u.u_procp;
			for(sp = (short *)(&data[i]); sp >= (short *)data; --sp) {
				*sp ^= x;
			}
			(void)grow((unsigned)usp - i - 4);
			if(copyout(data, usp - i, i)
			  || suword(usp - i - 4, n) < 0) {
				sp = (short *)data;
				*sp++ ^= x;
				*sp ^= x;
				badstack();
				return(0);
			}
			sp = (short *)data;
			*sp++ ^= x;
			*sp ^= x;
			usp -= i + 4;
			TRACE(T_sendsig, ("copyout frame from 0x%x to 0x%x\n",
				data, usp));
		}
		else {
			if((u_short)n != checksum) {
				badstack();
				return(0);
			}
			switch(u.u_traptype & TRAPTMASK) {
			case TRAPLONG:
			case TRAPSHORT:
			case TRAPLONG10:
				if(((struct buserr *)regs)->ber_sstat & 0x4) {
					badstack();
					return(0);
				}
				switch(((struct buserr *)regs)->ber_format) {
				case FMT_LONG:
				case FMT_LONG10:
				case FMT_SHORT:
					break;
				default:
					badstack();
					return;
				}
				break;
			default:
				badstack();
				return(0);
			}
			usp += i + 4;
			u.u_traptype |= TRAPREST;
		}
	}
#ifdef	mc68881
	if(out) {
		if((usp = (caddr_t)fp_copyout((unsigned)usp)) == 0) {
			badstack();
			return(0);
		}
	}
#endif	mc68881
	return((int)usp);
}

/*	badstack -- kill off a process with a bad stack.
 *	Process has trashed its stack; give it an illegal
 *	instruction to halt it in its tracks.
 */

static
badstack()
{
	int	signo;
	register struct user *up;
	register struct proc *p;

	TRACE(T_sendsig, ("badstack called from 0x%x\n", caller()));
	up = &u;
	p = up->u_procp;
	((struct buserr *) up->u_ar0)->ber_format = 0;
#ifdef	mc68881
	fpnull();
#endif	mc68881
	up->u_traptype = 0;
	up->u_signal[SIGILL-1] = SIG_DFL;
	signo = sigmask(SIGILL);
	p->p_sigignore &= ~signo;
	p->p_sigcatch &= ~signo;
	p->p_sigmask &= ~signo;
	psignal(p, SIGILL);
}

/*
 * Routine to cleanup state after a signal
 * has been taken.  Reset signal mask and
 * stack state from context left by sendsig (above).
 * Pop these values in preparation for rei which
 * follows return from this routine.
 */
sigcleanup()
{
	struct sigframe lfp;
	struct sigcontext lscp;
	register struct user *up;
	register int	*regs;

	up = &u;
	regs = up->u_ar0;
	if ( (up->u_procp->p_compatflags & COMPAT_BSDSIGNALS) == 0)
		return(EINVAL);
	if(copyin(regs[SP]-sizeof(lfp.sf_retaddr), &lfp, 
				sizeof(lfp)-sizeof(lfp.sf_retasm))) {
		badstack();
		return(EFAULT);
	}
	if (copyin(lfp.sf_scp, &lscp, sizeof(lscp))) {
		badstack();
		return(EFAULT);
	}
	up->u_onstack = lscp.sc_onstack & 01;
	up->u_procp->p_sigmask =
	    lscp.sc_mask &~ (sigmask(SIGKILL)|sigmask(SIGCONT)|sigmask(SIGSTOP));
	regs[R0] = lfp.sf_regs[0];
	regs[R1] = lfp.sf_regs[1];
	regs[AR0] = lfp.sf_regs[2];
	regs[AR1] = lfp.sf_regs[3];
	regs[SP] = lscp.sc_sp;
	regs[PC] = lscp.sc_pc;
	regs[RPS] = (lscp.sc_ps & 0xFF) | (regs[RPS] & 0xFF00);
	up->u_traptype = lfp.sf_kind;
	(void)copyframe(NO, regs, (caddr_t) (lfp.sf_scp+1));
	if(lscp.sc_pc != regs[PC]) {
		regs[PC] = lscp.sc_pc;
		((struct buserr *)regs)->ber_format = 0;
	}
	if (up->u_traptype & TRAPADDR) /* addr error */
		psignal(up->u_procp, SIGBUS);
	else if (up->u_traptype & TRAPBUS) /* bus error */
		psignal(up->u_procp, SIGSEGV);
	return (0);
}

/*
 * ovbcopy(f, t, l)
 *	copy from one buffer to another that are possibly overlapped
 *	this is like bcopy except that f and t may be overlapped in
 *	any manner, so you may need to copy from the rear rather than
 *	from the front
 * parameters
 *	char *f;	from address
 *	char *t;	to address
 *	int l;		length
 */
ovbcopy(f, t, l)
register char *f, *t;
register int l;
{
	if (f != t)
		if (f < t) {
			f += l;
			t += l;
			while (l-- > 0)
				*--t = *--f;
		} else {
			while (l-- > 0)
				*t++ = *f++;
		}
}
