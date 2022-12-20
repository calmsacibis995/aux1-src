#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/icpenviron.c */
#define _AC_NAME icpenviron_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:04}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of icpenviron.c on 87/11/11 21:07:04";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)icpenviron.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *	This file is organized in the following order
 *		defines and storage
 *		timer
 *		scc
 *		printf
 *		main
 */



#include <sys/types.h>
#include <sys/errno.h>
#include <sys/reg.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/debug.h>
#define	SCC_DEBUG
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/mmu.h>
#ifdef PAGING
#include <sys/page.h>
#endif PAGING
#include <sys/seg.h>
#ifdef PAGING
#include <sys/region.h>
#endif PAGING
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <fwd.h>
#include <fwdicp.h>
#include <sys/process.h>

#ifndef NULL
#define NULL 0
#endif

int 				wakeup_flag;	/* set when a wakeup is to be done */
extern		char		qrunflag;
extern		access_t	access;
extern		int		str_buffers[];
extern		int		sizeof_str_buffers;
extern		fep_specifics_t	icp_specifics;
extern		int		T_mainloop;
mblk_t				*qopenclose_h;
mblk_t				*qopenclose_t;
ushort				console;	/* scc channel for console */
unsigned char			*scc_ctl_read;
unsigned char			*scc_ctl_write;
unsigned char			*scc_data_read;
unsigned char			*scc_data_write;
uint				timer_counter1 = 12;
uint				timer_counter2;

extern stropen(), strclose(), strpush(), strpop();
extern strmname(), strlook(), strfind();
/*????*/
uint	level1s;
uint	gotlevel1;

int
bcopy(from, to, length)
short	*to;
short	*from;
int	length;		/* in bytes */

{
	if ((length | (int) from | (int) to) & 0x1)
	{
	    printf("bcopy: tried to copy odd boundry from %x to %x length %x\n",
		    from, to, length
		  );
	    length &= 0xfffe;
	}
	while (length > 0)
	{
	    *to++ = *from++;
	    length -= sizeof(*to);
	}
}

/************************************************************************/
/*									*/
/*			timer access code				*/
/*									*/
/************************************************************************/

int
init_via(via_no)
unsigned int	via_no;

{
	via_chip_t	*viap;

	if  (via_no >= N_VIA)
	    return(-1);
	viap = vias[(via_no)<<1];	/* *2 because two timers per via */
	viap->ddra = 0xc6;		/* ins and outs */
	viap->ddrb = 0;			/* bring in the modem control lines */
	viap->ier = VIA_IER_CLEAR | 0x7f; /* clear all interrupts */
	/* set up timer 1 for continuous interrupts, timer 2 for oneshot */
	viap->acr = VIA_ACR_T1_INTR_CONT | VIA_ACR_T2_INTR_ONESHOT;
	viap->pcr  = 0xee;		/* levels high */
}

at_timeout(time, timer_no)
register unsigned int	time;
unsigned int	timer_no;

{
	register via_chip_t	*viap = vias[timer_no];

#ifdef	DEBUG
	if  (timer_no >= N_TIMERS)
	    return(-1);
#endif	DEBUG
	if (!(timer_no & 1))  /* otherwise assume timer 2 */
	{
	    /* reset any pending interrupt */
	    viap->t1loc;
	    viap->t1loc = time & 0x00ff;
	    /* this resets the interrupt flag, & starts the counting */
	    viap->t1hoc = (time >> 8) & 0x00ff;
	    /* this enables the interrupt */
	    viap->ier = VIA_IER_SET | VIA_IER_TIMER1;
	}
	else
	{
	    /* reset any pending interrupt */
	    viap->t2loc;
	    viap->t2loc = time & 0x00ff;
	    /* this resets the interrupt flag, & starts the counting */
	    viap->t2hoc = (time >> 8) & 0x00ff;
	    /* this enables the interrupt */
	    viap->ier = VIA_IER_SET | VIA_IER_TIMER2;
	}
}


void
at_untimeout(timer_no)
unsigned int	timer_no;

{
	register via_chip_t	*viap = vias[timer_no];

	if (!(timer_no & 1))  /* otherwise assume timer 2 */
	    /* this disables the interrupt */
	    viap->ier = VIA_IER_CLEAR | VIA_IER_TIMER1;
	else
	    /* this disables the interrupt */
	    viap->ier = VIA_IER_CLEAR | VIA_IER_TIMER2;
}


