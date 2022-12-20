#ifndef lint	/* .../appletalk/fwd/streams/strdebug.c */
#define _AC_NAME strdebug_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:05:15}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of strdebug.c on 87/11/11 21:05:15";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)strdebug.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>

#ifndef NULL
#define NULL		0
#endif NULL



/*
 *	Print message block descriptor
 */

static
print_mblk(mp)
struct msgb		*mp;

{

    printf("mp = %x\n", mp);
    printf("     *b_next  = %x\t\t*b_rptr  = %x\n", mp->b_next, mp->b_rptr);
    printf("     *b_prev  = %x\t\t*b_wptr  = %x\n", mp->b_prev, mp->b_wptr);
    printf("     *b_cont  = %x\t\t*b_datap = %x\n", mp->b_cont, mp->b_datap);
    printf("\n");
}


/*
 *	Print data block descriptor
 */

print_datab(dbp)
struct datab		*dbp;
{

    printf("     *db_freep= %x\t\tdb_ref   = %x\n", dbp->db_freep, dbp->db_ref);
    printf("     *db_base = %x\t\tdb_type  = %x\n", dbp->db_base, dbp->db_type);
    printf("     *db_lim  = %x\t\tdb_class = %x\n", dbp->db_lim, dbp->db_class);
    printf("\n");
}

/*
 *	Print a block of data
 */

char	*hexchar = "0123456789abcdef";

static
print_data(from, length)
register char	*from;
register int	length;
{
	register int	value;
	register int	i1, i2;
	register char	*row_start;
	register char	*end = from + length;

	while (from < end)
	{
	    row_start = from;
	    printf("%x: ", row_start);
	    i2 = (from + 16 < end) ? 16 : end - from;
	    for (i1=0; i1 < i2; i1++)
	    {
		if ((i1 & 03) == 3)
		    printf(" ");
		value = *from;
		printf("%c", hexchar[(value>>4) & 0xf]);
		printf("%c", hexchar[(value) & 0xf]);
		++from;
	    }
	    printf(" ");
	    for(from = row_start; i2; i2--, from++)
	    {
		if(*from >= ' ' && *from < 0177)
		    printf("%c", *from);
		else
		    printf("%c", '.');
	    }
	    printf("\n");
	}
}




print_message(mp)
struct msgb		*mp;

{
	while (mp)
	{
	    print_mblk(mp);
	    if (mp->b_datap) {
	    	print_datab(mp->b_datap);
	    	print_data(mp->b_rptr, mp->b_wptr - mp->b_rptr); 
	    }
	    mp = mp->b_cont;
	}
}
