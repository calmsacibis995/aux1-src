	#	@(#)Copyright Apple Computer 1987	Version 1.3 of mst.s on 87/11/11 21:39:05
	file	"mst.s"
#define LOCORE
#include <sys/uconfig.h>
	set	ph0L,0
	set	ph0H,0x200
	set	ph1L,0x400
	set	ph1H,0x600
	set	ph2L,0x800
	set	ph2H,0xa00
	set	ph3L,0xc00
	set	ph3H,0xe00
	set	mtrOff,0x1000
	set	mtrOn,0x1200
	set	intDrive,0x1400
	set	extDrive,0x1600
	set	q6L,0x1800
	set	q6H,0x1a00
	set	q7L,0x1c00
	set	a7H,0x1e00
	set	VBase,VIA1_ADDR
	set	VBufA,0x1e00
	data	1
	comm	selwait,4
	text
	global	LAdrDisk

/*	LAdrDisk -- Set chip to access disk address.
 *	Entry:
 *	   (d0) == Number of the disk register to access
 *	Exit:
 *	   (a2) == address of IWM chip
 *	   (a4) == via address.
 *	Called from:
 *		fmt.s
 *	Note, this routine is duplicated in sony.c adrdisk().
 */
LAdrDisk:
	mov.l	iwm_addr,%a2
	mov.l	&VIA1_ADDR,%a4
	tst.b	ph0H(%a2)		# Ph0H 
	tst.b	ph1H(%a2)		# Ph1H
	tst.b	ph2L(%a2)		# Ph2L
	lsr.b	&1,%d0
	bcc.b	L%%20
	tst.b	ph2H(%a2)		# Ph2H

L%%20:
	bclr	&5,VBufA(%a4)
	lsr.b	&1,%d0
	bcc.b	L%%21
	bset	&5,VBufA(%a4)

L%%21:
	lsr.b	&1,%d0
	bcs.b	L%%22
	tst.b	ph0L(%a2)		# Ph0L

L%%22:
	lsr.b	&1,%d0
	bcs.b	L%%23
	tst.b	ph1L(%a2)		# Ph1L

L%%23:	rts
