#ifndef lint	/* .../appletalk/at/lib/at_dec_en.c */
#define _AC_NAME at_dec_en_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:28}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_dec_en.c on 87/11/11 20:58:28";
  static char *Version1 = "@(#)at_lib.c	5.2 10/13/86 (at_lib.c version)";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |	   #    #######         #         ###   ######           #####      |
  |	  # #      #            #          #    #     #         #     #     |
  |	 #   #     #            #          #    #     #         #           |
  |	#     #    #            #          #    ######          #           |
  |	#######    #            #          #    #     #   ###   #           |
  |	#     #    #            #          #    #     #   ###   #     #     |
  |	#     #    #    ####### #######   ###   ######    ###    #####      |
  |                                                                         |
  |                                                                         |
  |                                                                         |
  +-------------------------------------------------------------------------+
  |
  |  Description:
  |
  |  SCCS:
  |      @(#)at_lib.c	5.2 10/13/86
  |
  |  Copyright:
  |      Copyright 1986 by UniSoft Systems Corporation.
  |
  |  History:
  |      24-Jun-85: Created by Philip K. Ronzone.
  |
  +*/

#include "appletalk.h"

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


int at_decompose_en(en,enlen,optr,olen,tptr,tlen,zptr,zlen)
        char   *en;
        int     enlen;
        char   *optr;
        int    *olen;
        char   *tptr;
        int    *tlen;
        char   *zptr;
        int    *zlen;
        {
        char   *fn;
        char    object [AT_ENF_SIZE];
        char    type   [AT_ENF_SIZE];
        char    zone   [AT_ENF_SIZE];
        int     object_len;
        int     type_len;
        int     zone_len;
        int     remaining_en_length;
        int     maxlen;

        fn = "at_decompose_en";

        /*------------------------------------*/
        /* Check the entity name for validity */
        /*------------------------------------*/
        if (enlen == 0)
            {
            enlen = strlen (en);
            }
        if (!(1 <= enlen && enlen <= AT_NBP_TUPLE_STRING_MAXLEN))
            {
            at_error(1750,fn,"entity str length range error (%d)", enlen);
            return(-1);
            }

        if (enlen != 0)
            {
            remaining_en_length = enlen;
            }
        else
            {
            remaining_en_length = strlen(en);
            }

        /*-----------------------------------------------------------*/
        /* Decompose the object name part of the entity name string. */
        /*-----------------------------------------------------------*/
        if (remaining_en_length > AT_NBP_TUPLE_STRING_MAXLEN)
            {
            maxlen = AT_NBP_TUPLE_STRING_MAXLEN;
            }
        else
            {
            maxlen = remaining_en_length;
            }
        if (*en == '=')
            {
            object[0] = '=';
            object_len = 1;
            en++;
            }
        else
            {
            for (object_len = 0;  object_len < maxlen;  object_len++)
                {
                if (*en == ':')
                    {
                    break;
                    }
                else
                    {
                    if (' ' <= *en  &&  *en <= '~')
                        {
                        if (*en == ':')
                            {
                            break;
                            }
                        else if (*en != '='  &&  *en != '*'  &&
                                 *en != ':'  &&  *en != '@')
                            {
                            object[object_len] = *en++;
                            }
                        else
                            {
                            at_error(1751,fn,
                                     "illegal character in object field, `%c'",
                                     *en);
                            return(-1);
                            }
                        }
                    else
                        {
                        at_error(1752,fn,"invalid character in object field, 0x%x",
                                 (u8)*en);
                        return(-1);
                        }
                    }
                }
            }
        if (object_len < 1)
            {
            at_error(1753,fn,"missing object field");
            return(-1);
            }
        if (*en != ':')
            {
            at_error(1754,fn,"missing `:' after object name");
            return(-1);
            }
        object[object_len] = '\0';
        remaining_en_length -= object_len + 1;
        en++;

        /*---------------------------------------------------------*/
        /* Decompose the type name part of the entity name string. */
        /*---------------------------------------------------------*/
        if (remaining_en_length > AT_NBP_TUPLE_STRING_MAXLEN)
            {
            maxlen = AT_NBP_TUPLE_STRING_MAXLEN;
            }
        else
            {
            maxlen = remaining_en_length;
            }
        if (*en == '=')
            {
            type[0] = '=';
            type_len = 1;
            en++;
            }
        else
            {
            for (type_len = 0;  type_len < maxlen;  type_len++)
                {
                if (*en == '@')
                    {
                    break;
                    }
                else
                    {
                    if (' ' <= *en  &&  *en <= '~')
                        {
                        if (*en == ':')
                            {
                            break;
                            }
                        else if (*en != '='  &&  *en != '*'  &&
                            *en != ':'  &&  *en != '@')
                            {
                            type[type_len] = *en++;
                            }
                        else
                            {
                            at_error(1755,fn,
                                     "illegal character in type field, `%c'",
                                     *en);
                            return(-1);
                            }
                        }
                    else
                        {
                        at_error(1756,fn,"invalid character in type field, 0x%x",
                                 (u8)*en);
                        return(-1);
                        }
                    }
                }
            }
        if (type_len < 1)
            {
            at_error(1757,fn,"missing type field");
            return(-1);
            }
        if (*en != '@')
            {
            at_error(1758,fn,"missing `@' after type name");
            return(-1);
            }
        type[type_len] = '\0';
        remaining_en_length -= type_len + 1;
        en++;

        /*---------------------------------------------------------*/
        /* Decompose the zone name part of the entity name string. */
        /*---------------------------------------------------------*/
        if (remaining_en_length > AT_NBP_TUPLE_STRING_MAXLEN)
            {
            maxlen = AT_NBP_TUPLE_STRING_MAXLEN;
            }
        else
            {
            maxlen = remaining_en_length;
            }
        if (*en == '*')
            {
            zone[0] = '*';
            zone_len = 1;
            en++;
            }
        else
            {
            for (zone_len = 0;  zone_len < maxlen;  zone_len++)
                {
                if (*en == '\0')
                    {
                    break;
                    }
                else
                    {
                    if (' ' <= *en  &&  *en <= '~')
                        {
                        if (*en != '='  &&  *en != ':'  &&
                            *en != '@'  &&  *en != '@')
                            {
                            zone[zone_len] = *en++;
                            }
                        else
                            {
                            at_error(1759,fn,
                                     "illegal character in zone field, `%c'",
                                     *en);
                            return(-1);
                            }
                        }
                    else
                        {
                        at_error(1760,fn,"invalid character in zone field, 0x%x",
                                 (u8)*en);
                        return(-1);
                        }
                    }
                }
            }
        if (zone_len < 1)
            {
            at_error(1761,fn,"missing zone field");
            return(-1);
            }
        zone[zone_len] = '\0';
        remaining_en_length -= zone_len;

        /*----------------------------------------------------------------*/
        /* Check for a null terminated byte after the entity name ONLY in */
        /* those cases where the entity name length was NOT specified.    */
        /*----------------------------------------------------------------*/
        if (enlen == 0)
            {
            if (*en != '\0')
                {
                at_error(1762,fn,"missing null byte after zone name, 0x%x",
                         (u8)*en);
                return(-1);
                }
            }
        if (remaining_en_length != 0)
            {
            at_error(1763,fn,"extraneous characters after zone name, %d",
                     remaining_en_length);
            return(-1);
            }
        if (optr != (char *)NULL)
            {
            (void) memcpy(optr,object,object_len+1);
            }
        if (olen != (int *)NULL)
            {
            *olen = object_len;
            }
        if (tptr != (char *)NULL)
            {
            (void) memcpy(tptr,type,type_len+1);
            }
        if (tlen != (int *)NULL)
            {
            *tlen = type_len;
            }
        if (zptr != (char *)NULL)
            {
            (void) memcpy(zptr,zone,zone_len+1);
            }
        if (zlen != (int *)NULL)
            {
            *zlen = zone_len;
            }
        return(0);
        }
