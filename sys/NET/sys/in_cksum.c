#ifndef lint	/* .../sys/NET/sys/in_cksum.c */
#define _AC_NAME in_cksum_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1980-87 The Regents of the University of California, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:16:54}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of in_cksum.c on 87/11/11 21:16:54";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/mbuf.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"

#define M68000
#define bswap(x)	(((((int)(x))>>8)&0xff)|((((int)(x))&0xff)<<8))

/*
 * Checksum routine for Internet Protocol family headers.
 * This routine is very heavily used in the network
 * code and should be rewritten for each CPU to be as fast as possible.
 *
 * billn:  This shows the main outline for a prospective algorithm on
 * 	a prospective machine.   I suppose one could try to outline
 *	general guidelines for writing this routine, ie, if the machine
 * 	is byte-swapped, etc, etc.  In practice, what one does is find
 *	a machine which is known to conform to the "standard" and
 *	beat on the code of the new machine till it works with the
 *	known ok machine.
 */

/* int     in_ckodd;		/* number of calls on odd start add */
/* int     in_ckprint = 0;	/* print sums */
/* 68k */
union w {
	unsigned short wword;
	unsigned char wchar[2];
};

in_cksum(m, len)
register struct mbuf *m;
register short len;
{
	register unsigned short *ptr;	/* "unsigned" is important... */
	register short mlen = 0;       
	register long result = 0;
	register unsigned short r;
	register char * cptr;
	register unsigned short wasodd;
	register unsigned short thisodd;
	union w w;
	extern int tcpcksum;
	extern u_char ipcksum;

	/*
	if (in_ckprint) printf("ck m%o l%o",m,len);
	*/
	if (!tcpcksum)	/* not checksumming? */
	    if (!ipcksum)
		return 0;
	wasodd = 0;
	for (;;) {
		/*
		 * Each trip around loop adds in
		 * words from one mbuf segment.
		 */
		thisodd = 0;
		ptr = mtod(m, unsigned short *);
		mlen = m->m_len;
		if (len < mlen)
			mlen = len;
		len -= mlen;
		if (mlen > 0) {
			if (wasodd) { 	/* "last mbuf odd" code... */
				cptr = (char *)ptr;
				w.wchar[1] = *cptr++;
				result += w.wword;
				while (--mlen) {
					w.wchar[0] = *cptr++;
					mlen--;
					if (mlen) {
						w.wchar[1] = *cptr++;
						result += w.wword;
					}
					else
						/* note wasodd still set */
						goto nextbuf;	/* next mbuf */
				}
				wasodd = 0;
				goto nextbuf;	/* next mbuf */
			}

			/* main line... check odd byte count */
			if(mlen & 01) {
				thisodd++;
				mlen--;
				if (mlen == 0)
					goto lastbyte;
			}
			/* make wc a word count */
			mlen >>= 1;
			/*
			 * this is the main loop of the algorithm.
			 */
			mlen -= 1;
			do {
				result += *ptr++;
			} while(--mlen != -1);
			if (thisodd) {
lastbyte:
				wasodd++;
				cptr = (char *) ptr;
				w.wchar[0] = *cptr++;
			}
			else
				wasodd = 0;
		}
nextbuf:
		if (len <= 0)
			break;
		m = m->m_next;
		/*
		 * Locate the next block with some data.
		 */
		for (;;) {
			if (m == 0) {
				printf("cksum: out of data\n");
				goto done;
			}
			if (m->m_len)
				break;
			m = m->m_next;
		}
	}
	if (wasodd) {
	    w.wchar[1] = 0;
	    result += w.wword;
	}
done:
	if (r = (result >> 16)) {
	    result &= 0xffff;
	    result += (unsigned)r;
	    goto done;
	}

	/*
	if (in_ckprint) printf(" s%o\n",~result);
	*/
	return ((~((unsigned short)result)) & 0xffff);
}
