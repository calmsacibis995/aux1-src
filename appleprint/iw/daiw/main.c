#ifndef lint	/* .../appleprint/iw/daiw/main.c */
#define _AC_NAME main_c
#define _AC_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.4 87/11/11 21:44:46}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _main_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of main.c on 87/11/11 21:44:46";
  char *_Version_ = "A/UX Release 1.0";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

#define _AC_MODS

int	noEvents = 1;		/* this gives the keyboard back when running
				   toolbox applications */
int	noCD = 1;		/* this tells the toolbox not to change
				   directories upon execution */

main(argc,argv)
    int		argc;
    char      **argv;
{
    int		dbgvalue;
    FILE       *inpfile;
    FILE       *outfile;
    int		c;
    int		errflag = 0;
    extern int	optind;
    extern char *optarg;

    pgm = argv[0];
    inpfile = stdin;
    outfile = stdout;
    initme ();
    inittb ();
    while ((c = getopt (argc, argv, "o:d:r:")) != EOF) {
	switch (c) {
	case 'o':
	    outfile = fopen (optarg, "w");
	    if (outfile == NULL) {
		harderr(203,"can't open %s for output", optarg);
		return(1);
	    }
	    break;
	case 'd':
	    dbgvalue = atoi(optarg);
	    if (dbgvalue < MAXDBG)
		sw_debug[dbgvalue] = 1;
	    else {
		softerr(202,"Debug switch %d too big (max %d)", dbgvalue,MAXDBG-1);
		errflag++;
	    }
	    break;
	case 'r':
	    parm_res = atoi (optarg);
	    if (parm_res == 72)
		dev_scale = 2;
	    else if (parm_res == 144)
		dev_scale = 1;
	    else {
		softerr (202, "-r resolution must be 72 or 144 (%d)", parm_res);
		errflag++;
	    }
	    break;
	case '?':
	    errflag++;
	    break;
	}
    }
    if (errflag) {
	fprintf (stderr, "Usage: %s -r resolution file...\n", pgm);
	exit (1);
    }
    if (parm_res == 0) {
	parm_res = 144;
	dev_scale = 1;
    }
    if (optind == argc)
	process (inpfile, outfile);
    else {
	for (; optind < argc; optind++) {
	    inpfile = fopen (argv[optind], "r");
	    if (inpfile == NULL)
		softerr (206,"can't open input file %s",argv[optind]);
	    else {
		process (inpfile,outfile);
		fclose (inpfile);
	    }
	}
    }
    exit (0);
}
