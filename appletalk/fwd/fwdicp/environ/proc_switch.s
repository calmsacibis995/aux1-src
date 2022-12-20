#	@(#)Copyright Apple Computer 1987	Version 1.3 of proc_switch.s on 87/11/13 13:29:47
	global	proc_switch
proc_switch:
	mov.l	8(%sp), %a0
	movm.l	&0x3f3e, -(%sp)	#	<d2-d7, a2-a6> 	44 bytes
	mov.w	%sr, -(%sp)	#			2 bytes
	mov.l	%sp, (%a0)
	mov.l	50(%sp), %sp
	mov.w	(%sp)+, %sr
	movm.l	(%sp)+, &0x7cfc
	rts
