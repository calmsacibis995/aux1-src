#ifndef lint	/* .../sys/COMMON/os/vfs_conf.c */
#define _AC_NAME vfs_conf_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:13:09}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of vfs_conf.c on 87/11/11 21:13:09";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)vfs_conf.c 1.1 86/02/03 SMI	*/
/*	@(#)vfs_conf.c	2.1 86/04/15 NFSSRC */

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#undef	NFS
#include "sys/mount.h"

int	novfs_entry();
struct vfsops novfs_vfsops = {
	novfs_entry,
	novfs_entry,
	novfs_entry,
	novfs_entry,
	novfs_entry,
};
extern	struct vfsops svfs_vfsops;	/* XXX Should be ifdefed */

struct vfsops *vfssw[MOUNT_MAXTYPE + 1] = {
	&svfs_vfsops,		/* MOUNT_UFS */
	&novfs_vfsops,		/* MOUNT_NFS */
	&novfs_vfsops,		/* MOUNT_PC (== MOUNT_MAXTYPE) */
};

novfs_entry()
{
	return (ENODEV);
}