viairupt(ap)
register struct args *ap;

{
    register via_chip_t	*viap = vias[CLOCK_TIMER];
    register int	dummy;

    /* find the source, if timers read the LOC so the iterrupt is reset */
    if  (viap->ifr & VIA_IER_TIMER2)
    {
	dummy = viap->t2loc;
#ifdef	APPLETALK
	at_timer_interrupt(ap);
#else	APPLETALK
	ap->a_dev = 1;
#endif	APPLETALK
    }
	if  (viap->ifr & VIA_IER_TIMER1)
	{
	    dummy = viap->t1loc;
	    clock(ap);
	}
}


testtimeout(i, time)
{
	timeout(testtimeout,i+1,1*HZ);
	printf("testtimeout: i=%d, time=%d\n", i, time);
}


/************************************************************************/
/*									*/
/*			scc access code					*/
/*									*/
/************************************************************************/
static	int
init_scc_channel(channel_no)
int	channel_no;

{
	register unsigned char	*scc_ctl_write;

	if (channel_no < 0 || channel_no >= N_SCC_CHANNELS)
	    return(-1);
	scc_ctl_write = sccs[channel_no].scc_ctl_write;
	SCC_CTL_WRITE(SCC_WR9, sccs[channel_no].wr9_reset);
	return(0);
}




scc_null_irupt(ap)
struct args *ap;

{
	printf("scc_nul_irupt: some application are getting unwanted interrupts on scc channel %d\n",
		ap->a_dev
	      );
	scc_default_irupt(ap);
}

scc_default_irupt(ap)
struct args *ap;

{
	register  unsigned char	*scc_ctl_write = sccs[ap->a_dev].scc_ctl_write;

	SCC_CTL_WRITE(SCC_WR9, sccs[ap->a_dev].wr9_reset);
}
/*	ressccirupt -- make slot interrupt handler.
 *	     An interrupt handler is installed for a given scc channel. This
 *	routine must be called by the device specific initialization code.
 *	A 0 is returned if the slot is already reserved. Even if the
 *	application does not use interrupts, it still needs to register to
 *	reserve that scc channel for it self. In that case it can pass NULL
 *	as the function pointer. If one wants to unreserve a channel, you have
 *	to put the default interrupt back, scc_default_irupt.
 */

ressccirupt(num, fp)

int	num;		/* nu bus slot number */
int	(*fp)();
{
	register s;

	s = splhi();
	if  (scc_vecs[num] != scc_default_irupt)
	{
	    /* someone has already reserved the vector */
	    splx(s);
	    return(0);
	}
	if  (fp == NULL)
	    scc_vecs[num] = scc_null_irupt;
	else
	    scc_vecs[num] = fp;
	splx(s);
	return(1);
}

/************************************************************************/
/*									*/
/*			printf code					*/
/*									*/
/************************************************************************/

static
init_printf(channel)
int	channel;

{
	int			i;

	if (channel < 0 || channel >= N_SCC_CHANNELS)
	    return(-1);
	if (!ressccirupt(channel, NULL))
	    return(-1);
	console = channel;
	/* set up the globals for console output */
	scc_ctl_read   =  sccs[console].scc_ctl_read;
	scc_ctl_write  =  sccs[console].scc_ctl_write;
	scc_data_read  =  sccs[console].scc_data_read;
	scc_data_write =  sccs[console].scc_data_write;
	SCC_CTL_WRITE(SCC_WR9, sccs[console].wr9_reset);
	scc_delay();
	scc_cmd(scc_rvd_serialinit, &sccs[console]);
}

/*
 * printf - scaled down C library printf, only
 * %s, %u, %d, %o, and %x.
 */

int printfstall = 0;

/* VARARGS1 */

printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register unsigned int *adx;
	register x83 = 0;	/* do not send to 83 output queue */
	register char *s;
	int sps;

	for (c = 0; c < printfstall; c++)
		;
	sps = splhi();

	adx = &x1;
	if(*fmt == '!')
		fmt++;
	else
		x83 = 1;

	for(;;) {
		while((c = *fmt++) != '%') {
			if(c == '\0') {
				splx(sps);
				return;
			}
			outchar(c);
		}
		switch( c = *fmt++ ) {
		case 'd':
		case 'u':
		case 'o':
		case 'x':
			printn((unsigned int)*adx, c, x83);
			break;
		case 'b':
			{
			char fcode;
			int b, any, i;

			b = *adx++;
			s = (char *)*adx;
			switch ((int) *s++) {
				case 8:
					fcode = 'o';
					break;

				case 10:
					fcode = 'u';
					break;

				case 16:
				default:
					fcode = 'x';
					break;
			}
			printn((long)b, fcode, x83);
			any = 0;
			if (b) {
				outchar('<');
				while (i = *s++) {
					if (b & (1 << (i - 1))) {
						if (any)
							outchar(',');
						any = 1;
						for (; (c = *s) > 32; s++)
							outchar(c);
					}
					else
						for (; *s > 32; s++)
							;
				}
				outchar('>');
			}
			}
			break;
		case 'c':
			outchar(*adx);
			break;
		case 's':
			s = (char *)*adx;
			while(c = *s++) {
				outchar(c);
			}
		}
		adx++;
	}
}

printn(val, fcode, x83)
unsigned int val;
{
	register int hradix, lowbit, plmax, i;
	char d[12];

	switch(fcode) {
	case 'd':
		if((int)val < 0) {
			outchar('-');
			val = -val;
		}
	case 'u':
		hradix = 5;
		plmax = 10;
		break;
	case 'o':
		hradix = 4;
		plmax = 11;
		break;
	case 'x':
		hradix = 8;
		plmax = 8;
		break;
	}
	for(i=0; i < plmax; i++) {
		lowbit = val & 1;
		val = val >> 1;
		d[i] = "0123456789ABCDEF"[val % hradix * 2 + lowbit];
		val /= hradix;
		if(val == 0)
			break;
	}
	if(i == plmax)
		i--;
	for(; i >= 0; i--) {
		outchar(d[i]);
	}
}


/*
 * output only to console
 */
outchar(c)
char c;
{
	scputchar(c);
}


scputchar(c)
{
	register int	i;
	register int	j;
	register int	s;

	s = spl7();
	if (c == '\n')
		scputchar('\r');
	i = 100000;
	while ((*scc_ctl_read & SCC_RR0_TBE) == 0 && --i)
#ifdef SCC_DELAY
	    {
		j = 10;
		do  {
		}            while (--j);
	    }
#endif
	    ;
	*scc_data_write = c;
	splx(s);
}


/************************************************************************/
/*									*/
/*			getchar code					*/
/*									*/
/************************************************************************/

getchar()
{
	return(scgetchar());
}


/*
 * Get a character from the SCC.  Ignore DC1 and DC3.
 */

#define CTRL(c)		('c' - 'a' + 1)
#define ignore(c)	((c) == CTRL(s) || (c) == CTRL(q))

scgetchar()
{
	register int s, c;

	for (;;)
	{
		while ((*scc_ctl_read & SCC_RR0_RCA) == 0)
		{
#ifdef SCC_DELAY
		    scc_delay();
#endif SCC_DELAY
		    SCC_CTL_WRITE(SCC_WR0, SCC_WR0_ERE);  /* Error reset. */
		}
		c = *scc_data_read & 0x7f;
		SCC_CTL_WRITE(SCC_WR0, SCC_WR0_RXI);  /* Reset external int. */
		SCC_CTL_WRITE(SCC_WR0, SCC_WR0_ERE);  /* Error reset. */
		if (!ignore(c))
			break;
	}
	return(c);
}


/************************************************************************/
/*									*/
/*			misc interface code				*/
/*									*/
/************************************************************************/


char	*panicstr;

/*
 *	panic - called on unresolvable fatal errors.
 *		the fix.sed script ensures that calls
 *		to saveregs are inserted before the calls
 *		to panic, ensuring all general and cpu
 *		registers reflect status at panic.
 *	It syncs, prints "panic: mesg" and then loops.
 */

panic(s)
char *s;
{
	if(s && panicstr) {
		printf("!Double panic: %s\n", s);
	} else {
		register int sxl = splhi();

		if (s)
			panicstr = s;
		printf("panic: %s\n", panicstr ?  panicstr : "???");
#ifdef	DEBUG
		printf("Warning:  entering debugger.\n");
		debug();
#endif
		splx(sxl);
		printf("panic: %s\n", panicstr);
	}
	for(;;)
		;
}


