	#	@(#)Copyright Apple Computer 1987	Version 1.2 of bnetivec.s on 87/05/11 14:49:26
 #/*	@(#)bnetivec.s	UniPlus VVV.2.1.11	*/
	file "bnetivec.s"

#define LOCORE
#include	"mch.h"
#include "sys/param.h"
#include "sys/page.h"
#include "sys/uconfig.h"
#include "sys/sysmacros.h"
#include "sys/mmu.h"
#include "sys/seg.h"

 # Copyright 1983 UniSoft Corporation

 # Interrupt vector dispatch table
 # One entry per interrupt vector location

	text
	global ivect
ivect:
	bsr	lzero%		# 0	Reset: Initial SP
	bsr	lfault%		# 1	Reset: Initial PC
	bsr	lbuserr%		# 2	Bus Error
	bsr	laddrerr%		# 3	Address Error
	bsr	lfault%		# 4	Illegal Instruction
	bsr	lfault%		# 5	Zero Divide
	bsr	lfault%		# 6	CHK Instruction
	bsr	lfault%		# 7	TRAPV Instruction
	bsr	lfault%		# 8	Privilege Violation
	bsr	lfault%		# 9	Trace
#ifdef PMMU || NEW_PMMU
	br	lineA%		# 10	Line 1010 Emulator
lineAnext%:
#else
	bsr	lfault%		# 10	Line 1010 Emulator
