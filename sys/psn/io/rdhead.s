	#	@(#)Copyright Apple Computer 1987	Version 1.3 of rdhead.s on 87/11/11 21:39:07
/*	@(#)rdhead.s	UniPlus VVV.2.1.4 */
	file	"rdhead.s"
#define LOCORE
#include <sys/uconfig.h>
	set 	DiskQ6L,0x1800
	set	MustFindCnt,8000
	set	S%1,-32
	data	1
	comm	selwait,4
	even
	global	adrmks,DNib
adrmks:
	byte	0xD5
	byte	0xAA
	byte	0x96
	byte	0xDE
	byte	0xAA
	byte	0xFF
DNib:	
	byte	0x00,0x01,0xFF,0xFF,0x02,0x03
	byte	0xFF,0x04,0x05,0x06,0xFF,0xFF
	byte	0xFF,0xFF,0xFF,0xFF,0x07,0x08
	byte	0xFF,0xFF,0xFF,0x09,0x0A,0x0B
	byte	0x0C,0x0D,0xFF,0xFF,0x0E,0x0F
	byte	0x10,0x11,0x12,0x13,0xFF,0x14
	byte	0x15,0x16,0x17,0x18,0x19,0x1A
	byte	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
	byte	0xFF,0xFF,0xFF,0xFF,0xFF,0x1B
	byte	0xFF,0x1C,0x1D,0x1E,0xFF,0xFF
	byte	0xFF,0x1F,0xFF,0xFF,0x20,0x21
	byte	0xFF,0x22,0x23,0x24,0x25,0x26
	byte	0x27,0x28,0xFF,0xFF,0xFF,0xFF
	byte	0xFF,0x29,0x2A,0x2B,0xFF,0x2C
	byte	0x2D,0x2E,0x2F,0x30,0x31,0x32
	byte	0xFF,0xFF,0x33,0x34,0x35,0x36
	byte	0x37,0x38,0xFF,0x39,0x3A,0x3B
	byte	0x3C,0x3D,0x3E,0x3F
	text
	global	rdhead, RdAddr

/*	rdhead -- read address marks from disk.
 *	Called from C
 *	     stat = rdhead(buf)
 *	output if stat == 0:
 *		buf word 0 = <side>/<track>
 *		buf +2 = secnum (byte)
 *		buf +3 = <volume> (byte)
 */

/*	RdAddr -- read address marks from disk
 *	Called from fmt.s
 *	returns d0.w == secnum
 *		d1 bits 0-11 == <side>/<track>
 */
rdhead:
	link	%fp,&S%1
	movm.l	&0x30F8,S%1(%fp)	# A[54] D[76543]
	mov.w	&1,-2(%fp)		# flag that we are "rdhead()"
	bra.b	L%%0
RdAddr:					# for the formatting code
	link	%fp,&S%1
	movm.l	&0x30F8,S%1(%fp)	# A[54] D[76543]
	clr.w	-2(%fp)
L%%0:
	mov.l	&adrmks,%a4
	mov.l	&3,%d4
	mov.l	&0x300,%d5
	mov.l	&0,%d6
	mov.l	iwm_addr,%a5
	add.l	&DiskQ6L,%a5
L%%1:
	mov.b	(%a5),%d1		# try to find 3 nibbles in
	dbra	%d5,L%%3		# an ample number of attempts
	mov.l	&-6,%d0			# error if we can't (prob. blank)
	bra	RdAddrExit
L%%3:
	bpl.b	L%%1			# no nibbles if +
	sub.w	&1,%d4			# found one, decrement count
	bne.b	L%%1
	mov.w	&MustFindCnt-32,%d0	# track has data, look for header

RdAddr1:
	mov.l	%a4,%a0			# copy pointer to AdrMks
	mov.l	&3,%d1			# look for 3 address marks
L%%4:
	mov.b	(%a5),%d6
	bpl.b	L%%4			# wait until get nibble
L%%5:
	dbra	%d0,L%%6
	mov.l	&-5,%d0
	bra.b	RdAddrExit		# error after 1500 tries
L%%6:

	cmp.b	%d6,(%a0)+		# see if it's an addr. mark
	bne.b	RdAddr1
	sub.l	&1,%d1			# dec. count of marks to find
	bne.b	L%%4
	mov.l	&DNib,%a1		# nibble xlat table
	sub.l	&0x96,%a1		# <DNib - 0x96>
	mov.w	%d0,DskErr		# save remainder for formatting

RdAddrMk:				# found 3 address marks,read header
	mov.b	(%a5),%d6		# read nibble
	bpl.b	RdAddrMk
	mov.b	0(%a1,%d6.w),%d1	# get xlat'ed byte into d1
	mov.b	%d1,%d7			# d7 has checksum

	ror.w	&6,%d1
L%%7:
	mov.b	(%a5),%d6
	bpl.b	L%%7
	mov.b	0(%a1,%d6.w),%d4	# sector number

	eor.b	%d4,%d7
L%%8:
	mov.b	(%a5),%d6
	bpl.b	L%%8
	mov.b	0(%a1,%d6.w),%d1
	eor.b	%d1,%d7
	rol.w	&6,%d1			# bits 0-11 = <side><track>

L%%9:
	mov.b	(%a5),%d6		# read volume byte
	bpl.b	L%%9
	mov.b	0(%a1,%d6.w),%d3

	eor.b	%d3,%d7
L%%10:
	mov.b	(%a5),%d6
	bpl.b	L%%10
	mov.b	0(%a1,%d6.w),%d6

	eor.b	%d6,%d7
	bne.b	BadCkSum
	mov.l	&1,%d5
RdAdrEnd:				# look for slip marks
	mov.b	(%a5),%d6
	bpl.b	RdAdrEnd

	cmp.b	%d6,(%a0)+
	bne.b	NoSlip
	dbra	%d5,RdAdrEnd
	mov.l	&0,%d0			# no errors

RdAddrExit:
	tst.l	%d0
	bne	L%%12			# if errors, don't return info
	tst.w	-2(%fp)
	bne	L%%11			# if called from C
	mov.w	%d4,%d0
	bra	L%%12
L%%11:
	mov.l	8(%fp),%a0
	mov.w	%d1,(%a0)		# <side> / <track>
	mov.b	%d4,2(%a0)		# current sector
	mov.b	%d3,3(%a0)		# <volume>
L%%12:
	movm.l	S%1(%fp),&0x30F8
	unlk	%fp
	rts

BadCkSum:
	mov.l	&-4,%d0
	bra	RdAddrExit
NoSlip:
	mov.l	&-3,%d0
	bra.b	RdAddrExit
