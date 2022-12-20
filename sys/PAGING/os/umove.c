#ifndef lint	/* .../sys/PAGING/os/umove.c */
#define _AC_NAME umove_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:28:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of umove.c on 87/11/11 21:28:24";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)umove.c	UniPlus VVV.2.1.8	*/

#ifdef HOWFAR
extern int	T_subyte;
#endif HOWFAR

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "setjmp.h"
#include "sys/pmmu.h"
#include "sys/trace.h"
#include "sys/var.h"
#endif lint

/* This file "os/umove.c" corresponds with the 3B20's file "ml/move.s". */

extern pte_t *copyptbl;
extern pte_t *copypte;
pte_t *phystosvir();

/*
 * fuword - Fetch word from user space.
 */
fuword(addr)
caddr_t addr;
{
	int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;
	int tmp;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(int)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fuword: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(int)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fuword: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		val = *(int *)(addr+USEROFF);
#else	UMVSEG
		tmp = soff(addr) - stob(1) + 4;
		if (tmp > 0) {
			switch(tmp) {
			case 1:
				val = (*(short *)(stob(UMOVESEG) + soff(addr)) 
					 << 16) & 0xffff0000;
				val |= (*(char *)(stob(UMOVESEG) + soff(addr)+2)
					 << 8) & 0xff00;
				break;
			case 2:
				val = ((*(short *)(stob(UMOVESEG) + soff(addr)))
						<< 16) & 0xffff0000;
				break;
			case 3:
				val = (*(char *)(stob(UMOVESEG) + soff(addr)) 
					 << 24) & 0xff000000;
				break;
			}
			u_ste = (int)steaddr(u.u_stbl[snum(addr)+1]);
			wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
			clratb(SYSATB);
			switch(tmp) {
			case 1:
				val |= (*(char *)(stob(UMOVESEG))) & 0xFF;
				break;
			case 2:
				val |= (*(short *)(stob(UMOVESEG))) & 0xFFFF;
				break;
			case 3:
				val |= (*(short *)(stob(UMOVESEG)) << 8) &
					0xFFFF00;
				val |= (*(char *)(stob(UMOVESEG) + 2)) & 0xFF;
				break;
			}
		} else
			val = (*(int *)(stob(UMOVESEG) + soff(addr)));
#endif	UMVSEG
	} else
		val = -1;
	nofault = saved_jb;
#ifdef	UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * fubyte - Fetch byte from user space.
 */
fubyte(addr)
register caddr_t addr;
{
	register unsigned int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fubyte: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fubyte: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		val = *(char *)(addr+USEROFF) & 0xFF;
#else	UMVSEG
		val = (*(char *)(stob(UMOVESEG) + soff(addr)))&0xFF;
#endif	UMVSEG
	} else
		val = -1;
	nofault = saved_jb;
#ifdef UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * suword - Store word into user space.
 */
