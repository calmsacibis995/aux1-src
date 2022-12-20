#ifndef lint	/* .../sys/psn/os/startup.c */
#define _AC_NAME startup_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.8 87/11/11 21:33:23}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.8 of startup.c on 87/11/11 21:33:23";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)startup.c	UniPlus VVV.2.1.24	*/
/*
 *	Initial system startup code
 *
 *	Copyright 1986 Unisoft Corporation of Berkeley CA
 *
 *
 *	UniPlus Source Code. This program is proprietary
 *	with Unisoft Corporation and is not to be reproduced
 *	or used in any manner except as authorized in
 *	writing by Unisoft.
 *
 */

#ifdef	HOWFAR
#ifdef AUTOCONFIG
int	T_startup=0;
#ifdef NEW_PMMU
extern int T_meminit;
extern int T_availmem;
#endif
#else
extern int	T_startup;
#endif AUTOCONFIG
#endif HOWFAR
#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/uconfig.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/utsname.h"
#include "sys/ioctl.h"
#include "sys/tty.h"
#include "sys/var.h"

#include "sys/psl.h"
#include "sys/callout.h"

#include "sys/map.h"

#include "sys/page.h"
#include "sys/pfdat.h"
#include "sys/buf.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"

#include "sys/ivec.h"
#include "sys/tuneable.h"
#include "sys/debug.h"
#ifdef NEW_PMMU
#include "sys/pmmu.h"
#endif
#endif lint
#ifdef AUTOCONFIG
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <svfs/inode.h>
#include <svfs/mount.h>
#include <sys/mbuf.h>
#include <sys/locking.h>
#include <sys/flock.h>
#include <sys/conf.h>
#include <sys/heap_kmem.h>
extern unsigned long *Core;
extern struct freehdr *FreeHdr;
extern struct sem_undo **sem_undo;
extern struct tty *pts_tty;
extern ptsopen();
extern int cdevcnt;
extern struct pt_ioctl *pts_ioctl;
#endif AUTOCONFIG

/*
 *	Autoconfig requires that the kernel .text/.data and .bss be
 *	able to be increased in size AFTER the 'final' link of the
 *	kernel. In order to do this they are mapped into kernel virtual
 *	addresses that are non-contiguous. These addresses are chosen so
 *	they do not conflict with any existing hardware in the systems
 *	physical address space (everything else is mapped 1-1 virtual
 *	to physical initially). They are set in the makefile (see
 *	TEXTSTART, DATASTART and BSSSTART). After the kernel is linked
 *	a program 'patch_kernel' is run on it. It changes the COFF headers
 *	so that the physical address fields for each section are such that
 *	the four sections are loaded consecutively (on page boundarys)
 *	the booter reads these fields and uses them to load the code.
 *	Below is an example of how this particular kernel is loaded
 *
 *	Section		Physical address	Virtual address
 *	=======		================	===============
 *	
 *	pstart		0x4000			0x4000
 *	.text		0xe000			0x10000000
 *	.data		0x2d000			0x11000000
 *	.bss		0x45000			0x12000000
 *
 *	When patch_kernel fills in the COFF headers it also fills in
 *	sectinfo below with the appropriate information. Great care must
 *	be taken prior to the mmu being turned on to make sure that 
 *	variables and routines called must lie within the section pstart
 *	or the the appropriate routines below are used to access them.
 *	Variables that are assumed to be in pstart are marked XXXX below.
 *
 *	
 */

#ifdef AUTOCONFIG
struct sectinfo {
	long	vstart;
	long	pstart;
	long	size;
};

struct sectinfo sectinfo[3] = {0};

#define RVTL(x)		rv_long(&(x), 0)	/* read virtual .text long */
#define RVDL(x)		rv_long(&(x), 1)	/* read virtual .data long */
#define RVBL(x)		rv_long(&(x), 2)	/* read virtual .bss long */
#define RVTS(x)		rv_short(&(x), 0)	/* read virtual .text short */
#define RVDS(x)		rv_short(&(x), 1)	/* read virtual .data short */
#define RVBS(x)		rv_short(&(x), 2)	/* read virtual .bss short */

#define WVTL(x, v)	wv_long(&(x), v, 0)	/* write virtual .text long */
#define WVDL(x, v)	wv_long(&(x), v, 1)	/* write virtual .data long */
#define WVBL(x, v)	wv_long(&(x), v, 2)	/* write virtual .bss long */

static long
rv_long(x, s)
long x, s;
{
	return(*(long *)(x+sectinfo[s].pstart-sectinfo[s].vstart));
}

static short
rv_short(x, s)
long x, s;
{
	return(*(short *)(x+sectinfo[s].pstart-sectinfo[s].vstart));
}

static 
wv_long(x, v, s)
long x, v, s;
{
	*(long *)(x+sectinfo[s].pstart-sectinfo[s].vstart) = v;
}

int map_pages();
static alloc_page_tables();

#endif AUTOCONFIG

