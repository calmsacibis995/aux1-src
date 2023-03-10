#ifndef lint	/* .../appleprint/dwb/text/tbl.d/tr.c */
#define _AC_NAME tr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:14:56}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of tr.c on 87/11/11 22:14:56";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
# include "t..c"
 /* tr.c: number register allocation */
char * nregs[] ={
	/* this array must have at least 3*qcol entries
	   or illegal register names will result */
	"40","41","42","43","44","45","46","47","48","49",
	"50","51","52","53","54","55","56","57","58","59",
	"60","61","62","63","64","65","66","67","68","69",
	"70","71","72","73","74","75","76","77","78","79",
	"80","81","82","83","84","85","86","87","88","89",
	"90","91","92","93","94","95","96","97","4q","4r",
	"4s","4t","4u","4v","4w","4x","4y","4z","4;","4.",
	"4a","4b","4c","4d","4e","4f","4g","4h","4i","4j",
	"4k","4l","4m","4n","4o","4p","5a","5b","5c","5d",
	"5e","5f","5g","5h","5i","5j","5k","5l","5m","5n",
	"5o","5p","5q","5r","5s","5t","5u","5v","5w","5x",
	0};
char *
reg(col, place)
{
if(sizeof(nregs) < 2*3*qcol)
	error("Too many columns for registers");
return (nregs[qcol*place+col]);
}
