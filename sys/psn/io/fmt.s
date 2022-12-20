	#	@(#)Copyright Apple Computer 1987	Version 1.3 of fmt.s on 87/11/11 21:38:56
	file "fmt.s"
#define LOCORE
#include <sys/uconfig.h>
	# NO polling of serial is done
	# The code is not reentrant
	
	set 	DiskQ6L,0x1800
	set	DiskQ6H,0x1A00
	set	DiskQ7L,0x1C00
	set	DiskQ7H,0x1E00
	set	RdDataAdr,1
	set	RdData1Adr,3
	set	minSync,4
	set	TrkOffset,9
	set	SecOffset,10
	set	SidOffset,11
	set	VolOffset,12
	set	CkSumOffset,13
	set	DSecOffset,26
	set	VolumeFmt,2
	set	Vol2Fmt,0x22
	set	adrBytCnt,27
	set	syncBytCnt,6
	set	dataBytCnt,703
	set	strtSync,200
	set	Fmt1Err,-82
	set	Fmt2Err,-83
	set	MustFindCnt,1500
	set	WrUnderRun,-84
	data 1
	comm	selwait,4
	even
	global	SyncTbl,DskErr
SyncTbl:
AdrMkTbl:
	byte 0xff,0x3f,0xcf,0xf3,0xfc,0xff,0xd5,0xaa,0x96,0,0,0,0
	byte 0,0xde,0xaa,0xff,0xff,0x3f,0xcf,0xf3,0xfc,0xff
	byte 0xd5,0xaa,0xad,0,0

	comm	SectCnt,2
	comm 	DskErr,2
	comm	fmt2side,2

	text
	global FormatTrack

/*	FormatTrack -- format both sides of a single track.
 *	called from C:
 *		status = FormatTrack(number)
 */
FormatTrack:
	mov.l	4(%sp),%d1
	movm.l	&0x3f3c,-(%sp)		# D[2-7] A[2-5]
	mov.w	%d1,%d5
FmtTrk0:
	and.w	&0xff,%d1
	lsr.w	&4,%d1			# speed class
	mov.l	&12,%d0			# max. no. secs. per track
	sub.w	%d1,%d0			# reduce by 1 per speed class
	mov.w	%d0,SectCnt
	mov.w	%d5,%d4
	bsr	FillInMarks		# put needed data in fmtbuf
	
FmtTrk1:
	bsr	DoFormat
	beq.b	L%%1
	bra	FTExit			# exit immediately on error
	# check the inter-sector gap ...
L%%1:
	bsr 	RdAddr
	bmi.b	L%%2			# error
	tst.b	%d0			# contains sector number, should be 0
	beq.b	L%%2
	mov.w	&Fmt1Err,DskErr		# set "not sec. 0" error
L%%2:
	mov.l	&GapSync,%a0
	mov.w	DskErr,%d0		# either an error code or the remainder
					# from RdAddr which is an indication of
					# the number of nibbles before sec. 0
	bmi.b	DecrSn1			# decrement sync count
	mov.l	&MustFindCnt+4,%d1
	sub.w	%d0,%d1
	divu	&5,%d1
	mov.w	&Fmt2Err,DskErr		# assume 'not enough sync' error
	sub.w	(%a0),%d1
	bmi.b	DecrSync
	ext.l	%d1
	divu	SectCnt,%d1
	beq.b	FTExitOK
	cmp.w	%d1,&1
	beq.b	FTExitOK
	add.w	&1,(%a0)
FTExitOK:
	clr.w	DskErr
	tst.w	fmt2side
	beq	FTExit			# exit if one-sided requested
	bset	&11,%d5
	bne	FTExit			# exit if all sides formatted
	mov.w	%d5,%d1
	bra	FmtTrk0			# do the second side
FTExit:
	movm.l	(%sp)+,&0x3CFC		# A[5-2] D[7-2]
	mov.l	&0,%d0
	mov.w	DskErr,%d0
	rts
DecrSync:
	add.w	&1,%d1
	beq.b	FTExitOK
DecrSn1:
	sub.w	&1,(%a0)
	cmp.w	(%a0),&minSync		# sync too small?
	blt.b	FTExit			# yes
	mov.w	%d5,%d4
	bra	FmtTrk1			# go try again
	# actually write the format on the track