extern struct buf *sbuf;	/* start of buffer headers */
extern caddr_t 	  iobufs;	/* start of buffers */
caddr_t physiobuf=0;		/* XXXX */

#ifdef NEW_PMMU
extern ste_t cpu_rp;
extern char k1to1tbl[];
extern int maxmem;
struct pfdat_desc on_board_desc;

#else NEW_PMMU

#define ksegneed()	(ptos(physmem+btop(0x100000)))/* # of kernel virtual segs needed */
pte_t *locptbl=0;		/* XXXX gets a full pgtbl for io & physio */
pte_t *copyptbl=0;		/* XXXX */
pte_t *iduptbl=0;		/* XXXX */

pte_t *kptbl=0;			/* XXXX base of kernel page tables */
pte_t *ekptbl=0;		/* XXXX end of kernel page tables */
extern pte_t *copypte;		/* Page tables for copypage and clearpage */
extern pte_t *mmpte;		/* pointer to pte for the memory driver */
proc_t	m_idleproc = {0};	/* XXXX The idle u-blocks point to	*/
int lastaddr=0;			/* XXXX size of equipped memory (bytes) */
int kt_ppmem = 0;		/* for tight memory hang problem */
int kmemory = 0;
#endif NEW_PMMU

#ifdef	MMB
pte_t *ioptbl=0;		/* XXXX */
#endif	MMB

extern int freemem;
pte_t *uptbl=0;			/* XXXX */
int physmem=0;			/* XXXX total number of pages of memory */

extern short physiosize;	/* XXXX size of the physiobuf (in pages) */

int *nendp = 0;			/* XXXX */

/* Setup virtual address space
 * Called from start.s after memory size
 * has been calculated
 */

vadrspace(tblend, up)
caddr_t tblend;		/* where to start using memory */
struct user *up;	/* where our current stack is .... to be used as */
{			/*	the idle udot */
	register i;
	int ubase;
	extern int ivecstart, kstart;
	extern etext;
	extern int	mtimer;
	extern int (*init_first[])(), init_firstl;

#ifdef AUTOCONFIG
	if(RVDL(v.v_maxpmem)  &&  ptob(physmem) > ptob(RVDL(v.v_maxpmem)))
		physmem = RVDL(v.v_maxpmem);
#else
	if(v.v_maxpmem  &&  ptob(physmem) > ptob(v.v_maxpmem))
		physmem = v.v_maxpmem;
#endif AUTOCONFIG

#ifndef NEW_PMMU
	lastaddr = ptob(physmem);
#endif

	/* Just round up to next page.
	 *              Sbrpte points to the start of a full set of
	 * PTEs for the kernel. The last few segments are allocated
	 * for special windows, e.g. the u_area. The user page tables
	 * follow the kernel's and are allocated by malloc on ptmap.
	 */
	nendp = ptround(tblend);
	/* allocate page tables */
#ifdef NEW_PMMU
	uptbl = (pte_t *) nendp; 
	nendp = (int *)(uptbl + USIZE);
#else NEW_PMMU

#ifdef	MMB
	ioptbl = (pte_t *) nendp; 
	nendp = (int *)(ioptbl + (NBRIOSEG * NPGPT));
#endif	MMB

	locptbl = (pte_t *) nendp; 
	nendp = (int *)(locptbl + NPGPT);

	copyptbl = (pte_t *) nendp; 
	nendp = (int *)(copyptbl + (ptos(COPYSEGSZ) * NPTBL));

	iduptbl = (pte_t *) nendp; 
	nendp = (int *)(iduptbl + USIZE);
	nendp = (int *) ptround((int) nendp);
	uptbl = (pte_t *) nendp; 
	nendp = (int *)(uptbl + USIZE);
	nendp = (int *)(iduptbl + (ptos(2 * USIZE) * NPTBL));
#endif ! NEW_PMMU

	if (physiosize) {
		/* allocate physio pages */
		nendp = (int *) ptob(btop((int) nendp));
		physiobuf = (caddr_t)nendp;
		nendp +=  physiosize * (NBPP >> BPWSHFT);
	} else
		physiobuf = (caddr_t)NULL;

#ifndef NEW_PMMU
	kptbl = (pte_t *) nendp; 
	nendp = (int *)((pte_t *)nendp + (ksegneed() * NPGPT));  
	nendp = (int *) ptob(btop((int) nendp));
	ekptbl = (union pte *)nendp;  
AUTO_TRACE(T_startup, ("vadrspace: kptbl=0x%x ekptbl=0x%x\n", kptbl, ekptbl));

	/* Initialize the kernel segment table
	 * The segment table is never changed except for
	 * the last entry which points to the u_area
	 * page tables of the running process
	 */
AUTO_TRACE(T_startup, ("vadrspace: initializing kernel segment table\n"));
	for (i = 0; i < ksegneed(); i++)
	{
		wtste(kstbl[i], SEG_RW, NPGPT, &kptbl[i << NPGPTSHFT]);
	}

