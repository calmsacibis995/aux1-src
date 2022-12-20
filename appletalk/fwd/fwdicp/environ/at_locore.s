#	@(#)Copyright Apple Computer 1987	Version 1.2 of at_locore.s on 87/11/11 21:07:40 VV.2.1.1

#	Copyright (c) 1986 UniSoft Systems
#	All Rights Reserved
#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft
#	The copyright notice above does not evidence any  
#	actual or intended publication of such source code.

#ident	"@(#)at_locore.s 1.2 87/11/11"

	global	sccvec1, sccvec2, scc1i, scc2i
	text
	long	stackend	# 0	Reset: Initial SP
	long	reset		# 1	Reset: Initial PC
	long	lbuserr%	# 2	Bus Error
	long	laddrerr%	# 3	Address Error
	long	lillinstru%	# 4	Illegal Instruction
	long	lzerodiv%	# 5	Zero Divide
	long	lfault%		# 6	CHK Instruction
	long	lfault%		# 7	TRAPV Instruction
	long	lfault%		# 8	Privilege Violation
	long	lfault%		# 9	Trace
	long	lfault%		# 10	Line 1010 Emulator
	long	lfault%		# 11	Line 1111 Emulator
	long	lfault%		# 12	(Unassigned, reserved)
	long	lfault%		# 13	(Unassigned, reserved)
	long	lfault%		# 14	(Unassigned, reserved)
	long	lfault%		# 15	(Unassigned, reserved)
	long	lfault%		# 16	(Unassigned, reserved)
	long	lfault%		# 17	(Unassigned, reserved)
	long	lfault%		# 18	(Unassigned, reserved)
	long	lfault%		# 19	(Unassigned, reserved)
	long	lfault%		# 20	(Unassigned, reserved)
	long	lfault%		# 21	(Unassigned, reserved)
	long	lfault%		# 22	(Unassigned, reserved)
	long	lfault%		# 23	(Unassigned, reserved)
	long	lspur%		# 24	Spurious Interrupt
	long	nbus%		# 25	Level 1 Interrupt Autovector
	long	lfault%		# 26	Level 2 Interrupt Autovector
	long	via%		# 27	Level 3 Interrupt Autovector
