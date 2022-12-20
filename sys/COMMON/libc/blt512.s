	#	@(#)Copyright Apple Computer 1987	Version 1.3 of blt512.s on 87/11/11 21:16:08
	file	"blt512.s"
	#	@(#)blt512.s	UniPlus 2.1.1
	# Block transfer subroutine: blt512(destination, source, count)

	global	blt512
	text	

blt512:
	mov.l	4(%sp),%a0		# destination
	mov.l	8(%sp),%a1		# source
	mov.l	12(%sp),%d0		# count
	sub.w	&1,%d0		# pre decrement
	movm.l	&0x7F3E,-(%sp)		# save some registers
LS%1:	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,48(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,96(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,144(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,192(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,240(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,288(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,336(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,384(%a0)
	movm.l	(%a1)+,&0x7CFE		# block move via various registers
	movm.l	&0x7CFE,432(%a0)
	add.w	&480,%a0		# moveml won't let me auto inc a destination
	mov.l	(%a1)+,(%a0)+		# copy
	mov.l	(%a1)+,(%a0)+		# copy
	mov.l	(%a1)+,(%a0)+		# copy
	mov.l	(%a1)+,(%a0)+		# copy
	mov.l	(%a1)+,(%a0)+		# copy
	mov.l	(%a1)+,(%a0)+		# copy
	mov.l	(%a1)+,(%a0)+		# copy
	mov.l	(%a1)+,(%a0)+		# copy
	dbra	%d0,LS%1		# while alignment count
	movm.l	(%sp)+,&0x7CFE		# restore registers
	rts	
