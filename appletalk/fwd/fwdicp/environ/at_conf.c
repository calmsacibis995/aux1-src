#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/at_conf.c */
#define _AC_NAME at_conf_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:06:27}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_conf.c on 87/11/11 21:06:27";
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
extern struct streamtab ddptab;
extern struct streamtab lineinfo;
extern struct streamtab loopinfo;
extern struct streamtab ploopinfo;
extern struct streamtab ddloopinfo;
extern struct streamtab nbp_info;
extern struct streamtab nbpd_info;
extern struct streamtab atp_info;
extern struct streamtab papd_info;
extern struct streamtab pap_info;

/*
 *	NOTE: that we use the fmodsw format over here, so we can put our own
 *	name on the module without changing the module itself.
 */

struct fmodsw fepdevsw[] = {
	"icplap_", &laptab,
	"icpddp_", &ddptab,
	"fwd_loop", &loopinfo,
	"fwd_ploop", &ploopinfo,
	"ddp_loop", &ddloopinfo
};

int	fepdevcnt = sizeof(fepdevsw)/sizeof(struct fmodsw);




/*
 *	Streams modules 
 */

struct fmodsw fepmodsw[] = {
	"at_nbp", &nbp_info,
	"at_nbpd", &nbpd_info,
	"at_atp", &atp_info,
	"at_papd", &papd_info,
	"at_pap", &pap_info,
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


