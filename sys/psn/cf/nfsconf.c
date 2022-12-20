#ifndef lint	/* .../sys/psn/cf/nfsconf.c */
#define _AC_NAME nfsconf_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.8 87/11/11 21:40:43}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.8 of nfsconf.c on 87/11/11 21:40:43";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)nfsconf.c	UniPlus VVV.2.1.24	*/
/*
 *  Configuration information
 */


#include	"../psn/cf/bnetconfig.h"
#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/mmu.h"
#include	"sys/conf.h"
#include	"sys/cpuid.h"
#include	"sys/space.h"
#include	"sys/uconfig.h"
#include	"sys/iobuf.h"
#include	"sys/termio.h"
#include	"sys/stream.h"

extern nodev(), nulldev(), seltrue(), ttselect();
extern strinit(), strselect();
extern shlinit(), shlrinit();
extern lineinit();
extern struct streamtab lineinfo;
extern struct streamtab cloneinfo;
extern struct streamtab shlinfo;
extern struct streamtab shlrinfo;
extern ptcopen(), ptcclose();
extern ptcread(), ptcwrite(), ptcioctl(), ptcselect();
extern ptsopen(), ptsclose();
extern ptsread(), ptswrite(), ptsioctl();
extern syopen();
extern syread(), sywrite(), syioctl(), syselect();
extern mminit(), mmread(), mmwrite(), mmioctl();
extern erropen(), errclose();
extern errread();
extern osmopen();
extern osmread(), osmwrite();
extern sxtopen(), sxtclose();
extern sxtread(), sxtwrite(), sxtioctl();
extern sxtselect();
extern prfread(), prfwrite(), prfioctl();
extern cachinit();

extern nvram_init(), nvram_open(), nvram_close();
extern nvram_read(), nvram_write();
extern scinit(), scopen(), scclose();
extern scread(), scwrite(), scioctl();
extern scsiinit();
extern hdread(), hdwrite(), hdioctl(), hdioctl();
extern hdstrategy(), hdopen(), hdclose(), hdprint();
extern sninit(), snopen(), snclose(), snstrategy(), snprint();
extern snread(), snwrite(), snioctl();
extern fpioctl();
extern via1init(), rev8init();
extern struct streamtab disp_tab;
extern	mouseopen(), mouseclose(), mouseread(), mousewrite(), mouseioctl();  

#ifdef STREAMS
extern	video_init();
extern	fdb_init();
extern	key_init();
extern	mouse_init();
extern	dispinit();
#endif STREAMS
#ifndef	AUTOCONFIG
extern	nfsinit();
#endif

extern struct tty sc_tty[];
extern struct ttyptr sc_ttptr[];
int	sxt_cnt = 1;

struct bdevsw bdevsw[] = {
	nodev,    nulldev,   nulldev,      nulldev,   /*  0 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  1 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  2 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  3 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  4 */
	snopen,   snclose,   snstrategy,   snprint,   /*  5 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  6 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  7 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  8 */
	nodev,    nulldev,   nulldev,      nulldev,   /*  9 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 10 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 11 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 12 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 13 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 14 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 15 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 16 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 17 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 18 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 19 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 20 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 21 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 22 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 23 */
	hdopen,   hdclose,   hdstrategy,   hdprint,   /* 24 */
	hdopen,   hdclose,   hdstrategy,   hdprint,   /* 25 */
	hdopen,   hdclose,   hdstrategy,   hdprint,   /* 26 */
	hdopen,   hdclose,   hdstrategy,   hdprint,   /* 27 */
	hdopen,   hdclose,   hdstrategy,   hdprint,   /* 28 */
	hdopen,   hdclose,   hdstrategy,   hdprint,   /* 29 */
	hdopen,   hdclose,   hdstrategy,   hdprint,   /* 30 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 31 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 32 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 33 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 34 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 35 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 36 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 37 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 38 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 39 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 40 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 41 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 42 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 43 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 44 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 45 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 46 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 47 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 48 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 49 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 50 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 51 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 52 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 53 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 54 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 55 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 56 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 57 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 58 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 59 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 60 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 61 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 62 */
	nodev,    nulldev,   nulldev,      nulldev,   /* 63 */
};

