#ifndef lint	/* .../appleprint/iw/daiw/local.h */
#define _AC_NAME local_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:45:14}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _local_h[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of local.h on 87/11/11 21:45:14";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	(none)
 *
 *  Arguments:
 *	(none)
 *
 *  Description:
 *	This file is the header file for all modules in the dlp program.
 *	Note that all data declarations are in this file also. The storage
 *	class preprocessor name (SCLASS) is set to `extern' for all modules
 *	that include "local.h", EXCEPT for data.c, where extern is set to a
 *	C comment. This causes all data variables to `reside' in data.c.
 *
 *  Algorithm:
 *	(none)
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <mac/types.h>
#include <mac/quickdraw.h>

#include "dev.h"

extern	int	errno;
extern	void	exit();
extern	int	fprintf();
extern	void	free();
extern	char   *malloc();
extern	int	printf();
extern	int	sys_nerr;
extern	char   *sys_errlist[];


extern	void	badinput();
extern  void    clear_page ();
extern	void	dbg_cc();
extern	void	dbg_cc1();
extern  void	dev_init();
extern	void	do_Cxy();
extern	void	do_D();
extern	void	do_Hn();
extern	void	do_Vn();
extern  void	do_cx();
extern  void	do_fn();
extern  void	do_hn();
extern  void	do_nb_a();
extern  void	do_nnc();
extern  void	do_pn();
extern  void	do_sn();
extern  void	do_vn();
extern  void	do_x();
extern	int	eatc();
extern	int	eatnnn();
extern	void	eatword();
extern  void	eatline();
extern  void	font_init();
extern	void	harderr();
extern	void	initme();
extern  void    inittb();
extern  void	init_printer ();
extern  void	load_font ();
extern  struct iw2   *open_printer ();
extern  GrafPtr	open_prport();
extern	void	pageflush();
extern	void	pf_lp();
extern  void	print_page ();
extern	void	process();
extern	void	softerr();
extern  char   *strsave();
extern	void	stuffc();
extern	void	uneatc();

#define TROFF_ROOT	"/usr/lib/font"	/* base directory for fonts */
#define NFONT		60		/* maximum number of fonts */

/* standard paper sizes */
#define IW2_STD_PORT	1
#define STD_HEIGHT_PORT	11
#define STD_WIDTH_PORT	8.5

#define IW2_STD_LAND	2
#define STD_HEIGHT_LAND	8.5
#define STD_WIDTH_LAND	11

/* resolutions */
#define IW2_HIGH_RES	144
#define IW2_FASTER_RES	72

#define NEW(type) ((type *) malloc (sizeof (type)))
#define NEWN(type, count) ((type *) malloc (sizeof (type) * (count)))

#define is_odd(x)	((x) & 01)
#define is_even(x)	(!is_odd(x))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))

#define	MAXDBG	16	/* Maximum number of debug switches.		     */
#define	CCNCHR	 4	/* Maximum number of characters to expect in Cxy.    */

typedef	struct	cc	/* Character control structure.                      */
	{
	struct  cc     *cc_next;
	struct  cc     *cc_prev;
	short		cc_vert;
	short		cc_horz;
	short		cc_font;
	short		cc_ptsz;
	char		cc_char[CCNCHR];
	char		cc_guard;
	}		CC;

struct iw2 {
    char	*d_name;	/* the devices name */
    FILE	*d_fp;		/* its I/O port */
    short	 d_res;		/* printing resolution */
    short	 d_hor;		/* horizontal dimension */
    short	 d_vert;	/* vertical dimension */
};

struct font_pos {
    int		 fp_index;	/* its index */
    char	*fp_name;	/* the fonts name */
    struct font *fp_font;	/* its internal font structure */
};

#define IS_HIGH_RES(device)	(device->d_res == IW2_HIGH_RES)

#ifdef	DATA
#define	SCLASS	/* */
#else
#define	SCLASS	extern
#endif

SCLASS	CC     *cchead;		/* Start of initial CC linked list.          */
SCLASS	CC     *cctail;		/* End of CC linked list.		     */
SCLASS	int	cur_font;	/* Current font.			     */
SCLASS	int	hwm_vert;	/* High Water Mark, Vertical.                */
SCLASS	int	init_mz;	/* Initial memory sizing.		     */
SCLASS	int	inpcount;	/* Count of chars. read from input so far.   */
SCLASS	int	linecnt;	/* Lines read from input.		     */
SCLASS	int	linecol;	/* Line col. (char. pos.) in current line.   */
SCLASS	int	oldlcol;	/* Old line column (ungetc handling).        */
SCLASS	int	pageno;		/* The current page number.		     */
SCLASS	char   *pgm;		/* Name of program (from argv[0]).	     */
SCLASS	int	pointsz;	/* Current point size.			     */
SCLASS	int	pos_horz;	/* Current typesetting position, horizontal. */
SCLASS	int	pos_vert;	/* Current typesetting position, vertical.   */
SCLASS  int	tpos_horz;	/* troffs current horizontal position	     */
SCLASS  int	tpos_vert;	/* troffs current vertical position	     */
SCLASS	int	sw_debug[MAXDBG]; /* The -d argument boolean switch.	     */
SCLASS	int	x_H_n;		/* Character height.			     */
SCLASS	int	x_S_n;		/* Character slant.			     */
SCLASS	char	x_T_s[128];	/* Typesetter name.			     */
SCLASS	int	x_r_n;		/* Resolution per inch.			     */
SCLASS	int	x_r_h;		/* Horizontal resolution (units of x_r_n).   */
SCLASS	int	x_r_v;		/* Vertical resolution (units of x_r_n).     */
SCLASS	int	x_f_n;		/* Last font position font number.	     */
SCLASS	char	x_f_s[128][64];	/* Font name as would be indexed by font no. */

SCLASS	struct dev   dev;	/* the troff device 			     */
SCLASS	u_char 	    *dev_data;  /* holds ENTIRE device description 	     */
SCLASS	u_short	    *sizes;	/* sizes available 			     */
SCLASS	u_short	    *ch_tab;	/* special character index table 	     */
SCLASS	u_char 	    *ch_name;	/* the special characters 		     */
SCLASS	struct font *fonts[NFONT+1];   /* the font descriptors 		     */
SCLASS	u_char	    *widths[NFONT+1];  /* character widths 		     */
SCLASS	u_char	    *kerning[NFONT+1]; /* really ascender-descender information */
SCLASS	u_char	    *codes[NFONT+1];   /* character codes 		     */
SCLASS	u_char	    *fitab[NFONT+1];   /* character availability table 	     */

SCLASS  struct font_pos font_pos[NFONT+1]; /* font position translation table */

SCLASS  int	     dev_scale; /* scale factor for device resolution        */
SCLASS  int	     parm_res;  /* resolution on command line		     */
SCLASS  struct iw2  *iw2;	/* the printer */
SCLASS  GrafPtr      gp;	/* and its grafport */


/*

Debug switches:

sw_debug[0]  General debug output. Nothing too filling.

sw_debug[1]  Dump the CC linked list at the beginning of the page flush.

sw_debug[2]  Dump the CC linked list after each ``character'' is stuffed
             onto it.

sw_debug[3]  Dump the CC entry upon each ``stuff'' into the linked list.

sw_debug[4]  Dump the CC entry when printing page

*/
