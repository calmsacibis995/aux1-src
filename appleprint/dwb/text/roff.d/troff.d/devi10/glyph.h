#ifndef lint	/* .../appleprint/dwb/text/roff.d/troff.d/devi10/glyph.h */
#define _AC_NAME glyph_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:04:47}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *glyph_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of glyph.h on 87/11/11 22:04:47";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS

struct Fontheader
  {
	short		f_magic ;	/* Should be 0436 */
	unsigned short	f_size ;	/* Total size of raster section
					 * of the font file.
					 */
	short		f_maxx ;	/* Largest x of character */
	short		f_maxy ;	/* Largest y of character */
	short		f_xtnd ;	/* Largest extender */
  } ;

struct Charparam
  {
	unsigned short	c_addr ;	/* Offset in file where character
					 * is located.
					 */
	short		c_size ;	/* Size in bytes of character	*/
	char		c_up ;		/* c_up + c_down = heigth of	*/
	char		c_down ;	/* raster in font file.		*/
	char		c_left ;	/* c_right + c_left = width of	*/
	char		c_right ;	/* raster in font file.		*/
	char		c_width ;	/* Width, including space required
					 * to print character.
					 */
  } ;

/*	RSTRUCTS is the size of the structures at the beginning of a	*/
/*	Versatec raster file.  RSTRUCTS + (long)charparam->c_addr is	*/
/*	the address of the character's raster.				*/

#define	RSTRUCTS	((long)(sizeof(struct Fontheader)+ \
			    256*sizeof(struct Charparam)))