#endif PMMU
	bsr	lfault%		# 11	Line 1111 Emulator
	bsr	lfault%		# 12	(Unassigned, reserved)
	bsr	lfault%		# 13	(Unassigned, reserved)
	bsr	lfault%		# 14	(Unassigned, reserved)
	bsr	lfault%		# 15	(Unassigned, reserved)
	bsr	lfault%		# 16	(Unassigned, reserved)
	bsr	lfault%		# 17	(Unassigned, reserved)
	bsr	lfault%		# 18	(Unassigned, reserved)
	bsr	lfault%		# 19	(Unassigned, reserved)
	bsr	lfault%		# 20	(Unassigned, reserved)
	bsr	lfault%		# 21	(Unassigned, reserved)
	bsr	lfault%		# 22	(Unassigned, reserved)
	bsr	lfault%		# 23	(Unassigned, reserved)
	bsr	spur0%		# 24	Spurious Interrupt
	bsr	via10%		# 25	Level 1 Interrupt Autovector
	bsr	via21%		# 26	Level 2 Interrupt Autovector
	bsr	lfault%		# 27	Level 3 Interrupt Autovector
	bsr	sc0%		# 28	Level 4 Interrupt Autovector
	bsr	lfault%		# 29	Level 5 Interrupt Autovector
	bsr	pw0%		# 30	Level 6 Interrupt Autovector
	bsr	ab0%		# 31	Level 7 Interrupt Autovector
	bsr	lsyscll0%		# 32	System call
	bsr	lfault%		# 33	TRAP Instruction Vector
	bsr	lfault%		# 34	TRAP Instruction Vector
	bsr	lfault%		# 35	TRAP Instruction Vector
	bsr	lfault%		# 36	TRAP Instruction Vector
	bsr	lfault%		# 37	TRAP Instruction Vector
	bsr	lfault%		# 38	TRAP Instruction Vector
	bsr	lfault%		# 39	TRAP Instruction Vector
	bsr	lfault%		# 40	TRAP Instruction Vector
	bsr	lfault%		# 41	TRAP Instruction Vector
	bsr	lfault%		# 42	TRAP Instruction Vector
	bsr	lfault%		# 43	TRAP Instruction Vector
	bsr	lfault%		# 44	TRAP Instruction Vector
	bsr	lfault%		# 45	TRAP Instruction Vector
	bsr	lfault%		# 46	TRAP Instruction Vector
	bsr	lsyscll1%		# 47	TRAP Instruction Vector
	bsr	lfault%		# 48	(Unassigned, reserved)
	bsr	lfault%		# 49	(Unassigned, reserved)
	bsr	lfault%		# 50	(Unassigned, reserved)
	bsr	lfault%		# 51	(Unassigned, reserved)
	bsr	lfault%		# 52	(Unassigned, reserved)
	bsr	lfault%		# 53	(Unassigned, reserved)
	bsr	lfault%		# 54	(Unassigned, reserved)
	bsr	lfault%		# 55	(Unassigned, reserved)
	bsr	lfault%		# 56	(Unassigned, reserved)
	bsr	lfault%		# 57	(Unassigned, reserved)
	bsr	lfault%		# 58	(Unassigned, reserved)
	bsr	lfault%		# 59	(Unassigned, reserved)
	bsr	lfault%		# 60	(Unassigned, reserved)
	bsr	lfault%		# 61	(Unassigned, reserved)
	bsr	lfault%		# 62	(Unassigned, reserved)
	bsr	lfault%		# 63	(Unassigned, reserved)
	bsr	lfault%		# 64	(User Interrupt Vector)
	bsr	lfault%		# 65	(User Interrupt Vector)
	bsr	lfault%		# 66	(User Interrupt Vector)
	bsr	lfault%		# 67	(User Interrupt Vector)
	bsr	lfault%		# 68	(User Interrupt Vector)
	bsr	lfault%		# 69	(User Interrupt Vector)
	bsr	lfault%		# 70	(User Interrupt Vector)
	bsr	lfault%		# 71	(User Interrupt Vector)
	bsr	lfault%		# 72	(User Interrupt Vector)
	bsr	lfault%		# 73	(User Interrupt Vector)
	bsr	lfault%		# 74	(User Interrupt Vector)
	bsr	lfault%		# 75	(User Interrupt Vector)
	bsr	lfault%		# 76	(User Interrupt Vector)
	bsr	lfault%		# 77	(User Interrupt Vector)
	bsr	lfault%		# 78	(User Interrupt Vector)
	bsr	lfault%		# 79	(User Interrupt Vector)
	bsr	lfault%		# 80	(User Interrupt Vector)
	bsr	lfault%		# 81	(User Interrupt Vector)
	bsr	lfault%		# 82	(User Interrupt Vector)
	bsr	lfault%		# 83	(User Interrupt Vector)
	bsr	lfault%		# 84	(User Interrupt Vector)
	bsr	lfault%		# 85	(User Interrupt Vector)
	bsr	lfault%		# 86	(User Interrupt Vector)
	bsr	lfault%		# 87	(User Interrupt Vector)
	bsr	lfault%		# 88	(User Interrupt Vector)
	bsr	lfault%		# 89	(User Interrupt Vector)
	bsr	lfault%		# 90	(User Interrupt Vector)
	bsr	lfault%		# 91	(User Interrupt Vector)
	bsr	lfault%		# 92	(User Interrupt Vector)
	bsr	lfault%		# 93	(User Interrupt Vector)
	bsr	lfault%		# 94	(User Interrupt Vector)
	bsr	lfault%		# 95	(User Interrupt Vector)
	bsr	lfault%		# 96	(User Interrupt Vector)
	bsr	lfault%		# 97	(User Interrupt Vector)
	bsr	lfault%		# 98	(User Interrupt Vector)
	bsr	lfault%		# 99	(User Interrupt Vector)
	bsr	lfault%		# 100	(User Interrupt Vector)
	bsr	lfault%		# 101	(User Interrupt Vector)
	bsr	lfault%		# 102	(User Interrupt Vector)
	bsr	lfault%		# 103	(User Interrupt Vector)
	bsr	lfault%		# 104	(User Interrupt Vector)
	bsr	lfault%		# 105	(User Interrupt Vector)
	bsr	lfault%		# 106	(User Interrupt Vector)
	bsr	lfault%		# 107	(User Interrupt Vector)
	bsr	lfault%		# 108	(User Interrupt Vector)
	bsr	lfault%		# 109	(User Interrupt Vector)
	bsr	lfault%		# 110	(User Interrupt Vector)
	bsr	lfault%		# 111	(User Interrupt Vector)
	bsr	lfault%		# 112	(User Interrupt Vector)
	bsr	lfault%		# 113	(User Interrupt Vector)
	bsr	lfault%		# 114	(User Interrupt Vector)
	bsr	lfault%		# 115	(User Interrupt Vector)
	bsr	lfault%		# 116	(User Interrupt Vector)
	bsr	lfault%		# 117	(User Interrupt Vector)
	bsr	lfault%		# 118	(User Interrupt Vector)
	bsr	lfault%		# 119	(User Interrupt Vector)
	bsr	lfault%		# 120	(User Interrupt Vector)
	bsr	lfault%		# 121	(User Interrupt Vector)
	bsr	lfault%		# 122	(User Interrupt Vector)
	bsr	lfault%		# 123	(User Interrupt Vector)
	bsr	lfault%		# 124	(User Interrupt Vector)
	bsr	lfault%		# 125	(User Interrupt Vector)
	bsr	lfault%		# 126	(User Interrupt Vector)
	bsr	lfault%		# 127	(User Interrupt Vector)
	bsr	lfault%		# 128	(User Interrupt Vector)
	bsr	lfault%		# 129	(User Interrupt Vector)
	bsr	lfault%		# 130	(User Interrupt Vector)
	bsr	lfault%		# 131	(User Interrupt Vector)
	bsr	lfault%		# 132	(User Interrupt Vector)
	bsr	lfault%		# 133	(User Interrupt Vector)
	bsr	lfault%		# 134	(User Interrupt Vector)
	bsr	lfault%		# 135	(User Interrupt Vector)
	bsr	lfault%		# 136	(User Interrupt Vector)
	bsr	lfault%		# 137	(User Interrupt Vector)
	bsr	lfault%		# 138	(User Interrupt Vector)
	bsr	lfault%		# 139	(User Interrupt Vector)
	bsr	lfault%		# 140	(User Interrupt Vector)
	bsr	lfault%		# 141	(User Interrupt Vector)
	bsr	lfault%		# 142	(User Interrupt Vector)
	bsr	lfault%		# 143	(User Interrupt Vector)
	bsr	lfault%		# 144	(User Interrupt Vector)
	bsr	lfault%		# 145	(User Interrupt Vector)
	bsr	lfault%		# 146	(User Interrupt Vector)
	bsr	lfault%		# 147	(User Interrupt Vector)
	bsr	lfault%		# 148	(User Interrupt Vector)
	bsr	lfault%		# 149	(User Interrupt Vector)
	bsr	lfault%		# 150	(User Interrupt Vector)
	bsr	lfault%		# 151	(User Interrupt Vector)
	bsr	lfault%		# 152	(User Interrupt Vector)
	bsr	lfault%		# 153	(User Interrupt Vector)
	bsr	lfault%		# 154	(User Interrupt Vector)
	bsr	lfault%		# 155	(User Interrupt Vector)
	bsr	lfault%		# 156	(User Interrupt Vector)
	bsr	lfault%		# 157	(User Interrupt Vector)
	bsr	lfault%		# 158	(User Interrupt Vector)
	bsr	lfault%		# 159	(User Interrupt Vector)
	bsr	lfault%		# 160	(User Interrupt Vector)
	bsr	lfault%		# 161	(User Interrupt Vector)
	bsr	lfault%		# 162	(User Interrupt Vector)
	bsr	lfault%		# 163	(User Interrupt Vector)
	bsr	lfault%		# 164	(User Interrupt Vector)
	bsr	lfault%		# 165	(User Interrupt Vector)
	bsr	lfault%		# 166	(User Interrupt Vector)
	bsr	lfault%		# 167	(User Interrupt Vector)
	bsr	lfault%		# 168	(User Interrupt Vector)
	bsr	lfault%		# 169	(User Interrupt Vector)
	bsr	lfault%		# 170	(User Interrupt Vector)
	bsr	lfault%		# 171	(User Interrupt Vector)
	bsr	lfault%		# 172	(User Interrupt Vector)
	bsr	lfault%		# 173	(User Interrupt Vector)
	bsr	lfault%		# 174	(User Interrupt Vector)
	bsr	lfault%		# 175	(User Interrupt Vector)
	bsr	lfault%		# 176	(User Interrupt Vector)
	bsr	lfault%		# 177	(User Interrupt Vector)
	bsr	lfault%		# 178	(User Interrupt Vector)
	bsr	lfault%		# 179	(User Interrupt Vector)
	bsr	lfault%		# 180	(User Interrupt Vector)
	bsr	lfault%		# 181	(User Interrupt Vector)
	bsr	lfault%		# 182	(User Interrupt Vector)
	bsr	lfault%		# 183	(User Interrupt Vector)
	bsr	lfault%		# 184	(User Interrupt Vector)
	bsr	lfault%		# 185	(User Interrupt Vector)
	bsr	lfault%		# 186	(User Interrupt Vector)
	bsr	lfault%		# 187	(User Interrupt Vector)
	bsr	lfault%		# 188	(User Interrupt Vector)
	bsr	lfault%		# 189	(User Interrupt Vector)
	bsr	lfault%		# 190	(User Interrupt Vector)
	bsr	lfault%		# 191	(User Interrupt Vector)
	bsr	lfault%		# 192	(User Interrupt Vector)
	bsr	lfault%		# 193	(User Interrupt Vector)
	bsr	lfault%		# 194	(User Interrupt Vector)
	bsr	lfault%		# 195	(User Interrupt Vector)
	bsr	lfault%		# 196	(User Interrupt Vector)
	bsr	lfault%		# 197	(User Interrupt Vector)
	bsr	lfault%		# 198	(User Interrupt Vector)
	bsr	lfault%		# 199	(User Interrupt Vector)
	bsr	lfault%		# 200	(User Interrupt Vector)
	bsr	lfault%		# 201	(User Interrupt Vector)
	bsr	lfault%		# 202	(User Interrupt Vector)
	bsr	lfault%		# 203	(User Interrupt Vector)
	bsr	lfault%		# 204	(User Interrupt Vector)
	bsr	lfault%		# 205	(User Interrupt Vector)
	bsr	lfault%		# 206	(User Interrupt Vector)
	bsr	lfault%		# 207	(User Interrupt Vector)
	bsr	lfault%		# 208	(User Interrupt Vector)
	bsr	lfault%		# 209	(User Interrupt Vector)
	bsr	lfault%		# 210	(User Interrupt Vector)
	bsr	lfault%		# 211	(User Interrupt Vector)
	bsr	lfault%		# 212	(User Interrupt Vector)
	bsr	lfault%		# 213	(User Interrupt Vector)
	bsr	lfault%		# 214	(User Interrupt Vector)
	bsr	lfault%		# 215	(User Interrupt Vector)
	bsr	lfault%		# 216	(User Interrupt Vector)
	bsr	lfault%		# 217	(User Interrupt Vector)
	bsr	lfault%		# 218	(User Interrupt Vector)
	bsr	lfault%		# 219	(User Interrupt Vector)
	bsr	lfault%		# 220	(User Interrupt Vector)
	bsr	lfault%		# 221	(User Interrupt Vector)
	bsr	lfault%		# 222	(User Interrupt Vector)
	bsr	lfault%		# 223	(User Interrupt Vector)
	bsr	lfault%		# 224	(User Interrupt Vector)
	bsr	lfault%		# 225	(User Interrupt Vector)
	bsr	lfault%		# 226	(User Interrupt Vector)
	bsr	lfault%		# 227	(User Interrupt Vector)
	bsr	lfault%		# 228	(User Interrupt Vector)
	bsr	lfault%		# 229	(User Interrupt Vector)
	bsr	lfault%		# 230	(User Interrupt Vector)
	bsr	lfault%		# 231	(User Interrupt Vector)
	bsr	lfault%		# 232	(User Interrupt Vector)
	bsr	lfault%		# 233	(User Interrupt Vector)
	bsr	lfault%		# 234	(User Interrupt Vector)
	bsr	lfault%		# 235	(User Interrupt Vector)
	bsr	lfault%		# 236	(User Interrupt Vector)
	bsr	lfault%		# 237	(User Interrupt Vector)
	bsr	lfault%		# 238	(User Interrupt Vector)
	bsr	lfault%		# 239	(User Interrupt Vector)
	bsr	lfault%		# 240	(User Interrupt Vector)
	bsr	lfault%		# 241	(User Interrupt Vector)
	bsr	lfault%		# 242	(User Interrupt Vector)
	bsr	lfault%		# 243	(User Interrupt Vector)
	bsr	lfault%		# 244	(User Interrupt Vector)
	bsr	lfault%		# 245	(User Interrupt Vector)
	bsr	lfault%		# 246	(User Interrupt Vector)
	bsr	lfault%		# 247	(User Interrupt Vector)
	bsr	lfault%		# 248	(User Interrupt Vector)
	bsr	lfault%		# 249	(User Interrupt Vector)
	bsr	lfault%		# 250	(User Interrupt Vector)
	bsr	lfault%		# 251	(User Interrupt Vector)
	bsr	lfault%		# 252	(User Interrupt Vector)
	bsr	lfault%		# 253	(User Interrupt Vector)
	bsr	lfault%		# 254	(User Interrupt Vector)
	bsr	lfault%		# 255	(User Interrupt Vector)

 # Put actual "C" routine name onto the stack
 # and call the system interrupt dispatcher

	global	fault%, buserr%, addrerr%, call%, syscall0%, syscall1%, idleflg

