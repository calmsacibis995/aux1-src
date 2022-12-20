	#	@(#)Copyright Apple Computer 1987	Version 1.2 of local.s on 87/05/11 14:42:48
	file	"local.s"
 #	@(#)local.s	UniPlus VVV.2.1.3

#define LOCORE
#include "sys/param.h"
	global  cputype, runrun, curpri, mtimer, lticks, copypte
	data	

	even
cputype:	long	0		# for 68000, 68010, 68020 id
curpri:		long	0		#
mtimer:		long	0		#
lticks:		long	0		#
copypte:	long	0		#
runrun:		long	0		# for process switching

#ifndef	MMB
	global	sup_rp
sup_rp:		long 	0           	# long descriptor to load into
		long	0		#	srp (supv root ptr) 
#endif MMB
#ifdef PMMU
	global	cpu_rp
cpu_rp:		long 	0           	# long descriptor to load into
		long	0		#	crp (cpu root ptr) 
#endif PMMU
	# <@(#)local.s	6.1>