	/*    
	 * Initialize the kernel text and data page tables
	 */
AUTO_TRACE(T_startup, ("vadrspace: initializing kernel text & data tables\n"));
	for (i = 0; i < physmem; i++)
	{
		pg_zero(&kptbl[i]);
		wtpte(kptbl[i].pgi, PG_V|PG_RW, i);
	}

	nendp = (int *) ptob(btop((int) nendp));

	/* set up local data area - it is seg LOCSEG  */
	 
AUTO_TRACE(T_startup, ("vadrspace: setting up LOCSEG\n"));
	for (i=0; i < LOCSIZE; i++) {
		pg_zero(&locptbl[i]);
		wtpte(locptbl[i].pgi, PG_V|PG_RW, btop((int) nendp));
		nendp +=  NBPP >> BPWSHFT;
	}
	wtste(kstbl[LOCSEG], SEG_RW, LOCSIZE+physiosize+MEMIOSIZE, locptbl); 

	/* Set up the COPYSEG segment.
	 */
	
AUTO_TRACE(T_startup, ("vadrspace: setting up COPYSEG\n"));
	wtste(kstbl[COPYSEG], SEG_RW, COPYSEGSZ, copyptbl);

	nendp = (int *) ptob(btop((int) nendp));

	/* setup the idle ptbls and the idle u-block. */
AUTO_TRACE(T_startup, ("vadrspace: setting up the idle ptbls & the idle u-block\n"));

	for (i=0; i < USIZE; i++) {
		pg_zero(&iduptbl[i]);
		wtpte(iduptbl[i].pgi, PG_V|PG_RW, (btop((int)up)+i));
	}
/**********************************IO MAPPING**********************************/
#ifdef	MMB
	{ register unsigned ioaddr; register pte_t *pgtbl;

	/* page tables for io addresses */
	ioaddr = (unsigned)DEVICE_MAP>>PAGESHIFT;
	/* zero ptes */
	for (i=0; i < NPGPT*NBRIOSEG; i++) {
		pg_zero(&ioptbl[i]);
		wtpte(ioptbl[i].pgi, PG_V|PG_RW|PG_CI, ioaddr++);
		}
	/* map page tables to IOSEG */
	wtste(kstbl[IOSEG], S_KRW, NPGPT, ioptbl);
	wtste(kstbl[IOSEG+1], S_KRW, NPGPT, &ioptbl[NPGPT]);
	wtste(kstbl[NETSEG], S_KRW, NPGPT, (long)&ioptbl[2*NPGPT]);
	/* page table 2 for net addresses */
	ioaddr = (unsigned)(NET_MAP)>>PAGESHIFT;
	for (pgtbl= &ioptbl[2*NPGPT]; pgtbl<&ioptbl[NBRIOSEG*NPGPT]; pgtbl++)
		wtpte(pgtbl->pgi, PG_V|PG_CI, ioaddr++);
	}
#else	MMB
	/*                   
	 * map in all memory 1-1 for I/O starting at TOPSEG
	 * PMMU can use segment level pte's (long pte's)
	 */                  
	for (i = TOPSEG; i < MAXSEG; i++)
		wtlpte(kstbl[i], S_KRW, NPGPT, stob(i));
#endif	MMB
/**********************************IO MAPPING**********************************/
	up->u_procp = &m_idleproc;
	up->u_stack[0] = STKMAGIC;
	m_idleproc.p_pid = -1;
	/* USEG is used for UDOT, PHYSIO, and IO pages */
AUTO_TRACE(T_startup, ("vadrspace: setting up USEG 0x%x 0x%x\n",&kstbl[USEG],
				iduptbl));
	wtste(kstbl[USEG], SEG_RW, USIZE, iduptbl);
#else ! NEW_PMMU
	kstbl = (ste_t *) up;
	AUTO_TRACE(T_startup, ("vadrspace: kstbl = 0x%x\n", kstbl));
	for (i = 0; i < TAKVNUM; i++) {
		wtrte(ktbla[i + TAKVINDEX], RT_KRW, NSTBL,
					(long) &kstbl[i * NSTBL]);
		k1to1tbl[i + TAKVINDEX] = 0;
	}
	/*
	 *	For now we put the kstbl in the udot for sched - this is
	 *	okay since sched never runs as a user process.  If the
	 *	kernel grows bigger than the u.u_stbl array, the memory
	 *	can be grabbed some other way.  We can map up to 512 Meg
	 *	of Virtual address space and save up to 4k of memory with
	 *	the current method.
	 */
	for (i = 0; i < TAKVNUM * NSTBL; i++) {
		kstbl[i].segm.ld_dt = DTLINV;
		AUTO_TRACE((i * sizeof(ste_t) > sizeof(u.u_stbl)),
			("vadrspace: sizeof(kstbl) > sizeof(u.u_stbl)\n"));
	}
	for (i=0; i < USIZE; i++) {
		pg_zero(&uptbl[i]);
		wtpte(uptbl[i].pgi, PG_V|PG_RW, btop((int)up) + i);
	}
	wtste(kstbl[USEG], SEG_RW, NPGPT, uptbl);
AUTO_TRACE(T_startup, ("vadrspace: &kstbl[USEG] = 0x%x\n", &kstbl[USEG]));

#endif NEW_PMMU