suword(addr, word)
caddr_t addr;
int word;
{
	int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(word)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("suword: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(word)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("suword: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		*(int *)(addr+USEROFF) = word;
#else	UMVSEG
		val = soff(addr) - stob(1) + 4;
		if (val > 0) {
			switch(val) {
			case 1:
				*(short *)(stob(UMOVESEG) + soff(addr)) =
					word >> 16;
				*(char *)(stob(UMOVESEG) + soff(addr) + 2) =
					word >> 8;
				break;
			case 2:
				*(short *)(stob(UMOVESEG) + soff(addr)) =
					word >> 16;
				break;
			case 3:
				*(char *)(stob(UMOVESEG) + soff(addr)) =
					word >> 24;
				break;
			}
			u_ste = (int)steaddr(u.u_stbl[snum(addr)+1]);
			wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
			clratb(SYSATB);
			switch(val) {
			case 1:
				*(char *)(stob(UMOVESEG)) = word;
				break;
			case 2:
				*(short *)(stob(UMOVESEG)) = word;
				break;
			case 3:
				*(short *)(stob(UMOVESEG)) = word >> 8;
				*(char *)(stob(UMOVESEG) + 2) = word;
				break;
			}
		} else
			*(int *)(stob(UMOVESEG) + soff(addr)) = word;
#endif UMVSEG
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
#ifdef UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * subyte - Store byte into user space.
 */
subyte(addr, byte)
register caddr_t addr;
char byte;
{
	int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("subyte: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("subyte: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		*(char *)(addr+USEROFF) = byte & 0xFF;
#else UMVSEG
		*(char *)(stob(UMOVESEG) + soff(addr)) = byte&0xFF;
#endif UMVSEG
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
#ifdef UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * copyout - Move bytes out of the system into user's address space.
 */
copyout(from, to, nbytes)
register caddr_t from, to;
unsigned nbytes;
{
	int val = 0;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;

	if (to < (caddr_t)v.v_ustart ||
	    (to+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyout: to address 0x%x failed\n",to));
		return(EFAULT);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	while (nbytes != 0) {
		u_faddr = (int)to;
		u_ste = (int)steaddr(u.u_stbl[snum(to)]);
TRACE(T_subyte,("copyout: wtste into kstbl[UMOVESEG] = 0x%x\n",u_ste));
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)to & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			blt((caddr_t)(stob(UMOVESEG) + soff(to)), from, count);
		} else {
			val = EFAULT;
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
		from + = count;
		to + = count;
	}
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else	UMVSEG
	if (to < (caddr_t)v.v_ustart ||
	    (to+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyout: to address 0x%x failed\n",to));
		return(EFAULT);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		blt((caddr_t)(to+USEROFF), from, nbytes);
	} else {
		val = EFAULT;
	}
	nofault = saved_jb;
#endif	UMVSEG
	return(val);
}

/*
 * copyin - Move bytes into the system space out of user's address space.
 */
copyin(from, to, nbytes)
register caddr_t from, to;
unsigned nbytes;
{
	int val = 0;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;

	if (from < (caddr_t)v.v_ustart ||
	    (from+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyin: to address 0x%x failed\n",from));
		return(EFAULT);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	while (nbytes != 0) {
		u_faddr = (int)from;
		u_ste = (int)steaddr(u.u_stbl[snum(from)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)from & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			blt(to, (caddr_t)(stob(UMOVESEG) + soff(from)), count);
		} else {
			val = EFAULT;
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
		from + = count;
		to + = count;
	}
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else	UMVSEG
	if (from < (caddr_t)v.v_ustart ||
	    (from+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyin: to address 0x%x failed\n",from));
		return(EFAULT);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		blt(to, (caddr_t)(from+USEROFF), nbytes);
	} else {
		val = EFAULT;
	}
	nofault = saved_jb;
#endif	UMVSEG
	return(val);
}

/*
 * bcopyin - Move bytes into the system space out of user's address space.
 *	copy only until a null or nbytes
 *	return number of bytes
 */
bcopyin(from, to, nbytes)
register caddr_t from, to;
register unsigned nbytes;
{
	register int val;
	jmp_buf jb;int *saved_jb;
#ifdef	UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;
	register int retval = 0;

	if (from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("bcopyin: to address 0x%x failed\n",from));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	val = 0;
	while (nbytes != 0) {
		u_faddr = (int)from;
		u_ste = (int)steaddr(u.u_stbl[snum(from)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)from & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			for (; val < count;) {
				if (from >= (caddr_t)v.v_uend) {
					TRACE(T_subyte,("bcopyin(2): to address 0x%x failed\n",from));
					nofault = saved_jb;
					retval = -1;
					goto out;
				}
				*to = *((caddr_t)(stob(UMOVESEG) + soff(from++)));	/* this can cause a fault */
				val++;
				if (*to++ == 0) {
					nofault = saved_jb;
					retval = val;
					goto out;
				}
			}
		} else {
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
	}
out:
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
	return( retval ? retval : (val == nbytes ? 0 : -1) );
#else	UMVSEG
	if (from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("bcopyin: to address 0x%x failed\n",from));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		from += USEROFF;
		for (val = 0; val < nbytes; ) {
			if ((from-USEROFF) >= (caddr_t)v.v_uend) {
				TRACE(T_subyte,("bcopyin(2): to address 0x%x failed\n",from));
				nofault = saved_jb;
				return(-1);
			}
			*to = *from++;	/* this can cause a fault */
			val++;
			if (*to++ == 0) {
				nofault = saved_jb;
				return(val);
			}
		}
	}
	nofault = saved_jb;
	if (val == nbytes)
		return(0);
	return(-1);
#endif	UMVSEG
}

/*
 * wcopyin - Move words into the system space out of user's address space.
 *	copy only until a null or nbytes
 *	return number of bytes
 */
wcopyin(from, to, nbytes)
register int *from, *to;
register unsigned nbytes;
{
	register int val;
	jmp_buf jb;int *saved_jb;
#ifdef	UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;
	register char *fromcp;
	register int retval = 0;

	if ((caddr_t)from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("wcopyin(1): to address 0x%x failed\n",from));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	val = 0;
	fromcp = (char *) from;
	while (nbytes != 0) {
		u_faddr = (int)fromcp;
		u_ste = (int)steaddr(u.u_stbl[snum(fromcp)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)fromcp & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			for (; val < count;) {
				if ((fromcp+sizeof(int)) > (caddr_t)v.v_uend) {
					TRACE(T_subyte,("wcopyin(2): to address 0x%x failed\n",from));
					nofault = saved_jb;
					retval = -1;
					goto out;
				}
				ASSERT(count >= sizeof(int));
				from = (int *) (stob(UMOVESEG) + soff(fromcp));
				*to = *from;	/* this can cause a fault */
				fromcp += sizeof(int);
				val += sizeof(int);
				if (*to++ == 0) {
					nofault = saved_jb;
					retval = val;
					goto out;
				}
			}
		} else {
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
	}
out:
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
	return( retval ? retval : (val == nbytes ? 0 : -1) );
#else	UMVSEG
	if ((caddr_t)from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("wcopyin(1): to address 0x%x failed\n",from));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		from = (int *)((int)from + USEROFF);
		for (val = 0; val < nbytes; ) {
			if (((caddr_t)(from+1)-USEROFF) > (caddr_t)v.v_uend) {
				TRACE(T_subyte,("wcopyin(2): to address 0x%x failed\n",from));
				nofault = saved_jb;
				return(-1);
			}
			*to = *from++;	/* this can cause a fault */
			val += sizeof(int);
			if (*to++ == 0) {
				nofault = saved_jb;
				return(val);
			}
		}
	}
	nofault = saved_jb;
	if (val == nbytes)
		return(0);
	return(-1);
#endif	UMVSEG
}

/*
 * copypage - Copy one logical page given the page frame number for
 * the source and destination.
 *
 * NOTE: The New pmmu code assumes that all ram is mapped in 1-1,
 *	an assumption that should be ok until someone makes a box
 *	with more than 2^31 bytes of physical memory.  However,
 *	the kernel Virtual address, and the user address in kernel
 *	space may need to be adjusted to avoid discontiguous memory.
 *	See TAKVINDEX/TAKVNUM, and TAUINDEX/TAUNUM in sys/pmmu.h
 */
copypage(from, to)
int from, to;
{
	/* convert page frame number to pfdat index */
#ifdef NEW_PMMU
	blt512(ptob(pptop(to)), ptob(pptop(from)), ptob(1)>>9);
#else NEW_PMMU
	wtpte(copypte->pgi, PG_V|PG_RW, pptop(from));	/* load kernel vaddrs */
	wtpte((copypte+1)->pgi, PG_V|PG_RW, pptop(to));
	invsatb(SYSATB, stob(COPYSEG), 2);		/* clear old vals */
	blt512((caddr_t)stob(COPYSEG)+NBPP, (caddr_t)stob(COPYSEG), ptob(1)>>9);
#endif NEW_PMMU
}

/*
 * clearpage - Clear one physical page given its page frame number
 */
clearpage(pfn)
int pfn;
{
	/* convert page frame number to pfdat index */
#ifdef NEW_PMMU
	clear(ptob(pptop(pfn)), ptob(1));
#else NEW_PMMU
	wtpte(copypte->pgi, PG_V|PG_RW, pptop(pfn));	/* load kernel vaddrs */
	invsatb(SYSATB, stob(COPYSEG), 1);		/* clear old vals */
	clear((caddr_t)stob(COPYSEG), ptob(1));
#endif NEW_PMMU
}

/* uclear: clear a section of user space
 */
uclear(uva, nbytes)
register caddr_t uva;
{
	int val = 0;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int inc;
	register caddr_t addr;
	register int off, count;
	register int ofaddr;
	register int oste;

	if (uva < (caddr_t)v.v_ustart ||
	    (uva+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("uclear: to address 0x%x failed\n",uva));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	while (nbytes > 0) {
		u_faddr = (int)uva;
		u_ste = (int)steaddr(u.u_stbl[snum(uva)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)uva & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			addr = (caddr_t)(stob(UMOVESEG) + soff(uva));
			for (inc=count; inc > 0; inc--, addr++)
				*addr = (char)0;
		} else {
			val = -1;
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
		uva + = count;
	}
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else UMVSEG
	if (uva < (caddr_t)v.v_ustart ||
	    (uva+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("uclear: to address 0x%x failed\n",uva));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		clear(uva+USEROFF,nbytes);
	} else {
		val = -1;
	}
	nofault = saved_jb;
#endif	UMVSEG
	return(val);
}