/*

/*#if OSDEBUG == YES*/
#ifdef HOWFAR

assfail(a, f, l)
register char *a, *f;
{
	/*	Save all registers for the dump since crash isn't
	 *	very smart at the moment.
	 */
	
#ifndef lint
	register int	r6, r5, r4, r3;
#endif lint

	printf("assertion failed: %s, file: %s, line: %d\n", a, f, l);
	panic("assertion error");
}

#endif


/************************************************************************/
/*									*/
/*			misc interface code				*/
/*									*/
/************************************************************************/

fault(dummy)
int	dummy;

{
	int	i;
	ushort	*sp;
	register int s;

	s = splhi();
	printf("fault: received ");
	switch(dummy)
	{
	    case 2:
		printf("a bus error");
		break;
	    case 3:
		printf("an address error");
		break;
	    case 4:
		printf("an illegal instruction");
		break;
	    case 5:
		printf("a zero divide");
		break;
	    case 24:
		printf("a spurious interrupt");
		break;
	    default:
		printf("a misc fault");
		break;
	} /* of switch */
	printf(", backtrace follows\n");
	sp = (ushort *) &dummy;
	printf("	%x\tfault number\n", * (ulong *) sp);
	sp += 2;
	printf("\t\tsaved registers\n");
	for  (i=4;i--;)					/* print saved regs  */
	{
	    printf("	%x\n", * (ulong *) sp);
	    sp += 2;
	}
	printf("\n");
	printf("	%x\tstatus/fc\n", *sp++);
	printf("	%x\treturn/access\n", * (ulong *) sp);
	sp += 2;
	if  ((dummy == 2) || (dummy == 3))
	{
	    printf("\n");
	    printf("	TM0 TM1 = %x\n", vias[0]->ora_ira);
	    printf("	%x\tinstruction reg\n", *sp++);
	    printf("	%x\tstatus\n", *sp++);
	    printf("	%x\treturn\n", * (ulong *) sp);
	    sp += 2;
	}
	printf("\t\tother things on stack\n");
	for (i=4;i--;)
	{
	    printf("	%x\n", * (ulong *) sp);
	    sp += 2;
	}
#ifdef	DEBUG
	debug();
#else	DEBUG
	if  ((dummy == 2) || (dummy == 3))
	    while (1)
		;
#endif	DEBUG
	splx(s);
}

/************************************************************************/
/*									*/
/*			main code					*/
/*									*/
/************************************************************************/

void
main()