	/* Copy 680xx interrupt vectors. */
	/* BOBJ: for multiprocessors (68010 or 68020) need to use vector
	 * offset (see SIII dsv) */

	{
	register int q;
	register int *ivecp;
	extern catchintr();
	extern runtime;
AUTO_TRACE(T_startup, ("vadrspace: setting up vectors\n"));
	for (ivecp = &((int *) kstart)[0], q=0; ivecp < &((int *) kstart)[N_IVECS]; ivecp++, q++)
		if (runtime & RT_20CCHEBUG)
			/* vector all interrupts through cache clear code */
			*ivecp = (int)catchintr + ivecstart;
		else
			*ivecp = (int)ivect + ((int)ivecp - kstart);
	}
#ifdef AUTOCONFIG

	/*
	 *	Now we create the pagetables etc to map in .text/.data/.bss
	 */

	map_pages(btop(sectinfo[0].vstart),	/* .text */
		  btop(sectinfo[0].pstart),
		  btop(sectinfo[0].size),
		  alloc_page_tables);
	map_pages(btop(sectinfo[1].vstart),	/* .data */
		  btop(sectinfo[1].pstart),
		  btop(sectinfo[1].size),
		  alloc_page_tables);
	map_pages(btop(sectinfo[2].vstart),	/* .bss */
		  btop(sectinfo[2].pstart),
		  btop(sectinfo[2].size),
		  alloc_page_tables);

#endif AUTOCONFIG

#ifdef NEW_PMMU
	{       /* mmb_on loads the cpu_rp - feed it a dummy value */
		int addr;

		addr = (long) uptbl[1].pgi[0].pg_pte & ~0xFF;
		wtrte(cpu_rp, RT_VALID, UNTBLA, addr);
	}
#endif

AUTO_TRACE(T_startup, ("vadrspace: turning on MMU\n"));
	mmb_on();

#ifdef KDB
	kdb_call(0);
#else KDB
#ifdef AUTOCONFIG
	for (i = 0; i < btos(sectinfo[0].size); i++) 
		kstbl[i].segf.ld_msadr |= STWPROT;
#endif
#endif KDB

	/*	No user page tables available yet.
	 */
	
	ptfree.pf_next = &ptfree;
	ptfree.pf_prev = &ptfree;

	/*
	 * Allocate memory for paging tables
	 */

	ubase = btop((int)nendp);
TRACE(T_startup, ("vadrspace: mktables(0x%x)\n", ubase));
	ubase=mktables(ubase);

	/* initialize the region table */

TRACE(T_startup, ("vadrspace: reginit()\n"));
	reginit();

	/* Initialize the map of free kernel virtual address space 
	 */

TRACE(T_startup, ("vadrspace: mapinit(0x%x, 0x%x)\n", sptmap, v.v_sptmap));
	mapinit(sptmap, v.v_sptmap);
#ifndef NEW_PMMU
TRACE(T_startup, ("vadrspace: mfree(0x%x, 0x%x, 0x%x)\n", sptmap,
		 (ksegneed()*NPGPT)-physmem, physmem));
	mfree(sptmap, (ksegneed() * NPGPT) - physmem, physmem);
#endif
	maxmem = physmem;

	/*
 	 *	Call init_first routines ....
	 */

	for (i = 0; i < init_firstl && init_first[i]; i++) {
		TRACE(T_startup, ("vadrspace calling 0x%x ubase = 0x%x\n", 
				init_first[i], ubase));
		(*init_first[i])(&ubase);
	}

	/* initialize queue of free pages */

#ifdef NEW_PMMU
	on_board_desc.pfd_dmaokay = 1;
	on_board_desc.pfd_dmaspeed = PFD_SLOW;
	on_board_desc.pfd_memokay = 1;
	on_board_desc.pfd_memspeed = PFD_FAST;

	memregadd(ptob(ubase), ptob(physmem - ubase), &on_board_desc);

	memalloctables(ubase);

	initmemfree();
	v.v_maxpmem = maxmem;
#else
TRACE(T_startup, ("vadrspace: meminit(0x%x, 0x%x)\n", ubase, physmem));
	meminit(ubase, physmem);

	/* Setup special windows for copypage/clearpage 
	 * and the memory driver */

	mmpte = &locptbl[MEMIOPTE];
	copypte = copyptbl;
#endif

	/*	Initialize process 0.
	 */
	
TRACE(T_startup, ("vadrspace: p0init()\n"));
	p0init();


	/*	Indicate that we are the master cpu for timing.
	 *	This is checked in clock.
	 */
	
	mtimer = 1;

	/*	Off to main.  We never return from this call.
	 */
	
TRACE(T_startup, ("vadrspace: returning\n"));
}