struct cdevsw cdevsw[] = {
	scopen,   scclose,   scread,   scwrite,   scioctl,   
	sc_tty, ttselect, 0,    				/*  0 */
	syopen,   nulldev,   syread,   sywrite,   syioctl,   
	0, syselect, 0,   					/*  1 */
	nulldev,  nulldev,   mmread,   mmwrite,   mmioctl,     
	0, seltrue, 0,    					/*  2 */
	erropen,  errclose,  errread,  nulldev,   nulldev,     
	0, seltrue, 0,    					/*  3 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,   
	0, seltrue, 0,    					/*  4 */
	snopen,   snclose,   snread,   snwrite,   snioctl,   
	0, seltrue, 0,    					/*  5 */
	nulldev,  nulldev,   nulldev,  nulldev,   fpioctl,   
	0, seltrue, 0,    					/*  6 */
	nulldev,  nulldev,   nulldev,  nulldev,   nulldev,
	0, strselect, &disp_tab,				/*  7 */
	mouseopen,mouseclose,mouseread,mousewrite,mouseioctl,   
	0, seltrue, 0, 						/*  8 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/*  9 */
	sxtopen,  sxtclose,  sxtread,  sxtwrite,  sxtioctl,  
	0, sxtselect, 0,    					/* 10 */
	nulldev,  nulldev,   prfread,  prfwrite,  prfioctl,  
	0, seltrue, 0,    					/* 11 */
	nulldev,  nulldev,   nulldev,  nulldev,   nulldev,     
	0, strselect, &cloneinfo,    				/* 12 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, strselect, &shlinfo, 				/* 13 */
	nvram_open,nvram_close,nvram_read,nvram_write,nulldev,     
	0, seltrue, 0,    					/* 14 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 15 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 16 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 17 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 18 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 19 */
	ptcopen,  ptcclose,  ptcread,  ptcwrite,  ptcioctl,  
	0, ptcselect, 0,  					/* 20 */
	ptsopen,  ptsclose,  ptsread,  ptswrite,  ptsioctl,  
	0, ttselect, 0, 	  				/* 21 */
	osmopen,  nulldev,   osmread,  osmwrite,  nulldev,     
	0, seltrue, 0,    					/* 22 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 23 */
	hdopen,   hdclose,   hdread,   hdwrite,   hdioctl,     
	0, seltrue, 0,    					/* 24 */
	hdopen,   hdclose,   hdread,   hdwrite,   hdioctl,     
	0, seltrue, 0,    					/* 25 */
	hdopen,   hdclose,   hdread,   hdwrite,   hdioctl,     
	0, seltrue, 0,    					/* 26 */
	hdopen,   hdclose,   hdread,   hdwrite,   hdioctl,     
	0, seltrue, 0,    					/* 27 */
	hdopen,   hdclose,   hdread,   hdwrite,   hdioctl,     
	0, seltrue, 0,    					/* 28 */
	hdopen,   hdclose,   hdread,   hdwrite,   hdioctl,     
	0, seltrue, 0,    					/* 29 */
	hdopen,   hdclose,   hdread,   hdwrite,   hdioctl,     
	0, seltrue, 0,    					/* 30 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 31 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 32 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 33 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 34 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 35 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 36 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 37 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 38 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 39 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 40 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 41 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 42 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 43 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 44 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 45 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 46 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 47 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 48 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 49 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 50 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 51 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 52 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 53 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 54 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 55 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 56 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 57 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 58 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 59 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 60 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 61 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 62 */
	nodev,    nulldev,   nulldev,  nulldev,   nulldev,     
	0, seltrue, 0,    					/* 63 */
};

int	bdevcnt = sizeof(bdevsw)/sizeof(bdevsw[0]);
int	cdevcnt = sizeof(cdevsw)/sizeof(cdevsw[0]);


/*
 *	Streams modules 
 */

struct fmodsw fmodsw[64] = {
	"line", 	&lineinfo,
	"shlr", 	&shlrinfo,
};

int fmodcnt = sizeof(fmodsw)/sizeof(struct fmodsw);

dev_t	rootdev = makedev(0xFF, 0xFF);
dev_t	pipedev = makedev(0xFF, 0xFF);
dev_t	swapdev = makedev(0xFF, 0xFF);
daddr_t	swaplow = 0;
int	swapcnt = 28672;			/* 14 Meg of swap */

dev_t	dumpdev = makedev(0xFF, 0xFF);
extern hddump();
int	(*dump)() = hddump;
int	dump_addr = 0;

struct ttyptr *tty_stat[] = {
	sc_ttptr,
	0
};

int	(*init_first[64])() = {
	rev8init,
#ifdef STREAMS
	strinit,
#endif STREAMS
	via1init,
#ifdef STREAMS
	video_init,
	fdb_init,
	key_init,
	mouse_init,
	dispinit,
#endif STREAMS
	(int(*)())0
};	/* force it into .data */
int	init_firstl = sizeof(init_first)/sizeof(init_first[0]);

int	(*init_second[64])() = {(int(*)())0};	/* force it into .data */
int	init_secondl = sizeof(init_second)/sizeof(init_second[0]);

int	(*init_normal[64])() = {
#ifdef STREAMS
	lineinit,
	shlinit,
	shlrinit,
#endif STREAMS
	scinit,
	scsiinit,
	sninit,
	cachinit,
	mminit,
#ifndef	AUTOCONFIG
	nfsinit,
#endif
	(int (*)())0
};
int	init_normall = sizeof(init_normal)/sizeof(init_normal[0]);

int	(*init_0[64])() = {(int(*)())0};	/* force it into .data */
int	init_0l = sizeof(init_0)/sizeof(init_0[0]);

int	(*init_last[64])() = {(int(*)())0};	/* force it into .data */
int	init_lastl = sizeof(init_last)/sizeof(init_last[0]);
