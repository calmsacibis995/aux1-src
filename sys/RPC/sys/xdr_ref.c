#ifndef lint	/* .../sys/RPC/sys/xdr_ref.c */
#define _AC_NAME xdr_ref_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:31:18}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of xdr_ref.c on 87/11/11 21:31:18 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif

/*
 * xdr_reference.c, Generic XDR routines impelmentation.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * These are the "non-trivial" xdr primitives used to serialize and de-serialize
 * "pointers".  See xdr.h for more info on the interface to xdr.
 */

#ifdef KERNEL
#include "sys/param.h"
#include "sys/signal.h"
#include "sys/types.h"
#include "rpc/types.h"
#include "rpc/xdr.h"
#else
#include "types.h"	/* <> */
#include "xdr.h"	/* <> */
#include <stdio.h>
#endif
#define LASTUNSIGNED	((u_int)0-1)

/*
 * XDR an indirect pointer
 * xdr_reference is for recursively translating a structure that is
 * referenced by a pointer inside the structure that is currently being
 * translated.  pp references a pointer to storage. If *pp is null
 * the  necessary storage is allocated.
 * size is the sizeof the referneced structure.
 * proc is the routine to handle the referenced structure.
 */
bool_t
xdr_reference(xdrs, pp, size, proc)
	register XDR *xdrs;
	caddr_t *pp;		/* the pointer to work on */
	u_int size;		/* size of the object pointed to */
	xdrproc_t proc;		/* xdr routine to handle the object */
{
	register caddr_t loc = *pp;
	register bool_t stat;

	if (loc == NULL)
		switch ((int) xdrs->x_op) {
		case XDR_FREE:
			return (TRUE);

		case XDR_DECODE:
			*pp = loc = (caddr_t) mem_alloc(size);
#ifndef KERNEL
			if (loc == NULL) {
				fprintf(stderr,
				    "xdr_reference: out of memory\n");
				return (FALSE);
			}
#endif
			bzero(loc, (int)size);
			break;
	}

	stat = (*proc)(xdrs, loc, LASTUNSIGNED);

	if (xdrs->x_op == XDR_FREE) {
		mem_free(loc, size);
		*pp = NULL;
	}
	return (stat);
}