/* Setup proc[0] to look like a user process that has 
 * done a system call.
 */

p0init()
{
	register struct proc *p;
	register struct user *up;
	register pte_t *pt;
	register	t, addr;

	p = proc;
	up = &u;
	pt = (pte_t *)p->p_uptbl;
	p->p_size = USIZE;
#ifdef NEW_PMMU
        for (t = 0; t < USIZE; t++)
		pt[t] = uptbl[t];
	pt = uptbl;
	wtste(kstbl[USEG], SEG_RW, NPGPT, pt);
	wtste(p->p_addr, SEG_RW, NPGPT, pt);
	clratb(SYSATB);				/* flush the mmu */
	addr = (long) pt->pgi[0].pg_pte & ~0xFF;
	for (t = 0; t < UNTBLA; t++, addr += NSTBL) {
		wtrte(u.u_tbla[t], RT_VALID, NSTBL, addr);
		wtrte(ktbla[TAUINDEX + t], RT_VALID, NSTBL, addr);
		k1to1tbl[TAUINDEX + t] = 0;
	}
	addr = (long) ++pt->pgi[0].pg_pte & ~0xFF;
	wtrte(cpu_rp, RT_VALID, UNTBLA, addr);
#else NEW_PMMU

	reglock(&sysreg);
	memlock();
	if(ptmemall(&sysreg, pt, USIZE, 1))
		panic("p0init - ptmemall failed");
	memunlock();
	regrele(&sysreg);
TRACE(T_startup,("setting up USEG\tbcopy(0x%x, 0x%x, 0x%x)\n",
p->p_uptbl, uptbl, USIZE*sizeof(pte_t)));
	bcopy((caddr_t)p->p_uptbl, (caddr_t)uptbl, USIZE*sizeof(pte_t));
	pt = (pte_t *)uptbl;
TRACE(T_startup, ("p0init: pt=0x%x\n", pt));
	wtste(kstbl[USEG], SEG_RW, USIZE, pt);
	wtste(p->p_addr, SEG_RW, USIZE, pt);
	clratb(SYSATB);				/* flush the mmu */
	for (t = 0; t < NSEGP; t++)
		wtste(up->u_stbl[t], 0, 0, 0);
#endif NEW_PMMU

	up->u_stack[0] = STKMAGIC;
	up->u_procp = p;
TRACE(T_startup, ("p0init() returning\n"));
}

/*
 * Create system space to hold page allocation and
 * buffer mapping structures and hash tables
 */