sccvec2:long	scc2i		# 28	Level 4 Interrupt Autovector
sccvec1:long	scc1i		# 29	Level 5 Interrupt Autovector
	long	lfault%		# 30	Level 6 Interrupt Autovector
	long	lfault%		# 31	Level 7 Interrupt Autovector
	long	lfault%		# 32	System call
	long	lfault%		# 33	TRAP Instruction Vector
	long	lfault%		# 34	TRAP Instruction Vector
	long	lfault%		# 35	TRAP Instruction Vector
	long	lfault%		# 36	TRAP Instruction Vector
	long	lfault%		# 37	TRAP Instruction Vector
	long	lfault%		# 38	TRAP Instruction Vector
	long	lfault%		# 39	TRAP Instruction Vector
	long	lfault%		# 40	TRAP Instruction Vector
	long	lfault%		# 41	TRAP Instruction Vector
	long	lfault%		# 42	TRAP Instruction Vector
	long	lfault%		# 43	TRAP Instruction Vector
	long	lfault%		# 44	TRAP Instruction Vector
	long	lfault%		# 45	TRAP Instruction Vector
	long	lfault%		# 46	TRAP Instruction Vector
	long	lfault%		# 47	TRAP Instruction Vector
	long	lfault%		# 48	(Unassigned, reserved)
	long	lfault%		# 49	(Unassigned, reserved)
	long	lfault%		# 50	(Unassigned, reserved)
	long	lfault%		# 51	(Unassigned, reserved)
	long	lfault%		# 52	(Unassigned, reserved)
	long	lfault%		# 53	(Unassigned, reserved)
	long	lfault%		# 54	(Unassigned, reserved)
	long	lfault%		# 55	(Unassigned, reserved)
	long	lfault%		# 56	(Unassigned, reserved)
	long	lfault%		# 57	(Unassigned, reserved)
	long	lfault%		# 58	(Unassigned, reserved)
	long	lfault%		# 59	(Unassigned, reserved)
	long	lfault%		# 60	(Unassigned, reserved)
	long	lfault%		# 61	(Unassigned, reserved)
	long	lfault%		# 62	(Unassigned, reserved)
	long	lfault%		# 63	(Unassigned, reserved)
	long	lfault%		# 64	(User Interrupt Vector)
	long	lfault%		# 65	(User Interrupt Vector)
	long	lfault%		# 66	(User Interrupt Vector)
	long	lfault%		# 67	(User Interrupt Vector)
	long	lfault%		# 68	(User Interrupt Vector)
	long	lfault%		# 69	(User Interrupt Vector)
	long	lfault%		# 70	(User Interrupt Vector)
	long	lfault%		# 71	(User Interrupt Vector)
	long	lfault%		# 72	(User Interrupt Vector)
	long	lfault%		# 73	(User Interrupt Vector)
	long	lfault%		# 74	(User Interrupt Vector)
	long	lfault%		# 75	(User Interrupt Vector)
	long	lfault%		# 76	(User Interrupt Vector)
	long	lfault%		# 77	(User Interrupt Vector)
	long	lfault%		# 78	(User Interrupt Vector)
	long	lfault%		# 79	(User Interrupt Vector)
	long	lfault%		# 80	(User Interrupt Vector)
	long	lfault%		# 81	(User Interrupt Vector)
	long	lfault%		# 82	(User Interrupt Vector)
	long	lfault%		# 83	(User Interrupt Vector)
	long	lfault%		# 84	(User Interrupt Vector)
	long	lfault%		# 85	(User Interrupt Vector)
	long	lfault%		# 86	(User Interrupt Vector)
	long	lfault%		# 87	(User Interrupt Vector)
	long	lfault%		# 88	(User Interrupt Vector)
	long	lfault%		# 89	(User Interrupt Vector)
	long	lfault%		# 90	(User Interrupt Vector)
	long	lfault%		# 91	(User Interrupt Vector)
	long	lfault%		# 92	(User Interrupt Vector)
	long	lfault%		# 93	(User Interrupt Vector)
	long	lfault%		# 94	(User Interrupt Vector)
	long	lfault%		# 95	(User Interrupt Vector)
	long	lfault%		# 96	(User Interrupt Vector)
	long	lfault%		# 97	(User Interrupt Vector)
	long	lfault%		# 98	(User Interrupt Vector)
	long	lfault%		# 99	(User Interrupt Vector)
	long	lfault%		# 100	(User Interrupt Vector)
	long	lfault%		# 101	(User Interrupt Vector)
	long	lfault%		# 102	(User Interrupt Vector)
	long	lfault%		# 103	(User Interrupt Vector)
	long	lfault%		# 104	(User Interrupt Vector)
	long	lfault%		# 105	(User Interrupt Vector)
	long	lfault%		# 106	(User Interrupt Vector)
	long	lfault%		# 107	(User Interrupt Vector)
	long	lfault%		# 108	(User Interrupt Vector)
	long	lfault%		# 109	(User Interrupt Vector)
	long	lfault%		# 110	(User Interrupt Vector)
	long	lfault%		# 111	(User Interrupt Vector)
	long	lfault%		# 112	(User Interrupt Vector)
	long	lfault%		# 113	(User Interrupt Vector)
	long	lfault%		# 114	(User Interrupt Vector)
	long	lfault%		# 115	(User Interrupt Vector)
	long	lfault%		# 116	(User Interrupt Vector)
	long	lfault%		# 117	(User Interrupt Vector)
	long	lfault%		# 118	(User Interrupt Vector)
	long	lfault%		# 119	(User Interrupt Vector)
	long	lfault%		# 120	(User Interrupt Vector)
	long	lfault%		# 121	(User Interrupt Vector)
	long	lfault%		# 122	(User Interrupt Vector)
	long	lfault%		# 123	(User Interrupt Vector)
	long	lfault%		# 124	(User Interrupt Vector)
	long	lfault%		# 125	(User Interrupt Vector)
	long	lfault%		# 126	(User Interrupt Vector)
	long	lfault%		# 127	(User Interrupt Vector)
	long	lfault%		# 128	(User Interrupt Vector)
	long	lfault%		# 129	(User Interrupt Vector)
	long	lfault%		# 130	(User Interrupt Vector)
	long	lfault%		# 131	(User Interrupt Vector)
	long	lfault%		# 132	(User Interrupt Vector)
	long	lfault%		# 133	(User Interrupt Vector)
	long	lfault%		# 134	(User Interrupt Vector)
	long	lfault%		# 135	(User Interrupt Vector)
	long	lfault%		# 136	(User Interrupt Vector)
	long	lfault%		# 137	(User Interrupt Vector)
	long	lfault%		# 138	(User Interrupt Vector)
	long	lfault%		# 139	(User Interrupt Vector)
	long	lfault%		# 140	(User Interrupt Vector)
	long	lfault%		# 141	(User Interrupt Vector)
	long	lfault%		# 142	(User Interrupt Vector)
	long	lfault%		# 143	(User Interrupt Vector)
	long	lfault%		# 144	(User Interrupt Vector)
	long	lfault%		# 145	(User Interrupt Vector)
	long	lfault%		# 146	(User Interrupt Vector)
	long	lfault%		# 147	(User Interrupt Vector)
	long	lfault%		# 148	(User Interrupt Vector)
	long	lfault%		# 149	(User Interrupt Vector)
	long	lfault%		# 150	(User Interrupt Vector)
	long	lfault%		# 151	(User Interrupt Vector)
	long	lfault%		# 152	(User Interrupt Vector)
	long	lfault%		# 153	(User Interrupt Vector)
	long	lfault%		# 154	(User Interrupt Vector)
	long	lfault%		# 155	(User Interrupt Vector)
	long	lfault%		# 156	(User Interrupt Vector)
	long	lfault%		# 157	(User Interrupt Vector)
	long	lfault%		# 158	(User Interrupt Vector)
	long	lfault%		# 159	(User Interrupt Vector)
	long	lfault%		# 160	(User Interrupt Vector)
	long	lfault%		# 161	(User Interrupt Vector)
	long	lfault%		# 162	(User Interrupt Vector)
	long	lfault%		# 163	(User Interrupt Vector)
	long	lfault%		# 164	(User Interrupt Vector)
	long	lfault%		# 165	(User Interrupt Vector)
	long	lfault%		# 166	(User Interrupt Vector)
	long	lfault%		# 167	(User Interrupt Vector)
	long	lfault%		# 168	(User Interrupt Vector)
	long	lfault%		# 169	(User Interrupt Vector)
	long	lfault%		# 170	(User Interrupt Vector)
	long	lfault%		# 171	(User Interrupt Vector)
	long	lfault%		# 172	(User Interrupt Vector)
	long	lfault%		# 173	(User Interrupt Vector)
	long	lfault%		# 174	(User Interrupt Vector)
	long	lfault%		# 175	(User Interrupt Vector)
	long	lfault%		# 176	(User Interrupt Vector)
	long	lfault%		# 177	(User Interrupt Vector)
	long	lfault%		# 178	(User Interrupt Vector)
	long	lfault%		# 179	(User Interrupt Vector)
	long	lfault%		# 180	(User Interrupt Vector)
	long	lfault%		# 181	(User Interrupt Vector)
	long	lfault%		# 182	(User Interrupt Vector)
	long	lfault%		# 183	(User Interrupt Vector)
	long	lfault%		# 184	(User Interrupt Vector)
	long	lfault%		# 185	(User Interrupt Vector)
	long	lfault%		# 186	(User Interrupt Vector)
	long	lfault%		# 187	(User Interrupt Vector)
	long	lfault%		# 188	(User Interrupt Vector)
	long	lfault%		# 189	(User Interrupt Vector)
	long	lfault%		# 190	(User Interrupt Vector)
	long	lfault%		# 191	(User Interrupt Vector)
	long	lfault%		# 192	(User Interrupt Vector)
	long	lfault%		# 193	(User Interrupt Vector)
	long	lfault%		# 194	(User Interrupt Vector)
	long	lfault%		# 195	(User Interrupt Vector)
	long	lfault%		# 196	(User Interrupt Vector)
	long	lfault%		# 197	(User Interrupt Vector)
	long	lfault%		# 198	(User Interrupt Vector)
	long	lfault%		# 199	(User Interrupt Vector)
	long	lfault%		# 200	(User Interrupt Vector)
	long	lfault%		# 201	(User Interrupt Vector)
	long	lfault%		# 202	(User Interrupt Vector)
	long	lfault%		# 203	(User Interrupt Vector)
	long	lfault%		# 204	(User Interrupt Vector)
	long	lfault%		# 205	(User Interrupt Vector)
	long	lfault%		# 206	(User Interrupt Vector)
	long	lfault%		# 207	(User Interrupt Vector)
	long	lfault%		# 208	(User Interrupt Vector)
	long	lfault%		# 209	(User Interrupt Vector)
	long	lfault%		# 210	(User Interrupt Vector)
	long	lfault%		# 211	(User Interrupt Vector)
	long	lfault%		# 212	(User Interrupt Vector)
	long	lfault%		# 213	(User Interrupt Vector)
	long	lfault%		# 214	(User Interrupt Vector)
	long	lfault%		# 215	(User Interrupt Vector)
	long	lfault%		# 216	(User Interrupt Vector)
	long	lfault%		# 217	(User Interrupt Vector)
	long	lfault%		# 218	(User Interrupt Vector)
	long	lfault%		# 219	(User Interrupt Vector)
	long	lfault%		# 220	(User Interrupt Vector)
	long	lfault%		# 221	(User Interrupt Vector)
	long	lfault%		# 222	(User Interrupt Vector)
	long	lfault%		# 223	(User Interrupt Vector)
	long	lfault%		# 224	(User Interrupt Vector)
	long	lfault%		# 225	(User Interrupt Vector)
	long	lfault%		# 226	(User Interrupt Vector)
	long	lfault%		# 227	(User Interrupt Vector)
	long	lfault%		# 228	(User Interrupt Vector)
	long	lfault%		# 229	(User Interrupt Vector)
	long	lfault%		# 230	(User Interrupt Vector)
	long	lfault%		# 231	(User Interrupt Vector)
	long	lfault%		# 232	(User Interrupt Vector)
	long	lfault%		# 233	(User Interrupt Vector)
	long	lfault%		# 234	(User Interrupt Vector)
	long	lfault%		# 235	(User Interrupt Vector)
	long	lfault%		# 236	(User Interrupt Vector)
	long	lfault%		# 237	(User Interrupt Vector)
	long	lfault%		# 238	(User Interrupt Vector)
	long	lfault%		# 239	(User Interrupt Vector)
	long	lfault%		# 240	(User Interrupt Vector)
	long	lfault%		# 241	(User Interrupt Vector)
	long	lfault%		# 242	(User Interrupt Vector)
	long	lfault%		# 243	(User Interrupt Vector)
	long	lfault%		# 244	(User Interrupt Vector)
	long	lfault%		# 245	(User Interrupt Vector)
	long	lfault%		# 246	(User Interrupt Vector)
	long	lfault%		# 247	(User Interrupt Vector)
	long	lfault%		# 248	(User Interrupt Vector)
	long	lfault%		# 249	(User Interrupt Vector)
	long	lfault%		# 250	(User Interrupt Vector)
	long	lfault%		# 251	(User Interrupt Vector)
	long	lfault%		# 252	(User Interrupt Vector)
	long	lfault%		# 253	(User Interrupt Vector)
	long	lfault%		# 254	(User Interrupt Vector)
	long	lfault%		# 255	(User Interrupt Vector)

 # Put actual "C" routine name onto the stack
 # and call the system interrupt dispatcher

					#
					# the UNexpected interrupts
					#
