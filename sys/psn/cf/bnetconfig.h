#ifndef lint	/* .../sys/psn/cf/bnetconfig.h */
#define _AC_NAME bnetconfig_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.7 87/11/11 21:40:50}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.7 of bnetconfig.h on 87/11/11 21:40:50 */
/* Configuration Definition */
#define	NBUF	150
#define	NINODE	100
#define	NFILE	100
#define	NMOUNT	10
#define	NCALL	50
#define	NPROC	50
#define	NCLIST	200
#define	NTEXT	0
#define	NSVTEXT	0
#define	STACKGAP	8
#define	CMAPSIZ	0
#define	SMAPSIZ	0
#define	CXMAPSIZ	0
#define	NSABUF	0
#define	MAXMMU	1
#define	NMMU	MAXMMU*32
#define	POWER	0
#define	MAXUP	25
#define	NHBUF	128
#define	NPBUF	20
#define	NFLOCK	200
#define	CSIBNUM	20
#define	VPMBSZ	8192
#define	VPMNEXUS	0
#define	MESG	1
#define	MSGMAP	100
#define	MSGMAX	8192
#define	MSGMNB	16384
#define	MSGMNI	50
#define	MSGSSZ	8
#define	MSGTQL	200
#define	MSGSEG	8192
#define	SEMA	1
#define	SEMMAP	50
#define	SEMMNI	50
#define	SEMMNS	300
#define	SEMMNU	30
#define	SEMMSL	25
#define	SEMOPM	10
#define	SEMUME	10
#define	SEMVMX	32767
#define	SEMAEM	16384
#define	SESCONS	8
#define	SESBUFS	32
#define	SESBYTES	8192
#define	SHMEM	1
#define	SHMMAX	(256*1024)
#define	SHMMIN	1
#define	SHMMNI	100
#define	SHMSEG	6
#define	SHMBRK	16
#define	SHMALL	512
#define	STIHBUF	(ST_0*4)
#define	STOHBUF	(ST_0*4)
#define	STNPRNT	(ST_0>>2)
#define	STNEXUS	0
#define	STIBSZ	8192
#define	STOBSZ	8192
#define	FLCKFIL	50
#define	FLCKREC	200
#define	NMBUFS	500
#define	NPTY	16
#define	NSPTMAP	75
#define	NREGION	200
#define	VHNDFRAC	16
#define	MAXPMEM	0
#define	GETPGSLOW	20
#define	GETPGSHI	30
#define	VHANDR	5
#define	MAXSC	64
#define	MAXFC	100
#define	MAXUMEM	0x40000
#define	PPMEM	5
#define	EMTBSZ	8192
#define	EMRBSZ	8192
#define	EMRCVSZ	512
#define	UN56ACU	10
#define	DLO_TIME	80
#define	SZSMLOG	4096
#define	BX25LINKS	2
#define	BX25BUFS	80
#define	BX25BYTES	8192
#define	BX25HLPROT	2
#define	X25LINKS	1
#define	X25BUFS	256
#define	X25MAPS	30
#define	X25NEXUS	0
#define	X25BYTES	(16*1024)
#define MAXCORE		(192 * 1024)
#define MAXHEADER	2048
#define	DK_NDRIVE	32
#define NSTREAM		32
#define NQUEUE		256
#define NBLK4096	0
#define NBLK2048	20
#define NBLK1024	12
#define NBLK512		8
#define NBLK256		16
#define NBLK128		64
#define NBLK64		256
#define NBLK16		128
#define NBLK4		512
#define STRMSGSZ	1024
#define	MINARMEM	10
#define	MINASMEM	10