{
int	delay_value;
	int		i, j;
	mblk_t		*mp;
	int		main_loops = 0;
	fep_specifics_t	*fep_specifics = &icp_specifics;
	fwd_circuit_t	*circuitp;
	fwd_acktags_t	*acktags;
	struct process	*proc;
	via_chip_t	*viap;
	access_t	*accessp = &access;
	short       	unix_cnt = 0;
	short		fep_cnt = 0;
	struct args 	args;

	args.a_dev = 0;
	for (i=0; i<N_SCC_CHANNELS; i++)
	    init_scc_channel(i);
	init_via(CLOCK_TIMER);
#ifdef	DEBUG
	init_printf(CONSOLE_CHANNEL);
#endif	DEBUG
	init_access();
	strinit(str_buffers, sizeof_str_buffers );
	clock_init();
	at_timeout(30715, CLOCK_TIMER);  /* interrupt at 30HZ rate */
#ifdef	APPLETALK
	{
	extern void	at_lap_interrupt();
	extern		atrcv1();

	/*
	 * first reserve the channel, then circumvent the normal interrupt vectoring
	 * because it is to slow for appletalk. The SCC calls the rcv routine directly
	 * which then calls at_lap_interrupt when all characters have been handled.
	 */
	if (!ressccirupt(AT_CHANNEL, at_lap_interrupt))
	    printf("channel %d is in use and cannot be used for APPLETALK\n", AT_CHANNEL);
	else
	    newsccvec(0, atrcv1);
	}
#endif	APPLETALK
#ifdef	DEBUG
	printf("\ninitialized\n");
#endif	DEBUG
	init_proc();
	SPL0();
#ifdef IRUPTTEST
	printf("starting to do command loop\n");
	for (j=100; j;)
	{
	    for (i=50000; i; i--)
	    {
		vias[0]->pcr = IRUPT_UNIX | IRUPT_FEP_N;
		while  (!gotlevel1)
		    ;
		gotlevel1 = 0;
		level1s++;
	    }
	    printf("%d ", level1s);
	}
	printf("finished with commands %d\n", level1s);
#endif IRUPTTEST
	viap = vias[0];
	while (1)
	{
#ifdef	DEBUG
	    TRACE(T_mainloop, ("mainloop:\n"));
	    SPL7();
	    if (*scc_ctl_read & SCC_RR0_RCA)
	    {
		if  (getchar() == 'd') {
		    debug();
		}
	    }
	    SPL0();
	    SCC_CTL_WRITE(SCC_WR0, SCC_WR0_ERE);  /* Error reset.          */
#endif	DEBUG
	    /* if runqueues */
	    if  (qrunflag)
	    {
		runqueues();
	    }

	    /*
	     *	Run any ready processes
	     */

	    if (pr_ready) 
		schedule_proc();
	    if  (qopenclose_h)
	    {
		/*
		 * there is an open or close request queued up to be run at
		 * spl0, in otherwords, do it now!
		 */
		SPL1();
		mp = qopenclose_h;
		switch (mp->b_datap->db_type) {
	  	case M_FWD_CANCEL:
		    if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    SPL0();
		    /* cancel the transaction */
	            acktags = FWD_ACKTAGS(mp);
	 	    proc=fep_specifics->fwd_circuit[acktags->circuit].proc[acktags->id];
		    if (proc)
			unsleep_proc(proc);
		    freemsg(mp);
		    break;

		case M_FWD_CLOSE:
		    if (run_proc(strclose, mp)) {
		    	if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    }
		    SPL0();
		    break;
	
		case M_FWD_OPEN:
		case M_FWD_1stOPEN:
		    if (run_proc(stropen, mp)) {
		    	if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    }
		    SPL0();
		    break;

		case M_FWD_PUSH:
		    /* push the indicated stream */
		    if (run_proc(strpush, mp)) {
		    	if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    }
		    SPL0();
		    break;

		case M_FWD_POP:
		    /* pop the indicated stream */
		    if (run_proc(strpop, mp)) {
		    	if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    }
		    SPL0();
		    break;
	
		case M_FWD_LOOK:
		    /* look at the indicated stream */
		    if (run_proc(strlook, mp)) {
		    	if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    }
		    SPL0();
		    break;
	
		case M_FWD_FIND:
		    /* find on the indicated stream */
		    if (run_proc(strfind, mp)) {
		    	if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    }
		    SPL0();
		    break;
	
		case M_FWD_MNAME:
		    /* mname the indicated stream */
		    if (run_proc(strmname, mp)) {
		    	if  ((qopenclose_h = qopenclose_h->b_next) == NULL)
		    		qopenclose_t = NULL;
		    }
		    SPL0();
		    break;
	
		default:
		    panic("invalid forwarder request");
		}
	    }

	    /*
	     *	See if the hardware dropped an interrupt (in either direction)
	     *		we don't protect this code (except when we do an interrupt)
	     *		because this is a background process and we expect to get
	     *		it next time round our loop
	     */

	    if (accessp->rep2fep.rp->cmd ||
	       accessp->exp2fep.rp->cmd ||
	       accessp->cmd2fep.rp->cmd) {
	        fep_cnt++;
	        if (fep_cnt > 20) {
	    	    printf("waking FEP\n");
	            (void) splipc();
	            fwdicpirupt(&args);
	            (void) spl0();
	            fep_cnt = 0;
	        }
	    } else {
		    fep_cnt = 0;
	    }

	    if (accessp->rep2unix.rp->cmd ||
	       accessp->exp2unix.rp->cmd ||
	       accessp->cmd2unix.rp->cmd) {
	    	unix_cnt++;
	    	if (unix_cnt > 500) {
		    printf("waking Unix\n");
	       	    (void) splipc();
	    	    viap->pcr = IRUPT_UNIX | (viap->pcr & 0x1f);
	    	    (void) spl0();
	    	    unix_cnt = 0;
	    	}
	    } else {
	    	unix_cnt = 0;
	    }

	    main_loops++;
	}
}
