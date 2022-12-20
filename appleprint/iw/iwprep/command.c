#ifndef lint	/* .../appleprint/iw/iwprep/command.c */
#define _AC_NAME command_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:28}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _command_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of command.c on 87/11/11 21:42:28";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"

extern int cmd_comment();
extern int cmd_name();
extern int cmd_res();
extern int cmd_hor();
extern int cmd_vert();
extern int cmd_unitwidth();
extern int cmd_paperlength();
extern int cmd_paperwidth();
extern int cmd_font();

struct command command_table[] = {
    "#",            cmd_comment,
    "name",         cmd_name,
    "res",          cmd_res,
    "hor",          cmd_hor,
    "vert",         cmd_vert,
    "unitwidth",    cmd_unitwidth,
    "paperlength",  cmd_paperlength,
    "paperwidth",   cmd_paperwidth,
    "font",         cmd_font,
};

#define NUM_COMMANDS    (sizeof command_table) / (sizeof command_table[0])

/* command (argc, argv)
 *  compares word contained in argv[0] against command table
 *  if a match is found, performs the command
 *  if no match an error is returned
 *  returns 0 if everything is OK
 */
 
command (argc, argv)
    char            *argv[];
{
    register struct command     *cmd;
    int                          status;
    
    for (cmd = command_table; cmd < &command_table[NUM_COMMANDS]; cmd++) {
	if (strcmp (argv[0], cmd->c_name) == 0) {
/*          show (cmd->c_name, argc, argv);*/
	    status = (*cmd->c_func) (argc, argv);
	    break;
	}
    }
    if (cmd == &command_table[NUM_COMMANDS])
	return 1;
    else
	return status;
}


cmd_comment (argc, argv)
    char            *argv[];
{
    return 0;
}

show (name, argc, argv)
    char        *name;
    char        *argv[];
{
    int          ix;
    
    printf ("%s:", name);
    for (ix = 0; ix < argc; ix++)
	printf ("av[%d]=%s,", ix, argv[ix]);
    printf ("\n");
}

