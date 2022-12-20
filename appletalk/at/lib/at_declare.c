#ifndef lint	/* .../appletalk/at/lib/at_declare.c */
#define _AC_NAME at_declare_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:33}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_declare.c on 87/11/11 20:58:33";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#define DEF_DATA
#define  AT_NO_INCLUDE
#include <fcntl.h>
#include "appletalk.h"

extern  void    exit();

#ifdef DEF_DATA
int       at_error_status		 = AT_ERROR_ON;
int       at_bridge_node                 = 0;
char     *at_chooser_msg                 =
          "\n%s: ITEM number (0 to make no selection)? ";
int       at_chooser_msg_arg1            = 0;
int       at_chooser_msg_arg2            = 0;
int       at_chooser_net                 = 0;
int       at_chooser_node                = 0;
char      at_chooser_object[AT_NBP_TUPLE_STRING_MAXLEN+1] = {0};
int       at_chooser_print_net           = 1;
char     *at_chooser_print_net_fmt       = "0x%x";
char     *at_chooser_print_net_header    = "NET";
int       at_chooser_print_node          = 1;
char     *at_chooser_print_node_fmt      = "0x%x";
char     *at_chooser_print_node_header   = "NODE";
int       at_chooser_print_object        = 1;
char     *at_chooser_print_object_header = "OBJECT";
int       at_chooser_print_socket        = 1;
char     *at_chooser_print_socket_fmt    = "0x%x";
char     *at_chooser_print_socket_header = "SOCKET";
int       at_chooser_print_type          = 1;
char     *at_chooser_print_type_header   = "TYPE";
int       at_chooser_print_zone          = 1;
char     *at_chooser_print_zone_header   = "ZONE";
int       at_chooser_socket              = 0;
FILE     *at_chooser_stdin               = stdin;
FILE     *at_chooser_stdout              = stdout;
char      at_chooser_type[AT_NBP_TUPLE_STRING_MAXLEN+1]   = {0};
char      at_chooser_zone[AT_NBP_TUPLE_STRING_MAXLEN+1]   = {0};
at_ddp_t  at_datagram			 = {0,0,0,0,0,0,0,0,0,0};
int       at_errno			 = 0;
int       at_got_net_number              = 0;
int       at_net_number                  = 0;
char      at_network[MAXNAMLEN+1] 	 = {0};
int       at_node_number                 = 0;
int       at_nve_lkup_reply_count        = 0;
at_nve   *at_nve_lkup_reply_head         = (at_nve*)NULL;
at_nve   *at_nve_lkup_reply_tail         = (at_nve*)NULL;
char     *at_pgm                         = NULL;
int       at_pid                         = 0;
