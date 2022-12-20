#	@(#)Copyright Apple Computer 1987	Version 1.2 of at_mch.s on 87/11/11 21:07:44 _mch.s	VV.2.1.1

#	Copyright (c) 1986 UniSoft Systems
#	All Rights Reserved
#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft
#	The copyright notice above does not evidence any  
#	actual or intended publication of such source code.

#ident	"@(#)at_mch.s 1.1 87/04/14"

	global	atrcv1, newsccvec
	set	scc1bc,0xb0000
	set	scc1bd,0xb0002
	set	scc2bc,0xb4000
	set	scc2bd,0xb4002
	text

newsccvec:
	mov.w	%sr, %d0
	mov.w	&0x2700,%sr
	tst.l	4(%sp)
	bne	newsccvec1
	mov.l	8(%sp), sccvec1		# for scc 1
	bne	newsccvec2
newsccvec1:
	mov.l	8(%sp), sccvec2		# for scc 2
newsccvec2:
	mov.w	%d0, %sr
	rts




#	This function is used to receive characters via RIA interrupts. Since
#	it takes ? out of ? cycles at 7mhz, time is of the importence. Though
#	the SCC has a three byte fifo, we should not rely on it too much.
#	NOTE: lower priority SCC running AT should disable interrupts during
#	these routines.

atrcv1:
				# 47
	movm.l	&0xc0c0,-(%sp)	# save registers
	mov.l	&scc1bc,%a0	# ?? get address of scc control port.
	btst	&0,(%a0)	# ?? read RR0 to see if there is a character
	beq	atrcv13		#  8 must be something else, let c find it

	mov.l	at_rwp,%a1	# 16 get buffer pointer
	mov.b	scc1bd,(%a1)+	# 21 get character from scc
	mov.l	%a1,at_rwp	# 18 update pointer
atrcv10:
	mov.w	&20,%d0		#    intercharacter timeout
atrcv11:
	btst	&0,(%a0)	# ?? read RR0 to see if there is a character
	beq	atrcv12
	cmp.l	%a1,at_rwp+4	# 18 see if we have read too many already.
	bge	atrcv14		#  8
	mov.b	scc1bd, (%a1)+	# 21 get character from scc
	mov.l	%a1,at_rwp	# 18 update pointer
	bra	atrcv10		#    read next character
atrcv12:			#    no data yet
	mov.b	&1,(%a0)	# 20 read RR1
	mov.w	&1234,%d1	#    keep scc access separate
	mov.b	(%a0),%d1
	btst	&5,%d1		# ?? 
	bne	atrcv15		#  8 Overrun?
	btst	&7,%d1		# ?? 
	bne	atrcv16		#  8 EOF?
	dbf	%d0, atrcv11	# have we timed out for the character?

	mov.l	&5, -(%sp)		# RCV_TIMEOUT
	bra	atrcv19
atrcv13:
	mov.l	&0, -(%sp)		# NO_RCV
	bra	atrcv19
atrcv14:
	mov.l	&1, -(%sp)		# RCV_TOO_LARGE
	bra	atrcv19
atrcv15:
	mov.l	&2, -(%sp)		# RCV_OVERRUN
	bra	atrcv19
atrcv16:
	btst	&6,%d1			# CRC?
	beq	atrcv17
	mov.l	&3, -(%sp)		# RCV_CRC
	bra	atrcv19
atrcv17:			# flush the CRC (update the pointers)
	btst	&0,(%a0)	# ?? read RR0 to see if there is a character
	beq	atrcv17a
	cmp.l	%a1,at_rwp+4	# 18 see if we have read too many already.
	bge	atrcv14		#  8
	mov.b	scc1bd, (%a1)+	# 21 get character from scc
	mov.l	%a1,at_rwp	# 18 update pointer
	btst	&0,(%a0)	# ?? read RR0 to see if there is a character
	beq	atrcv17a
	cmp.l	%a1,at_rwp+4	# 18 see if we have read too many already.
	bge	atrcv14		#  8
	mov.b	scc1bd, (%a1)+	# 21 get character from scc
	mov.l	%a1,at_rwp	# 18 update pointer
atrcv17a:
	mov.l	&4, -(%sp)		# RCV_OK
	bra	atrcv19
atrcv19:
	mov.l	&1, -(%sp)		# channel B
	mov.l	scc_vecs+4, %a0		# get function array
	jsr	(%a0)
	add.l	&8, %sp
	movm.l	(%sp)+,&0x0303		# restore registers
	rte



	global	scc_cmd
scc_cmd:

	mov.l	8(%sp),%a1		# scc_ctl_write = sccp->scc_ctl_write;
	mov.l	4(%a1),%a1
	mov.l	4(%sp),%a0		# pointer to tbl
cmd1:
	mov.b	(%a0)+,(%a1)		# *scc_ctl_write = tbl->scc_register_number;
	mov.b	(%a0)+,(%a1)		# *scc_ctl_write = tbl->scc_register_value;
	cmp.b	(%a0),&128		# while not SCC_EOT
	bne	cmd1
	rts