lfault%:
	mov.w	&0x2700, %sr
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.l	&0,-(%sp)
	jmp	call%
lbuserr%:
	mov.w	&0x2700, %sr
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.l	&2,-(%sp)
	jmp	call%
laddrerr%:
	mov.w	&0x2700, %sr
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.l	&3,-(%sp)
	jmp	call%
lillinstru%:
	mov.w	&0x2700, %sr
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.l	&4,-(%sp)
	jmp	call%
lzerodiv%:
	mov.w	&0x2700, %sr
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.l	&5,-(%sp)
	jmp	call%
lspur%:
	mov.w	&0x2700, %sr
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.l	&24,-(%sp)
	jmp	call%
call%:
	jsr	fault
	add.l	&4, %sp
	movm.l	(%sp)+,&0x0303		# restore registers
	rte


					#
					# the expected interrupts
					#
nbus%:
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.w	&0, -(%sp) 
	mov.l	%sp,%a0			# save argument list pointer
	mov.l	%a0,-(%sp)		# push ap onto stack
	jsr	fwdicpirupt
	add.l	&6, %sp
	movm.l	(%sp)+,&0x0303		# restore registers
	rte
	
scc1i:
	movm.l	&0xC0C0,-(%sp)		# save registers
					# find out which channel A (0) or B (1)
	mov.l	sccs+4,%a0		# move scc_ctl_write
	mov.l	&3,%d0			# set up read from RR3, which IP
	mov.b	%d0,(%a0)
	mov.l	sccs,%a0		# move scc_ctl_write, and wait for scc
	mov.b	(%a0),%d0
	and.l	&7,%d0
	bne.b	scc11%
	mov.w	&0, -(%sp) 		# channel A
	mov.l	%sp,%a0			# save argument list pointer
	mov.l	%a0,-(%sp)		# push ap onto stack
	mov.l	scc_vecs, %a0		# get function array
	jsr	(%a0)
	add.l	&6, %sp
	movm.l	(%sp)+,&0x0303		# restore registers
	rte
