	#	@(#)Copyright Apple Computer 1987	Version 1.3 of snrd.s on 87/11/11 21:39:09
 	file	"snrd.s"
#define LOCORE
#include <sys/uconfig.h>
	set 	DiskQ6L,0x1800
	set	DiskQ6H,0x1A00
	set	DiskQ7L,0x1C00
	set	DiskQ7H,0x1E00
	data 1
	comm	selwait,4
	even
	global	DtaMks,TagData,Nibl
DtaMks:
	byte	0xD5
	byte	0xAA
	byte	0xAD
	byte	0xDE
	byte	0xAA
	byte	0xFF
MarkTbl:
	byte	0xff,0x3f,0xcf,0xf3,0xfc,0xff,0xd5,0xaa
TagData:
	byte	0,0,0,0,0,0,0,0,0,0,0,0,0
Nibl:
	byte	0x96,0x97,0x9a,0x9b,0x9d,0x9e,0x9f,0xa6
	byte	0xa7,0xab,0xac,0xad,0xae,0xaf,0xb2,0xb3
	byte	0xb4,0xb5,0xb6,0xb7,0xb9,0xba,0xbb,0xbc
	byte	0xbd,0xbe,0xbf,0xcb,0xcd,0xce,0xcf,0xd3
	byte	0xd6,0xd7,0xd9,0xda,0xdb,0xdc,0xdd,0xde
	byte	0xdf,0xe5,0xe6,0xe7,0xe9,0xea,0xeb,0xec
	byte	0xed,0xee,0xef,0xf2,0xf3,0xf4,0xf5,0xf6
	byte	0xf7,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
TrailMks:
	byte	0xde,0xaa,0xff,0xff

	text
	global snrd, snwr
/*	snrd -- low level read.
 *	Called only from C
 *	     stat = snrd(buf_to_fill);
 */
snrd:
	mov.l	4(%sp),%a0
	movm.l  &0x3F3C,-(%sp)		# d[2-7] a[2-5]
	mov.l	%a0,%a5
	mov.l	&DtaMks,%a1
	mov.l	iwm_addr,%a4
	add.l	&DiskQ6L,%a4
	mov.l	&DNib,%a3
	sub.l	&0x96,%a3
	mov.l	&60,%d2
	mov.l	&0,%d3
	mov.l	&-64,%d0
	mov.l	&0x1FE000A,%d4		# buffer byte count and tag count
	mov.l	&0,%d5
	mov.l	&0,%d6
	mov.l	&0,%d7
RdData1:
	mov.l	%a1,%a2
	mov.l	&3,%d1
L%%1:
	mov.b	(%a4),%d3
	bpl.b	L%%1
L%%2:
	dbra	%d2,L%%3
	mov.l	&-71,%d0
	bra	snXit
L%%3:
	cmp.b	%d3,(%a2)+
	bne.b	RdData1
	sub.w	&1,%d1
	bne.b	L%%1
	# found data mark header, retrieve info. from mark
	mov.l	&TagData,%a1
L%%4:
	mov.b	(%a4),%d3
	bpl.b	L%%4
	mov.b	0(%a3,%d3.w),(%a1)+

RdData2:
	mov.b	(%a4),%d3
	bpl.b	RdData2
	mov.b	0(%a3,%d3.w),%d1
	rol.b	&2,%d1
	mov.b	%d1,%d2
	and.b	%d0,%d2
L%%5:
	mov.b	(%a4),%d3
	bpl.b	L%%5
	or.b	0(%a3,%d3.w),%d2
	mov.b	%d7,%d3
	add.b	%d7,%d3
	rol.b	&1,%d7
	eor.b	%d7,%d2
	mov.b	%d2,(%a1)+
	addx.b	%d2,%d5
	rol.b	&2,%d1
	mov.b	%d1,%d2
	and.b	%d0,%d2
L%%6:
	mov.b	(%a4),%d3
	bpl.b	L%%6
	or.b	0(%a3,%d3.w),%d2
	eor.b	%d5,%d2
	mov.b	%d2,(%a1)+
	addx.b	%d2,%d6
	rol.b	&2,%d1
	and.b	%d0,%d1