mktables(physpage)
register int physpage;
{
	register int	m;
	register int	i;
	register preg_t	*prp;
	extern int	pregpp;
	register caddr_t memp;


	memp = (caddr_t)ptob(physpage);

#ifndef NEW_PMMU
	uptbase = (caddr_t)((int)ekptbl & ~POFFMASK);

	/*	Compute the smallest power of two larger than
	 *	the size of physical memory.
	 */

	m = physmem;
	while (m & (m - 1))
		 m = (m | (m - 1)) + 1;
	phashmask = (m>>1) - 1;

	/*
	 *	System buffers - these are first to put them on a page boundary
	 */

	iobufs = memp;
	memp += v.v_buf * v.v_sbufsz;
#endif !NEW_PMMU

	/*
	 *	Next do the mbufs .... put them o/n a 'nice' boundary also
	 */
	
	mbufbufs = (struct mbuf *)memp;
	memp += sizeof(struct mbuf) * (v.v_nmbufs+1);
	mbutl = (struct mbuf *)memp;
	memp += (NMBCLUSTERS * MCLBYTES) + MCLBYTES;

#ifndef NEW_PMMU
	/*
	 *	Allocate space for the page hash bucket
	 *	headers.
	 */

	phash = (struct pfdat **)memp;
	memp += (m >> 1) * sizeof(*phash);

	/*	Allocate space for the pfdat table.  It has one
	 *	entry per page of physical memory.
	 */

	pfdat = (struct pfdat *)memp;
	memp += physmem * sizeof(*pfdat);

	/*
	 *	System buffer headers
	 */

	sbuf = (struct buf *)memp;
	memp += sizeof(struct buf) * v.v_buf;

#endif !NEW_PMMU

	/*
	 *	Allocate the process table
	 */

	proc = (struct proc *)memp;
	memp += sizeof(struct proc) * v.v_proc;
	v.ve_proc = (char *)&proc[1];
	v.ve_proctab = (char *)&proc[0];

	/*
	 *	Allocate the per-process semaphore undo table
	 */

	sem_undo = (struct sem_undo **)memp;
	memp += sizeof(struct sem_undo *) * v.v_proc;

	/*
	 *	Allocate the spt map
	 */

	sptmap = (struct map *)memp;
	memp += sizeof(struct map) * v.v_sptmap;

	/*
	 *	Allocate the region table
	 */

	region = (reg_t *)memp;
	memp += sizeof(reg_t) * v.v_region;

	/*
	 *	Allocate the inode table
	 */

	inode = (struct inode *)memp;
	memp += sizeof(struct inode) * v.v_inode;
	v.ve_inode = (char *)&inode[v.v_inode];

	/*
	 *	Allocate the file table
	 */

	file = (struct file *)memp;
	memp += sizeof(struct file) * v.v_file;
	v.ve_file = (char *)&file[v.v_file];

	/*
	 *	Allocate the mount table
	 */

	mounttab = (struct mount *)memp;
	memp += sizeof(struct mount) * v.v_mount;
	v.ve_mount = (char *)&mounttab[v.v_mount];

	/*
	 *	Allocate the physio buffers
	 */

	pbuf = (struct buf *)memp;
	memp += sizeof(struct buf) * v.v_pbuf;

	/*
	 *	Allocate the file lock table
	 */

	locklist = (struct locklist *)memp;
	memp += sizeof(struct locklist) * v.v_flock;

	/*
	 *	Allocate the callout table
	 */

	callout = (struct callout *)memp;
	memp += sizeof(struct callout) * v.v_call;
	v.ve_call = (char *)&callout[v.v_call];

	/*
	 *	Allocate pty data structures
	 */

	pts_tty = (struct tty *)memp;
	memp += sizeof(struct tty) * v.v_npty;
	pts_ioctl = (struct pt_ioctl *)memp;
	memp += sizeof(struct pt_ioctl) * v.v_npty;
	for (i = 0; i < cdevcnt; i++)
	if (cdevsw[i].d_open == ptsopen) {
		cdevsw[i].d_ttys = pts_tty;
		break;
	}

	/*
	 *	Allocate heap kmem data structures
 	 *	N.B.:	the definitions for mbufbufs[], mbutl[], and Core[]
	 *		must occur in exactly that order.  Several macros depend
	 *		on it.
	 */

	Core = (unsigned long *)memp;
	memp += sizeof(unsigned long) + v.v_maxcore;
	FreeHdr = (struct freehdr *)memp;
	memp += sizeof(struct freehdr) * v.v_maxheader;

	/*
	 *	Allocate flock data structures
	 */

	flox = (struct filock *)memp;
	memp += sizeof(struct filock) * flckinfo.recs;
	flinotab = (struct flino *)memp;
	memp += sizeof(struct flino) * flckinfo.fils;

	/*	Allocate space for the pregion tables for each process
	 *	and link them to the process table entries.
	 *	The maximum number of regions allowed for is process is
	 *	3 for text, data, and stack plus the maximum number
	 *	of shared memory regions allowed.
	 */

	prp = (preg_t *)memp;
	memp += pregpp * sizeof(preg_t) * v.v_proc;
	for(i = 0  ;  i < v.v_proc  ;  i++, prp += pregpp)
		proc[i].p_region = prp;

	/*	Allocate the table which will be used to translate
	 *	from physical page table entry address to kernel
	 *	virtual address.  This table is used by pfault and
	 *	vfault.  The table must map all of physical
	 *	memory which can contain user page tables.  It
	 *	must have one table entry for each page of this
	 *	space and each table entry is one word long.
	 */

#ifndef NEW_PMMU
	uptvaddrs = (caddr_t *)memp;
	memp += btop(memp - uptbase) * NBPW;
#endif

	return(btop((long)memp));
}

/*
 * Machine dependant startup
 * Called from main
 */
startup()
{
	register int i;

	/*
	 * Initialize callouts
	 */
	callfree = &callout[0];
	for (i = 1; i < v.v_call; i++)
		callout[i-1].c_next = &callout[i];
	oem7init();
	SPL0();
	/**********
	printf("\nUNIX/%s: %s%s\n",
		utsname.release, utsname.sysname, utsname.version);
	**********/

#ifdef NEW_PMMU
	printf("total memory size: %d bytes\n", ptob(maxmem));
#else
	printf("total memory size: %d bytes\n", ptob(physmem));

	kmemory = 0;		/* keep track of physmem used by kernel */
	tune.t_ppmem = freemem - (tune.t_ppmem * freemem) / 100; 
				/* from percentage to actual number */
				/* superuser can use more than user */
	kt_ppmem = (freemem + tune.t_ppmem) / 2;
#endif
	printf("available  memory: %d bytes\n", ptob(freemem-USIZE));
}


/*
 * Initialize clists
 */
struct	chead	cfreelist;
struct	cblock	*cfree;

cinit()
{
	register int n;
	register struct cblock *bp;
	extern putindx;

	/* allocate memory */
	n = btop(v.v_clist*sizeof(struct cblock));

#ifdef NEW_PMMU
	availrmem -= n;
	availsmem -= n;

	if (availrmem < tune.t_minarmem  || availsmem  < tune.t_minasmem) {
		printf("cinit - can't get %d pages\n", n);
		panic("cannot allocate character buffers");
	}
	TRACE(T_availmem,("cinit: taking %d avail[RS]mem pages\n", n));
	if((cfree=(struct cblock *)sptalloc(n, PG_V|PG_RW, -1)) == NULL)
#else
	if((cfree=(struct cblock *)sptalloc(n, PG_V|PG_RW, 0)) == NULL)
#endif
		panic("cannot allocate character buffers");

	/* free all cblocks */
	bp = cfree;
	for(n = 0; n < v.v_clist; n++, bp++)
		putcf(bp);
	cfreelist.c_size = CLSIZE;

	/* print out messages so far */
}

