#ifndef lint	/* .../appleprint/iw/daiw/stuffc.c */
#define _AC_NAME stuffc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:45:05}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _stuffc_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of stuffc.c on 87/11/11 21:45:05";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *
 * Stuffc takes the character and buys a CC linked list member structure.
 * It then inserts the CC member into the CC linked list sorted by the
 * current vertical and horizontal positions.
 *
 */

#include "local.h"

void stuffc(c0,c1,c2,c3)
	int	c0;
	int	c1;
	int	c2;
	int	c3;
	{
	register CC    *cc;
	register CC    *ll;
	register CC    *pp;

	cc = (CC *)malloc(sizeof(CC));
	cc->cc_vert = pos_vert;
	cc->cc_horz = pos_horz;
	cc->cc_font = cur_font;
	cc->cc_ptsz = pointsz;
	cc->cc_char[0] = c0;
	cc->cc_char[1] = c1;
	cc->cc_char[2] = c2;
	cc->cc_char[3] = c3;
	cc->cc_guard   = '\0';
	ll = cchead;
	if (ll)	/* Dumb, linear search for now ... */
	    {
	    while (ll)
		{
                if (cc->cc_vert > ll->cc_vert)
		    {
		    ll = ll->cc_next;
		    continue;
		    }
                else if (cc->cc_vert == ll->cc_vert)
		    {
		    if (cc->cc_horz >= ll->cc_horz)
			{
		        ll = ll->cc_next;
			continue;
			}
		    else
			{
			break;
		        }
		    }
		else
		    {
		    break;
		    }
		}
	    if (ll)	/* cc goes on before ll */
		{
		pp = ll->cc_prev;
		if (pp)		/* cc will NOT become first on linked list */
		    {
		    pp->cc_next = cc;
		    ll->cc_prev = cc;
		    cc->cc_prev = pp;
		    cc->cc_next = ll;
		    }
		else		/* cc becomes first on linked list */
		    {
		    cchead = cc;
		    ll->cc_prev = cc;
		    cc->cc_prev = 0;
		    cc->cc_next = ll;
		    }
		}
	    else	/* tail end add on: cc goes on after ll */
		{
		ll = cctail;
		cctail = cc;
		ll->cc_next = cc;
		cc->cc_prev = ll;
		cc->cc_next = 0;
		}
	    }
	else
	    {
	    cc->cc_prev = 0;
	    cc->cc_next = 0;
	    cchead = cc;
            cctail = cc;
	    }
	if (sw_debug[3])
	    {
	    dbg_cc1(cc);
	    }
	if (sw_debug[2])
	    {
	    dbg_cc(cchead);
	    }
	}
