#ifndef lint	/* .../sys/psn/io/via.c */
#define _AC_NAME via_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer Inc., All Rights Reserved.  {Apple version 1.3 87/11/19 18:04:55}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of via.c on 87/11/19 18:04:55";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)via.c	UniPlus VVV.2.1.13	*/
/*
 * VIA device driver
 *
 *	Copyright 1986 UniSoft Corporation
 */

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/uconfig.h"
#include "sys/reg.h"
#endif lint
#include "sys/via6522.h"

extern	noint();
extern	onesec(), clock();
extern  fdb_intr(), scsiirq(), slotintr();

int	(*lvl1funcs[7])() = {
	onesec,			/* CA 2, one per sec */
	clock,			/* CA 1, 60 HZ */
	fdb_intr,		/* Shift Register */
	noint,			/* CB 2 */	
	noint,			/* CB 1 Serial from TOD clock shift clock */
	noint,			/* Timer 2 */
	noint,			/* Timer 1 */
	};

int	(*lvl2funcs[7])() = {
	noint,			/* CA 2, SCSI */
	slotintr,		/* CA 1, Slots */
	noint,			/* Shift Register */
	scsiirq,		/* CB 2 */
	noint,			/* CB 1 */
	noint,
	noint,
	};

int	(*slotfuncs[6])() = {
	noint,
	noint,
	noint,
	noint,
	noint,
	noint,
	};

char via1_soft = 0;
char via2_soft = 0;
static	struct via *iusvia;
static	u_char iusmsk;
static	char *iussoft;

/*	viaclrius -- clear interrupt under service.
 *	     This routine clears the current interrupt.
 */

viaclrius()

{
	if(iusvia == 0)
		panic("viaclrius called from non-interrupt");
	iusvia->ifr = iusmsk;
	*iussoft &= ~iusmsk;
}

/*	via1init -- perform initialization.
 *	This routine performs initialization for both via devices.
 */

via1init()
{
	register struct via *vp;
	
	vp = (struct via *)VIA1_ADDR;
	vp->pcr = 0x20;
	vp->ier = VIE_CA1 | VIE_CA2 | VIE_SET;
	vp = (struct via *)VIA2_ADDR;
	vp->pcr = 0x66;
	vp->regb |= 0x02;		/* clear NuBus lock */
	vp->ddrb |= 0x02;
}

via1intr(args)

struct	args *args;
{
	register ifr, msk;
	register (**fp)();
	register struct via *via1 = (struct via *)VIA1_ADDR;
	struct via *saviusvia;
	int	saviusmsk;
	char *saviussoft;

	saviusvia = iusvia;
	saviusmsk = iusmsk;
	saviussoft = iussoft;
	iusvia = via1;
	iussoft = &via1_soft;
	do {
		ifr = (via1_soft|via1->ifr) & via1->ier & 0x7F;
		msk = 1;
		fp = lvl1funcs;
		do {
			if(msk & ifr) {
				iusmsk = msk;
				(*fp)(args);
				ifr = (via1_soft|via1->ifr) & via1->ier & 0x7F;
			}
			msk <<= 1;
			++fp;
		} while(msk <= ifr);
	} while(via1->ifr & 0x80);
	iusvia = saviusvia;;
	iusmsk = saviusmsk;
	iussoft = saviussoft;
}


via2intr(args)

struct args *args;
{
	register ifr, msk;
	register (**fp)();
	register struct via *via2 = (struct via *)VIA2_ADDR;
	struct via *saviusvia;
	int	saviusmsk;
	char *saviussoft;

	saviusvia = iusvia;
	saviusmsk = iusmsk;
	saviussoft = iussoft;
	iusvia = via2;
	iussoft = &via2_soft;
	do {
		ifr = (via2_soft|via2->ifr) & via2->ier & 0x7F;
		msk = 1;
		fp = lvl2funcs;
		do {
			if(msk & ifr) {
				iusmsk = msk;
				(*fp)(args);
				ifr = (via2_soft|via2->ifr) & via2->ier & 0x7F;
			}
			msk <<= 1;
			++fp;
		} while(msk <= ifr);
	} while(via2->ifr & 0x80);
	iusvia = saviusvia;
	iusmsk = saviusmsk;
	iussoft = saviussoft;
}
	
/*	viamkslotintr -- make slot interrupt handler.
 *	     A slot interrupt handler is installed for a given slot.  This
 *	routine must be called by the device specific initialization code.
 *	If intr is non-zero slot interrupts are enabled.
 */

viamkslotintr(num, fp, intr)

int	num;		/* nu bus slot number */
int	(*fp)();
{
	register s;

	s = splhi();
	slotfuncs[num - SLOT_LO] = fp;	/* SLOT_LO is slot bias */
	if (intr)
		((struct via *)VIA2_ADDR)->ier = VIE_CA1 | VIE_SET;	
					/* turn on all slot interrupts */
	splx(s);
}
	

slotintr(args)

struct args *args;
{
	register msk, num;
	register (**fp)();
	register struct via *via2 = (struct via *)VIA2_ADDR;

	do {
		msk = 1;
		fp = slotfuncs;
		num = SLOT_LO;
		do {
			if(!(msk & (via2->rega&0x3F))){ /* Active low signal */
				args->a_dev = num;
				(*fp)(args);
			}
			msk <<= 1;
			++num;
			++fp;
		} while(msk <= ((~via2->rega) & 0x3F));
		via2->ifr = VIE_CA1;
	} while((via2->rega & 0x3F) != 0x3F);
}

/*	noint -- default handler.
 */

noint()

{
	viaclrius();
#ifndef	ANYKINDOFDEBUG
	printf("no interrupt handler\n");
#endif	ANYKINDOFDEBUG
}