scc11%:	mov.w	&1, -(%sp)		# channel B
	mov.l	%sp,%a0			# save argument list pointer
	mov.l	%a0,-(%sp)		# push ap onto stack
	mov.l	scc_vecs+4, %a0		# get function array
	jsr	(%a0)
	add.l	&6, %sp
	movm.l	(%sp)+,&0x0303		# restore registers
	rte
	
scc2i:
	movm.l	&0xC0C0,-(%sp)		# save registers
					# find out which channel A (2) or B (3)
	mov.l	sccs+64+4,%a0		# move scc_ctl_write
	mov.l	&3,%d0			# set up read from RR3, which IP
	mov.b	%d0,(%a0)
	mov.l	sccs+64,%a0		# move scc_ctl_write, and wait for scc
	mov.b	(%a0),%d0
	and.l	&7,%d0
	bne.b	scc21%
	mov.w	&2, -(%sp) 		# channel A
	mov.l	%sp,%a0			# save argument list pointer
	mov.l	%a0,-(%sp)		# push ap onto stack
	mov.l	scc_vecs+8, %a0		# get function array
	jsr	(%a0)
	add.l	&6, %sp
	movm.l	(%sp)+,&0x0303		# restore registers
	rte
scc21%:	mov.w	&3, -(%sp)		# channel B
	mov.l	%sp,%a0			# save argument list pointer
	mov.l	%a0,-(%sp)		# push ap onto stack
	mov.l	scc_vecs+12, %a0		# get function array
	jsr	(%a0)
	add.l	&6, %sp
	movm.l	(%sp)+,&0x0303		# restore registers
	rte

	global 	vias, clock, at_timer_interrupt