L%%7:
	mov.b	(%a4),%d3
	bpl.b	L%%7
	or.b	0(%a3,%d3.w),%d1
	eor.b	%d6,%d1
	mov.b	%d1,(%a1)+
	addx.b	%d1,%d7
	sub.w	&3,%d4
	bpl.b	RdData2

	swap	%d4
	# read verify check  was here
RdData3:
	mov.b	(%a4),%d3
	bpl.b	RdData3
	mov.b	0(%a3,%d3.w),%d1
	rol.b	&2,%d1
	mov.b	%d1,%d2
	and.b	%d0,%d2
L%%8:
	mov.b	(%a4),%d3
	bpl.b	L%%8
	or.b	0(%a3,%d3.w),%d2
	mov.b	%d7,%d3
	add.b	%d7,%d3
	rol.b	&1,%d7
	eor.b	%d7,%d2
	mov.b	%d2,(%a5)+
	addx.b	%d2,%d5
	rol.b	&2,%d1
	mov.b	%d1,%d2
	and.b	%d0,%d2
L%%9:
	mov.b	(%a4),%d3
	bpl.b	L%%9
	or.b	0(%a3,%d3.w),%d2
	eor.b	%d5,%d2
	mov.b	%d2,(%a5)+
	addx.b	%d2,%d6
	tst.w	%d4
	beq.b	RdCkSum
	rol.b	&2,%d1
	and.b	%d0,%d1
L%%10:
	mov.b	(%a4),%d3
	bpl.b	L%%10
	or.b	0(%a3,%d3.w),%d1
	eor.b	%d6,%d1
	mov.b	%d1,(%a5)+
	addx.b	%d1,%d7
	sub.w	&3,%d4
	bra.b	RdData3

	# read verify code was here

RdCkSum:
	mov.b	(%a4),%d3
	bpl.b	RdCkSum
	mov.b	0(%a3,%d3.w),%d1
	bmi.b	DCkSumBad
	rol.b	&2,%d1
	mov.b	%d1,%d2
	and.b	%d0,%d2
L%%11:
	mov.b	(%a4),%d3
	bpl.b	L%%11
	mov.b	0(%a3,%d3.w),%d3
	bmi.b	DCkSumBad
	or.b	%d3,%d2
	cmp.b	%d2,%d5
	bne.b	DCkSumBad
	rol.b	&2,%d1
	mov.b	%d1,%d2
	and.b	%d0,%d2
L%%12:
	mov.b	(%a4),%d3
	bpl.b	L%%12
	mov.b	0(%a3,%d3.w),%d3
	bmi.b	DCkSumBad
	or.b	%d3,%d2
	cmp.b	%d2,%d6
	bne.b	DCkSumBad
	rol.b	&2,%d1
	and.b	%d0,%d1
L%%13:
	mov.b	(%a4),%d3
	bpl.b	L%%13
	mov.b	0(%a3,%d3.w),%d3
	bmi.b	DCkSumBad
	or.b	%d3,%d1
	cmp.b	%d1,%d7
	beq.b	RdSlip

DCkSumBad:
	mov.l	&-72,%d0
	bra.b	snXit

RdSlip:
	mov.l	&1,%d4
L%%14:
	mov.b	(%a4),%d3
	bpl.b	L%%14
	cmp.b	%d3,(%a2)+
	bne.b	NoDSlip
	dbra	%d4,L%%14
	mov.l	&0,%d0

snXit:
	movm.l	(%sp)+,&0x3CFC
	rts

NoDSlip:
	mov.l	&-73,%d0
	bra.b	snXit

/*	snwr -- sony low level write routine.
 *	Called only from C
 *	   stat = snwr(buff_to_write);
 */
snwr:
	mov.l	4(%sp),%a0
	movm.l  &0x3F3C,-(%sp)		# d[2-7] a[2-5]
	mov.l	&MarkTbl,%a2
	mov.l	&TagData,%a1
	mov.l	&0x2010009,%d4		# buffer byte count and tag count
	mov.l	&0,%d2
	mov.l	&0,%d3
	mov.l	iwm_addr,%a3
	tst.b	DiskQ6H(%a3)
	mov.l	&6,%d0			# HeaderSize - 2
	mov.l	&0,%d5
	mov.b	(%a2)+,DiskQ7H(%a3)
	mov.l	&0,%d6
	mov.l	&0,%d7
