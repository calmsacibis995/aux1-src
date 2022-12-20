#ifndef lint	/* .../sys/PAGING/io/osm.c */
#define _AC_NAME osm_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:23:08}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of osm.c on 87/11/11 21:23:08";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)osm.c	UniPlus VVV.2.1.1	*/

/*
 *	OSM - operating system messages, allows system printf's to
 *	be read via special file.
 *	minor 0: starts from beginning of buffer and waits for more.
 *	minor 1: starts from beginning of buffer but doesn't wait.
 *	minor 2: starts at current buffer position and waits.
 */

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/page.h"
#include "sys/buf.h"
#include "sys/file.h"
#include "sys/uio.h"
#endif lint

extern	char	putbuf[];	/* system putchar circular buffer */
extern	int	putbsz;		/* size of above */
extern	int	putindx;	/* next position for system putchar */
char osmwake;			/* wake me up on a printf flag */

/*ARGSUSED*/
osmopen(dev, flag)
dev_t	dev;
{
	if ((int) dev == 2)
		u.u_ofile[u.u_rval1]->f_offset = putindx;
	else if (dev > 2)
		return(ENODEV);
	return (0);
}

osmread(dev, uio)
dev_t	dev;
struct uio	*uio;
{
	register int o, c, i;

	if ((int) dev == 1 && uio->uio_offset >= putindx)
		return(0);
	o = uio->uio_offset % putbsz;
	SPLCLK();
	while ((i = putindx) == o) {
		osmwake++;
		(void)sleep(putbuf, PWAIT);
	}
	SPL0();
	if (o < i)
		c = MIN(i - o, uio->uio_resid);
	else
		c = MIN(putbsz - o, uio->uio_resid);
	return(uiomove(&putbuf[o], c, UIO_READ, uio));
}

/*ARGSUSED*/
osmwrite(dev, uio)
dev_t	dev;
struct uio	*uio;
{
	register c;

	while ((c = uwritec(uio)) >= 0)
		putchar(c);
	wakeup(putbuf);
	return(0);
}

/* <@(#)osm.c	6.2> */