via%:
	movm.l	&0xC0C0,-(%sp)		# save registers
	mov.w	&0x2500,%sr		# set priority 5
	mov.l	vias+0, %a0
	btst	&5, 26(%a0)
	beq	clk%
	mov.b	16(%a0), %d0
	jsr	at_timer_interrupt
	mov.l	vias+0, %a0
clk%:
	mov.w	&0x2300, %sr		# set back spl
	btst	&6, 26(%a0)
	beq	clkend%
	mov.b	8(%a0), %d0
	jsr	clock
clkend%:
	movm.l	(%sp)+,&0x0303		# restore registers
	rte

	set	HIGH%,0x2700		# High priority supervisor mode (spl 7)
	set	LOW%,0x2000		# Low priority, supervisor mode (spl 0)

	global	end, edata, reset

reset:
	mov.w	&HIGH%,%sr		# spl7
	mov.l	&stackend,%sp		# 

	mov.l	&end,%d7		# End of bss
	mov.l	&edata,%a0		# Start clearing here
clrbss%:mov.l	&0,(%a0)+		# Clear bss
	cmp.l	%a0,%d7
	bcs	clrbss%

	jsr	main			# call the main ...
	jmp	lfault%			# we should never return ...
	
 # spl commands
	global	splhi, splx, spltty, splclock
	global	spl7, spl6, spl5, spl4, spl3, spl2, spl1, spl0

splhi:
spl7:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2700,%sr		# set priority 7
	rts	
spl6:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2600,%sr		# set priority 6
	rts	
spl5:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2500,%sr		# set priority 5
	rts	
spltty:					# console is output on #2B, so AT runs
spl4:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2400,%sr		# set priority 4
	rts	
splclock:
spl3:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2300,%sr		# set priority 3
	rts	
spl2:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2200,%sr		# set priority 2
	rts	
spl1:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2100,%sr		# set priority 1
	rts	
spl0:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2000,%sr		# set priority 0
	rts	

splx:	mov.w	6(%sp),%sr		# set priority
	rts	