WrHead:
	mov.b	(%a2)+,%d1
L%%21:	tst.b	DiskQ6L(%a3)
	bpl.b	L%%21
	mov.b	%d1,DiskQ6H(%a3)
	sub.w	&1,%d0
	bne.b	WrHead
	mov.b	(%a2)+,%d1
	mov.l	&Nibl,%a2
L%%22:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%22
	mov.b	%d1,DiskQ6H(%a3)
	mov.l	&0x0b,%d1
	mov.b	(%a1)+,%d2
	bra.b	WrData2
WrDataSw:
	mov.l	%a0,%a1
WrData1:
	addx.b	%d2,%d7
	eor.b	%d6,%d2
	mov.b	%d2,%d3
	lsr.w	&6,%d3
L%%23:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%23
	mov.b	0(%a2,%d3.w),DiskQ6H(%a3)
	sub.w	&3,%d4
	mov.b	%d7,%d3
	add.b	%d7,%d3
	rol.b	&1,%d7
	and.b	&0x3f,%d0
L%%24:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%24
	mov.b	0(%a2,%d0.w),DiskQ6H(%a3)
WrData2:
	mov.b	(%a1)+,%d0
	addx.b	%d0,%d5
	eor.b	%d7,%d0
	mov.b	%d0,%d3
	rol.w	&2,%d3
	and.b	&0x3f,%d1
L%%25:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%25
	mov.b	0(%a2,%d1.w),DiskQ6H(%a3)
	mov.b	(%a1)+,%d1
	addx.b	%d1,%d6
	eor.b	%d5,%d1
	mov.b	%d1,%d3
	rol.w	&2,%d3
	and.b	&0x3f,%d2
L%%26:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%26
	mov.b	0(%a2,%d2.w),DiskQ6H(%a3)
	mov.b	(%a1)+,%d2
	tst.w	%d4
	bne.b	WrData1
	swap 	%d4
	bne.b	WrDataSw
WrLast2:
	clr.b	%d3
	lsr.w	&6,%d3
L%%27:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%27
	mov.b	0(%a2,%d3.w),DiskQ6H(%a3)
	mov.b	%d5,%d3
	rol.w	&2,%d3
	mov.b	%d6,%d3
	rol.w	&2,%d3
	and.b	&0x3f,%d0
L%%28:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%28
	mov.b	0(%a2,%d0.w),DiskQ6H(%a3)
	and.b	&0x3f,%d1
L%%29:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%29
	mov.b	0(%a2,%d1.w),DiskQ6H(%a3)
WrCkSum:
	mov.b	%d7,%d3
	lsr.w	&6,%d3
L%%30:
	tst.b DiskQ6L(%a3)
	bpl.b	L%%30
	mov.b	0(%a2,%d3.w),DiskQ6H(%a3)
	and.b	&0x3f,%d5
L%%31:
	tst.b 	DiskQ6L(%a3)
	bpl.b	L%%31
	mov.b	0(%a2,%d5.w),DiskQ6H(%a3)
	and.b	&0x3f,%d6
L%%32:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%32
	mov.b	0(%a2,%d6.w),DiskQ6H(%a3)
	and.b	&0x3f,%d7
L%%33:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%33
	mov.b	0(%a2,%d7.w),DiskQ6H(%a3)
	mov.l	&TrailMks,%a2
	mov.l	&3,%d2
WrSlip:
	mov.b	DiskQ6L(%a3),%d1
	bpl.b	WrSlip
	mov.b	(%a2)+,DiskQ6H(%a3)
	dbra	%d2,WrSlip
	mov.l	&0,%d0
	btst	&6,%d1
	bne.b	L%%34
	mov.l	&-74,%d0
L%%34:
	tst.b	DiskQ7L(%a3)
	bra 	snXit