#ifdef AUTOCONFIG

/*
 *	map_pages maps size pages from virtual address vstart (in pages)
 *		to physical address pstart (in pages)
 *		it is also passed a routine to aalocate space for more
 *		page tables (different once the system is running)
 *		alloc_page_tables is the simple one used prior to the
 *		mmu being turned on
 */

#ifdef	MMB
extern ste_t *rootbl;		/* function code table */
extern ste_t *mmbtbla;		/* table A for MMB implementation only */
#else	MMB
extern ste_t sup_rp;    	/* long descriptor to load into srp */
				/* 	(supv root ptr) */
#endif MMB
#ifdef PMMU
extern ste_t cpu_rp;           	/* long descriptor to load into crp */
				/*	(cpu root ptr) */
#endif PMMU

static
alloc_page_tables(s, size)
ste_t *s;
{
#ifdef MMB
	register pte_t *p;
	register ste_t *s2;
	register long i;

	nendp = (int *) ptround((int) nendp);
	if (size == 0) {			/* page table */
		wtste(*s, SEG_RW, NPTBL, nendp);
		nendp += NPTBL*sizeof(*p);
		p = (pte_t *)segpptbl(*s);
		for (i = 0; i < NPGPT; i++)  {
			pg_zero(&p[i]);
		}
	} else {				/* segment table */
		wtrte(*s, RT_KRW, size, (unsigned long)nendp);
		nendp += size*sizeof(*s2);
		s2 = (ste_t *)segpptbl(*s);
		for (i = 0; i < size; i++)  {
			wtste(s2[i], 0, 0, 0);
		}
	}
#else
	register pte_t *p;
	register long sp, i;

	nendp = (int *) ptround((int) nendp);
	if (s->segm.ld_dt == DTPD)
		sp = (long) btop((u_int) segpptbl(*s));
	else
#ifdef  NEW_PMMU
		sp = btop(kvsegntova(s-kstbl));
#else
		sp = stopg(s-kstbl);
#endif
	wtste(*s, SEG_RW, NPGPT, nendp);
	nendp += NPGPT*sizeof(*p);
	p = (pte_t *)segpptbl(*s);
	for (i = 0; i < NPGPT; i++)  {
		pg_zero(&p[i]);
		wtpte(p[i].pgi, PG_V|PG_RW, sp++);
	}
#endif MMB
}

map_pages(vstart, pstart, size, alloc)
long vstart, pstart, size;
int (*alloc)();
{
#ifdef MMB
	register ste_t *s1, *s2;
	register pte_t *p;
	register int ssize, si, i, lim, inc, slim;

	s1 = &mmbtbla[((unsigned long)vstart)>>(PSTBLINDEX+PTBLINDEX)];
	while (size) {
		si = ptots(vstart)&(NSTBL-1);
		ssize = ((si + ptos(size)) >  NSTBL ? NSTBL : (si + ptos(size)));
		if (s1->segm.ld_dt == DTLINV)
			(*alloc)(s1, NSTBL);
		s2 = &((ste_t *)segpptbl(*s1))[si];
		slim = ssize - si;
		while (slim--) {
			if (s2->segm.ld_dt == DTLINV)
				(*alloc)(s2, 0);
			p = (pte_t *)segpptbl(*s2);
			i = inc = vstart&PNUMMASK;
			lim = (i + size > NPGPT ? NPGPT : i + size);
			while (i < lim) {
				pg_zero(&p[i]);
				wtpte(p[i].pgi, PG_V|PG_RW, pstart++);
				i++;
			}
			size -= lim - inc;
			vstart += lim - inc;
			s2++;
			si++;
		}
		s1++;
	}
#else
	register ste_t *s;
	register pte_t *p;
	register int i, lim, inc;

#ifdef NEW_PMMU
	s = &kstbl[vtokstblindex(ptob(vstart))];
#else
	s = &kstbl[(vstart>>PTBLINDEX)&SNUMMASK];
#endif
	while (size) {
		if (s->segm.ld_dt == DTLINV || s->segm.ld_dt == DTPD)
			(*alloc)(s);
		p = (pte_t *)segpptbl(*s);

		i = inc = vstart&PNUMMASK;
		lim = (i + size > NPGPT ? NPGPT : i + size);
		while (i < lim) {
			pg_zero(&p[i]);
			wtpte(p[i].pgi, PG_V|PG_RW, pstart++);
			i++;
		}
		size -= lim - inc;
		vstart += lim - inc;
		s++;
	}
#endif MMB
}

