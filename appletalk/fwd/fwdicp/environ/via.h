#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/via.h */
#define _AC_NAME via_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:08:21}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of via.h on 87/11/11 21:08:21 */
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*      This is the header file for accesses to a VIA SY6522	*/


/*+-------------------------------------------------------------------------+
  |     V I A    D E F I N I T I O N S                                      |
  +-------------------------------------------------------------------------+*/

/* only the ones that I need, the rest is left as an exercise */
#define	VIA_ACR_T1_INTR_ONESHOT	0x00
#define	VIA_ACR_T1_INTR_CONT	0x40
#define	VIA_ACR_T2_INTR_ONESHOT	0x00

#define	VIA_IER_SET		0x80
#define	VIA_IER_CLEAR		0x00
#define	VIA_IER_TIMER1		0x40
#define	VIA_IER_TIMER2		0x20

#define VIA_IRB_SCC1_DSRA	0x80
#define VIA_IRB_SCC1_RIA 	0x40
#define VIA_IRB_SCC1_DSRB	0x20
#define VIA_IRB_SCC1_RIB 	0x10
#define VIA_IRB_SCC2_DSRA	0x8
#define VIA_IRB_SCC2_RIA 	0x4
#define VIA_IRB_SCC2_DSRB	0x2
#define VIA_IRB_SCC2_RIB 	0x1

/*+-------------------------------------------------------------------------+
  |     V I A    T Y P E D E F						    |
  +-------------------------------------------------------------------------+*/

typedef struct
{
    unsigned char	orb_irb;
						    unsigned char	f0;
    unsigned char	ora_ira;
						    unsigned char	f1;
    unsigned char	ddrb;
						    unsigned char	f2;
    unsigned char	ddra;
						    unsigned char	f3;
    unsigned char	t1loc;
						    unsigned char	f4;
    unsigned char	t1hoc;
						    unsigned char	f5;
    unsigned char	t1lol;
						    unsigned char	f6;
    unsigned char	t1hol;
						    unsigned char	f7;
    unsigned char	t2loc;
						    unsigned char	f8;
    unsigned char	t2hoc;
						    unsigned char	f9;
    unsigned char	shift;
						    unsigned char	f10;
    unsigned char	acr;
						    unsigned char	f11;
    unsigned char	pcr;
						    unsigned char	f12;
    unsigned char	ifr;
						    unsigned char	f13;
    unsigned char	ier;
						    unsigned char	f14;
}   via_chip_t;

typedef via_chip_t	*via_t;
