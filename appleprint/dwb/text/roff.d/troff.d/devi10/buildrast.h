#ifndef lint	/* .../appleprint/dwb/text/roff.d/troff.d/devi10/buildrast.h */
#define _AC_NAME buildrast_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:03:53}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *buildrast_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of buildrast.h on 87/11/11 22:03:53";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS

/*
 *
 * Some important definitions used by the program that builds raster tables
 * and troff's ASCII files from Imagen supplied raster files.
 *
 */


#define	L_CHNAME	2		/* longest special character name */
#define MAX_INDEX	128		/* largest allowed glyph number */
#define GLYDIR_SIZE	15 * MAX_INDEX	/* enough room for a whole directory */


typedef struct  {

	char	name[L_CHNAME+1];	/* special char names saved here */

} Charset;

typedef struct  {

	char	name[L_CHNAME+1];	/* name of the special character */
	char	*map;			/* address of the bitmap */
	int	mapsize;		/* size in bytes of the bitmap */
	int	chwidth;		/* new char width if >= 0 */
	char	*glydir;		/* new glyph directory entry */
	char	rastname[15];		/* glyph comes from this file */
	int	index;			/* glyph number in *rastname */
	char	synonyms[100];		/* synonyms that go in font file */

} Charinfo;


extern unsigned	readvalue();