lfault%:  jmp	fault%
lbuserr%: jmp	buserr%
laddrerr%:jmp	addrerr%
lsyscll0%:jmp	syscall0%

lsyscll1%:jmp	syscall1%

	global	zerofunc
lzero%:	mov.l	&zerofunc,(%sp)
	mov.w	idleflg,-(%sp)
	jmp	call%

	global	spurintr
spur0%:	mov.l	&spurintr,(%sp)
	mov.w	&0,-(%sp)
	jmp	call%

	global	Xfdbint
via10%:	mov.w	&0x2400, %sr		# Make sure we go thru call% if req.
	jmp	Xfdbint

 	global	via2intr
via21%:	mov.l	&via2intr,(%sp)
 	mov.w	idleflg,-(%sp)
 	jmp	call%

	global	scintr
sc0%:	mov.l	&scintr,(%sp)
	mov.w	&0,-(%sp)
	jmp	call%

	global	abintr
ab0%:	mov.l	&abintr,(%sp)
	mov.w	&0,-(%sp)
	jmp	call%

	global	powerintr
pw0%:	mov.l	&powerintr,(%sp)
	mov.w	&0,-(%sp)
	jmp	call%

#ifdef PMMU || NEW_PMMU
lineA%: mov.w	&0x2700, %sr		# Disable interrupts
	tst.l	UDOT+U_USER%		# do we have a pc to go to?
	beq	lineAfail%

			    # Now, we need to flush the 68020 cache.
			    # This may be temporary.  If it is not, we 
			    # should get rid of the movec from the CACR,
			    # and just know what the other CACR bits should
			    # be set to.
	mov.l	%d1, -(%a7)		# We need d1 for movec instructions
	short	0x4e7a,0x1002		# movec %CACR, %d1
	short	0x08c1,0x0003		# bset &3, %d1
	short	0x4e7b,0x1002		# movec %d1, %CACR
	mov.l	(%a7)+, %d1		# Restore d1

	tst.l	runrun			# Did a reschedule occur?
	bne	lineAall%
	tst.b	qrunflag		# Are queues runnable?
	bne	lineAall%
	tst.l	sir			# Are there software interrupts pending?
	beq	lineAok%
