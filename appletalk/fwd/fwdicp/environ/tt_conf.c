#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/tt_conf.c */
#define _AC_NAME tt_conf_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:37}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of tt_conf.c on 87/11/11 21:07:37";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*
 *  Configuration information
 */


#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/conf.h"
#include	"sys/termio.h"
#include	"sys/ttychars.h"

extern nodev(), nulldev();
extern struct streamtab laptab;
extern struct streamtab lineinfo;
extern struct streamtab transinfo;
extern struct streamtab loopinfo;
extern struct streamtab ploopinfo;
extern struct streamtab scc_tab;

/*
 *	NOTE: that we use the fmodsw format over here, so we can put our own
 *	name on the module without changing the module itself.
 */

struct fmodsw fepdevsw[] = {
	"fwd_loop", &loopinfo,
	"fwd_ploop", &ploopinfo,
	"fwd_asyn", &scc_tab,
};

int	fepdevcnt = sizeof(fepdevsw)/sizeof(struct fmodsw);




/*
 *	Streams modules 
 */

struct fmodsw fepmodsw[] = {
	"line", 	&lineinfo,
	0,
};

int fepmodcnt = sizeof(fepmodsw)/sizeof(struct fmodsw);

/*
 * Default terminal characteristics
 */
char	ttcchar[NCC] = {
	0x03,
	CQUIT,
	0x7f,
	CKILL,
	CEOF,
	0,
	0,
	0
};
struct ttychars ttycdef = {
	0x7f,
	CKILL,
	0x03,
	CQUIT,
	CSTART,
	CSTOP,
	CEOF,
	CBRK,
	0xFF,	/* suspc */
	0xFF,	/* dsuspc */
	0xFF,	/* rprntc */
	0xFF,	/* flushc */
	0xFF,	/* werasc */
	0xFF,	/* lnextc */
};