#ifdef HOWFAR
/*
 *	The following routines are a 'mini' printf, only included when
 *		debugging, they are used via AUTO_TRACE, prior to
 *		the mmu being turned on
 */

auto_trace(s, a)
register char *s;
int a;
{
	int *ap = &a;

	while (*s) {
		if (*s != '%') {
			auto_putchar(*s++);
			continue;
		}
		s++;
		switch(*s++) {
		case '%':
			auto_putchar('%');
			break;
		case 'x':
			auto_num(*ap++, 16);
			break;
		}
	}
}

auto_num(x, b)
int x;
{
	if (x == 0) {
		auto_putchar('0');
		return;
	}
	if (b == 16) {
		auto_x(x, 4);
	}
}

auto_x(n, s)
{
	int i;

	if (n) {
		auto_x(n>>s, s);
		i=n&((1<<s)-1);
		if (i >= 10) {
			auto_putchar('a'+i-10);
		} else {
			auto_putchar('0'+i);
		}
	}
}

/*
 * 	NOTE: this routine (to output a character with the MMU turned
 *		off and using ONLY local variables) is system specific
 *		and is only used to get the system debugged enough to
 *		turn the mmu on
 */

#include <sys/scc.h>

#define	W5ON	(W5TXENABLE | W5RTS | W5DTR)	/* turn on to talk */

char sc_d5 = 0;

auto_putchar(c)
char c;
{
	register struct device *addr ;
	register int s;
	register int i;

	addr = (struct device *)0x50F04002;
	if((sc_d5 & W5ON) != W5ON) {
		sc_d5 |= W58BIT | W5ON;
		addr->csr = 5;
		addr->csr = sc_d5;
	}
	if (c == '\n') {
		auto_putchar('\r');
		for (i = 100000; i; i--);		/* DELAY */
	}
	i = 100000;
	while ((addr->csr & R0TXRDY) == 0 && --i)
		;
	addr->data = c;
}
#endif HOWFAR
#endif AUTOCONFIG

#ifdef NEW_PMMU
int s_buf_fact = 10;

memalloctables(ubase)
{
	register struct pfdd_type flags;
	register u_int m;
	int availmem = maxmem - ubase;

	if ( ! v.v_buf)
		v.v_buf = ptob(availmem) / (s_buf_fact * v.v_sbufsz);

	if (v.v_buf < v.v_mount + 10)
		v.v_buf = v.v_mount + 10;

	if (v.v_buf > (9 * ptob(availmem)) / (v.v_sbufsz * 10)) {
		printf("System Buffers are more than 90%% of remaining memory\n");
		printf("UNIX kernel may be unstable - may need to adjust NBUFS with kconfig\n");
	}

	flags.pfdt_dmaokay = 1;
	flags.pfdt_dmaspeed = PFD_FAST;
	flags.pfdt_memokay = 1;
	flags.pfdt_memspeed = PFD_ANY;

	iobufs = (caddr_t) memreg_alloc(v.v_buf * v.v_sbufsz, flags, v.v_sbufsz - 1);
	if (iobufs == (caddr_t) -1) {
		printf("memalloctable: can't get 0x%x bytes\n", v.v_buf * v.v_sbufsz);
		panic("cannot allocate buffer cache");
	}
	TRACE(T_meminit, ("memalloctables got 0x%x bytes at 0x%x for iobufs\n", v.v_buf * v.v_sbufsz, iobufs));
	
	flags.pfdt_memokay = 1;
	flags.pfdt_memspeed = PFD_FAST;
	flags.pfdt_dmaokay = 0;
	flags.pfdt_dmaspeed = PFD_ANY;

	sbuf = (struct buf *) memreg_alloc(sizeof(*sbuf) * v.v_buf, flags, 1);

	if (sbuf == (struct buf *) -1) {
		printf("memalloctable: can't get 0x%x bytes\n", sizeof(*sbuf) * v.v_buf);
		panic("cannot allocate buffer headers");
	}

	/*	Compute the smallest power of two larger than
	 *	the size of available memory.
	 */

	m = availmem; /* availmem is in pages, want one bucket per page maximum */
	while (m & (m - 1))
		 m = (m | (m - 1)) + 1;

	phashmask = m - 1;

	/*
	 *	Allocate space for the page hash bucket
	 *	headers.
	 */

	flags.pfdt_memokay = 1;
	flags.pfdt_memspeed = PFD_FAST;
	flags.pfdt_dmaokay = 0;
	flags.pfdt_dmaspeed = PFD_ANY;

	phash = (struct pfdat **) memreg_alloc(m * sizeof(*phash), flags, 1);
	if (phash == (struct pfdat **) -1) {
		printf("memalloctables - can't get %d pages\n", m * sizeof(*phash));
		panic("cannot allocate page hash table");
	}
TRACE(T_startup, ("memalloctables: phash (0x%x) array has 0x%x buckets\n", phash, m));

}
#endif NEW_PMMU