lineAall%:				# For some reason (system activity, page
	mov.w	&0x2000, %sr		# 	not present etc) we have to go
	mov.l	&lineA, -(%sp)		#	through the standard Unix fault
	mov.w	&0, -(%sp)		#	mechanism, so do it
	jmp	call%
lineAfail%:				# the call failed ... make like a
	mov.w	&0x2000, %sr		# normal A-line trap
	mov.l	&lineAnext%, -(%sp)
	jmp	fault%
lineAok%:				# do it inline
	mov.l	%a0, -(%a7)		# save a0
	#
	#	Stack
	#6	pc
	#4	sr
	#0	a0
	#
	mov.l	UDOT+U_USER%, %a0	# get ptr to c_layer structure
	mov.l	6(%sp), 4(%a0)		# place address of aline in c_return
	mov.l	(%a0), 6(%sp)		# place new user pc in stack frame
	mov.l	(%a7)+, %a0		# restore a0
	rte
	global 	lineAfault
lineAfault:
	mov.l	4(%sp), %sp
	mov.l	60(%sp),%a0
	mov.l	%a0,%usp		# restore usr stack ptr
	movm.l	(%sp)+,&0x7FFF		# restore all other registers
	add.w	&10,%sp			# sp, pop fault pc, and alignment word
	mov.l	&lineAnext%, -(%sp)
	jmp	fault%

#endif PMMU