DoFormat:
	mov.l	&RdDataAdr,%d0
	btst	&11,%d4			# Flag that this is side two
	beq	L%%41
	mov.l	&RdData1Adr,%d0
L%%41:
	bsr	LAdrDisk			# select side 1
	mov.l	iwm_addr,%a3
	mov.l	&fmtbuf,%a0
	mov.w	SectCnt,%d0
	mov.w	GapSync,%d1
	sub.w	&2,%d1
	mov.l	&SyncTbl,%a1
	mov.w	&strtSync-1,%d3
	tst.b	DiskQ6H(%a3)
	mov.b	%d3,DiskQ7H(%a3)	# get into write mode
WrStrtSync:
	mov.l	%a1,%a2
	mov.l	&2,%d2			# syncBytCnt/2 - 1
WSS1:
	mov.b	(%a2)+,%d4
L%%5:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%5
	mov.b	%d4,DiskQ6H(%a3)
	mov.b	(%a2)+,%d4
L%%6:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%6
	mov.b	%d4,DiskQ6H(%a3)
	
	dbra	%d2,WSS1
	dbra	%d3,WrStrtSync
WrNxtSect:
	mov.w	%d1,%d3
L%%7:
	mov.l	%a1,%a2
	mov.l	&syncBytCnt-1,%d2
L%%8:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%8
	mov.b	(%a2)+,DiskQ6H(%a3)
	dbra	%d2,L%%8
	dbra	%d3,L%%7
	mov.l	&adrBytCnt-1,%d2
L%%9:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%9
	mov.b	(%a0)+,DiskQ6H(%a3)
	dbra	%d2,L%%9
	mov.w	&dataBytCnt-1,%d2
L%%10:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%10
	mov.b	&0x96,DiskQ6H(%a3)		# write out 0's
	dbra	%d2,L%%10
	mov.l	&3,%d2
	add.l	&8,%a2			# point ar slip marks
L%%11:
	tst.b	DiskQ6L(%a3)
	bpl.b	L%%11
	mov.b	(%a2)+,DiskQ6H(%a3)
	dbra	%d2,L%%11
	sub.w	&1,%d0
	bgt.b	WrNxtSect
	
	mov.b	DiskQ6L(%a3),%d0
	mov.l	&0,%d2
	btst	&6,%d0
	bne.b	L%%12
	mov.l	&WrUnderRun,%d0
	bra.b	L%%13
L%%12:
	mov.l	&0,%d0
L%%13:
	tst.b	DiskQ7L(%a3)			# get out of write mode
	tst.w	%d0
	rts
	# Put address marks in the format buffer
FillInMarks:
	movm.l	&0xffC0,-(%sp)		# D[7-0] A[21]
	mov.w	%d4,%d6			# track num. is in d4
	mov.l	&fmtbuf,%a0
	mov.l	&Nibl,%a1
	mov.w	&Vol2Fmt,%d3
	tst.w	fmt2side
	bne	L%%131
	mov.w	&VolumeFmt,%d3
L%%131:
	mov.b	0(%a1,%d3.w),%d7
	mov.b	%d3,%d5
	mov.l	&0x3f,%d3
	and.b	%d6,%d3
	mov.b	0(%a1,%d3.w),%d2
	eor.b	%d3,%d5
	lsr.w	&6,%d6	
	eor.b	%d6,%d5
	mov.b	0(%a1,%d6.w),%d6
	mov.w	%d0,%d1
	sub.w	&1,%d1
	lsr.w	&1,%d1
	add.w	&1,%d1
	swap 	%d1
	clr.w	%d1
L%%20:
	mov.b	0(%a1,%d1.w),%d3
	mov.l	&0,%d4
	mov.b	%d5,%d4
	eor.b	%d1,%d4
	add	&TrkOffset,%a0
	mov.b	%d2,(%a0)+
	mov.b	%d3,(%a0)+
	mov.b	%d6,(%a0)+
	mov.b	%d7,(%a0)+
	mov.b	0(%a1,%d4.w),(%a0)+
	add	&12,%a0
	mov.b	%d3,(%a0)+
	add.w	&1,%d1
	swap %d1
	sub.w	&1,%d0
	bne.b	L%%20
	movm.l	(%sp)+,&0x3ff		# A[12] D[0-7]
	rts
