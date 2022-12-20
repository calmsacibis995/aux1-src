#ifndef lint	/* .../sys/psn/io/uinter.c */
#define _AC_NAME uinter_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.11 87/11/19 18:04:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.11 of uinter.c on 87/11/19 18:04:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)uinter.c	UniPlus VVV.2.1.5	*/
/*
 *	This is the user interface 'driver'
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/mmu.h>
#include <sys/time.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/video.h>
#include <sys/uinter.h>
#include <sys/mouse.h>
#include <sys/key.h>
#include <sys/region.h>
#include <sys/pfdat.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/tuneable.h>
#include <sys/systm.h>
#include <sys/debug.h>

/*
    Now, for a word about DO_FORK_STUFF.  Auto-config arranges to have the
routines ui_fork, ui_exec, and ui_exit called whenever those system calls are
executed.  We use this to automatically delete a processes layer whenever it
execs or exits.  We would like to use ui_fork to unattach a child process from
its parent's layer when the child process is created.  Unfortunately, ui_fork
is called by the parent process, not the child process.  I think this is a bug.
If it ever gets fixed, we should enable the code now ifdef'd out with
DO_FORK_STUFF.  In the meantime, we will just have to live with the fact that
a child process may still be attached to its parents layer.
*/

/*
 *	This define helps the compiler to produce slightly better
 *		code for the case where the only possible device number is 0.
 *		For now, this is always true.
 */

#if NDEVICES == 1
#   define DEV	0
#else
#   define DEV	dev
#endif NDEVICES

extern union ptbl *segvptbl();
extern ui_exit();
extern ui_display();
extern ui_keyboard();
extern ui_mouse();
extern ui_catch_timeout();
extern wakeup();

/*
 *	bit masks used for encoding events
 */

static short ui_mask[] = {
	0x0001,	0x0002,	0x0004,	0x0008,	0x0010,	0x0020,	0x0040,	0x0080,
	0x0100,	0x0200,	0x0400,	0x0800,	0x1000,	0x2000,	0x4000,	0x8000,
};


typedef int (*procedure_t)();
static procedure_t ui_vtrace[NDEVICES];		/* the old vtrace routine */
static unsigned char ui_cursor[NDEVICES];	/* sais cursor is 'on' */
static unsigned char ui_devices[NDEVICES];	/* sais devices are 'on' */
static pte_t *ui_pt[NDEVICES];			/* page table pointer for shm
						   page */
static reg_t *ui_rp[NDEVICES];			/* region pointer for shm
						   page */
static unsigned char ui_key_open[NDEVICES];	/* the key state before we */
static unsigned long ui_key_mode[NDEVICES];	/* opened it */
static unsigned long ui_key_intr[NDEVICES];

static unsigned char ui_mouse_open[NDEVICES];	/* the mouse state before we */
static unsigned long ui_mouse_mode[NDEVICES];	/* opened it */
static unsigned long ui_mouse_intr[NDEVICES];

static char ui_inited[NDEVICES];		/* is this device is inited? */
static unsigned char ui_phantom[NDEVICES];	/* layer belonging to daemon */
static unsigned char ui_active[NDEVICES];	/* active layer for device */
static struct ui_interface *ui_addr[NDEVICES];	/* the user interface struct */
static struct layer ui_layer[NDEVICES][NLAYERS];/* The layers */
static struct proc *ui_owner[NDEVICES][NLAYERS];/* temporary sanity check */
static short ui_mx[NDEVICES];			/* For calc. deltas for mouse */
static short ui_my[NDEVICES];			/*    movement */

ui_open(dev)
register dev_t dev;
{
    dev = minor(dev);
    if (dev >= NDEVICES || dev >= video_count)	/* make sure the */
	return EINVAL;				/*   'device' exists */
    if (UI_FLAG&u.u_user[1])			/* if we are already */
	return EINVAL;				/*   connected to a */
						/*   interface, fail */
    if (!ui_inited[DEV])
	{					/* init device-sepecific stuff*/
	register int i;
	register struct layer *lp;

	ui_inited[DEV] = 1;
	ui_phantom[DEV] = NOLAYER;
	ui_active[DEV] = NOLAYER;
	for (i = 0,lp = &ui_layer[DEV][0]; i < NLAYERS; i++,lp++)
	    lp->l_state.state = LS_EMPTY;
#ifndef ACFG_SELECT	/* if autoconfig can't handle special select routine */
	{
#include <sys/conf.h>
	extern int ui_select();
	register struct cdevsw *ptr;

	for (ptr = cdevsw; ptr->d_open != ui_open; ptr++)
	    ;
	ptr->d_select = ui_select;
	}
#endif ACFG_SELECT
	}
    u.u_user[1] = UI_FLAG|UI_DL(DEV)|NOLAYER;	/* label our process */
						/*   as connected to */
						/*   this device */
    return 0;
}

ui_close(dev)
{
    return 0;
}

/*
 *	reading is not supported
 */

ui_read(dev,uio)
struct uio *uio;
{
    return EINVAL;
}

/*
 *	writing is not supported
 */

ui_write(dev, uio)
struct uio *uio;
{
    return EINVAL;
}

/*
 *	All the user interface action is done via ioctls
 */

ui_ioctl(dev, cmd, addr, arg)
int dev;
register caddr_t addr;
{
    register int s, i;
    register struct ui_interface *uip;
    register struct layer *lp;

#if NDEVICES != 0
    dev = minor(dev);
#endif NDEVICES
    switch(cmd)
	{
	case UI_GETVERSION:

	   /*
	    *	Return driver version number
	    */

	    u.u_rval1 = UI_VERSION;
	    break;

	case UI_SET:

	   /*
	    *	Set up a lineA trap handler
	    */

	    if ((uip = ui_addr[DEV]) == NULL ||
			(i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    uip->c_aline[i].a_handler = * (caddr_t *) addr;
	    u.u_user[0] = (long) &uip->c_aline[i].a_handler;
	    break;

	case UI_CLEAR:

	   /*
	    *	Clear a lineA trap handler
	    */

	    u.u_user[0] = 0;
	    break;

	case UI_SCREEN:

	   /*
	    *	map in the screens associated with a 'device' to the
	    *		address space requested by the process (this
	    *		must be on a segment boundary)
	    *	TODO:
	    *		1) Returned addresses should be indented
	    *		2) Try to restrict physed area to frame buffer
	    *		3) Round off start_address to segment boundary
	    */

	    {
	    register caddr_t address;
	    register struct screen *ptr;
	    register int slot;
	    register struct video **video;

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    lp = &ui_layer[DEV][i];
	    for (i = 0; i < NSCREENS; i++)
		if (lp->l_screen[i] != -1)
		    return EINVAL;		/* screens are already mapped */
	    ptr = &((struct screens *) addr)->screens[0];
	    address = ((struct screens *) addr)->start_address;
	    video = &video_desc[0];
	    for (i = 0; i < NSCREENS; i++,video++,ptr++)
		if ((*video)->video_slot == 0)
		    ptr->slot = -1;
		else
		    {
		    slot = (*video)->video_slot;
		    lp->l_screen[i] =
			ui_phys(address,1024*1024,(*video)->video_base);
		    if (lp->l_screen[i] != -1)
			{
			ptr->address = address +
				((*video)->video_addr - (*video)->video_base);
			ptr->slot = slot;
			address += 1024*1024;
			}
		    }
	    break;
	    }

	case UI_UNSCREEN:

	   /*
	    *	Unmap the screen.
	    *	NOTE:  this code needs to be changed to unmap all
	    *		the screens
	    */

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    lp = &ui_layer[DEV][i];
	    for (i = 0; i < NSCREENS; i++)
		if (lp->l_screen[i] >= 0)
		    dophys(lp->l_screen[i], 0, 0, 0);
	    return u.u_error;

	case UI_ROM:

	   /*
	    *	map in the ROM to the address space requested by the
	    *		process (this must be on a segment boundary)
	    */

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER ||
		    (lp = &ui_layer[DEV][i])->l_rom >= 0)
		return EINVAL;
	    lp->l_rom = ui_phys(*(caddr_t *)addr,256*1024,(caddr_t)0x40000000);
	    if (lp->l_rom < 0)
		return EINVAL;
	    break;

	case UI_UNROM:

	   /*
	    *	unmap the ROM
	    */

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER ||
		    (lp = &ui_layer[DEV][i])->l_rom < 0)
		return EINVAL;
	    dophys(lp->l_rom, 0, 0, 0);
	    lp->l_rom = -1;
	    return u.u_error;

	case UI_MAP:

	   /*
	    *	map in and lock down a page for the user interface
	    *		structure ... the address given must be a page
	    *		in a shared memory segment, and it must be on
	    *		a page boundary
	    */

	    ASSERT(sizeof(*uip) <= ptob(1));
	    return ui_lockit(DEV, *(caddr_t *)addr);

	case UI_UNMAP:

	   /*
	    *	unmap a shared memory page. Also undo any other ioctls
	    *		that have started actions (such as cursors or
	    *		keyboard events etc) that depend on this area.
	    *		Fails if there are live layers using the dev.
	    *		Does nothing if no page is mapped.
	    */

	    for (i = 0,lp = &ui_layer[DEV][0]; i < NLAYERS; i++,lp++)
		if (lp->l_state.state != LS_EMPTY)
		    return EINVAL;
	    if (ui_devices[DEV])
		ui_remdevices(DEV);
	    if (ui_cursor[DEV])
		ui_remcursor(DEV);
	    return ui_unlockit(DEV);

	case UI_CURSOR:

	   /*
	    *	display a mouse cursor and enable cursor tracking
	    */

	    {
	    register struct video *vp;

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    if (ui_cursor[DEV])
		return EINVAL;		/* until multi-layers! */
	    ui_layer[DEV][i].l_state.wanted |= C_WANTED;
	    if (ui_cursor[DEV])
		return 0;		/* already anabled */

					/* initialize cursor data */
	    uip = ui_addr[DEV];
	    uip->c_style = CUR_SMALL1;
	    uip->c_mlookup[0] = 4;
	    uip->c_mlookup[1] = 7;
	    uip->c_mlookup[2] = 10;
	    uip->c_mlookup[3] = 15;
	    uip->c_mlookup[4] = 20;
	    uip->c_mlookup[5] = 25;
	    uip->c_mlookup[6] = 30;
	    uip->c_mlookup[7] = 35;
	    uip->c_mlookup[8] = 39;
	    uip->c_mlookup[9] = 256;
	    uip->c_lock = 0;
	    uip->c_visible = 0;
	    uip->c_saved = 0;
	    uip->c_draw = 0;

	    ui_cursor[DEV] = 1;		/* mark it as on */
	    ui_mx[DEV] = mouse_x[DEV];	/* clear the deltas */
	    ui_my[DEV] = mouse_y[DEV];
	    vp = video_desc[DEV];
	    s = spl1();			/* turn on interrupts */
	    ui_vtrace[DEV] = vp->video_intr;
	    vp->video_intr = ui_display;
	    (*vp->video_func)(vp, VF_ENABLE, 0);
	    ui_display(vp);			/* display it */
	    splx(s);
	    break;
	    }

	case UI_UNCURSOR:

	   /*
	    *	Stop the display of a cursor (this doesn't remove
	    *		the cursor from the screen, it just
	    *		stops its being updated)  We only actually
	    *		turn the cursor off if nobody else wants it.
	    */

	    if (!ui_cursor[DEV] || (i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    lp = &ui_layer[DEV][i];
	    if (!(lp->l_state.wanted & C_WANTED))
		return EINVAL;		/* this proc never did UI_CURSOR */
	    lp->l_state.wanted &= ~C_WANTED;
	    for (i = 0,lp = &ui_layer[DEV][0]; i < NLAYERS; i++,lp++)
		if (lp->l_state.state == LS_INUSE &&
			(lp->l_state.wanted & C_WANTED))
		    return 0;
	    ui_remcursor(DEV);
	    break;

	case UI_DEVICES:

	   /*
	    *	This connects the devices to the mouse and keyboard
	    *		so that mouse and keyboard events can be
	    *		posted.
	    *	The catch here is that we must wait for the phantom process
	    *		to deal with any layers in the LS_DONE state.
	    *		This is so he can make correct decisions about
	    *		cleaning up the screen based on which layers
	    *		have asked for the screen.
	    */

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;

	    do		/* Wait for phantom process */
		for (s = 0,lp = &ui_layer[DEV][0]; s < NLAYERS; s++,lp++)
		    if (lp->l_state.state == LS_DONE)
			{
			(void) sleep((caddr_t) lp,PZERO+1);
			break;
			}
	    while (s != NLAYERS);

	    lp = &ui_layer[DEV][i];
	    lp->l_state.wanted |= PHANTOM_FLAG;
	    lp->l_state.wanted |= A_WANTED;
	    if (ui_devices[DEV])
		return 0;	/* already connected */

				/* initialize mouse/keyboard data */
	    uip = ui_addr[DEV];
	    uip->c_keythres = 16;
	    uip->c_keyrate = 4;
	    uip->c_modifiers = btnState;
	    uip->c_button = 0;
	    for (i = 0; i < 128; i++)
		uip->c_key[i] = NOLAYER;

	    ui_devices[DEV] = 1;
	    if (ui_mouse_open[DEV] = mouse_op(DEV, MOUSE_OP_OPEN, 0))
		{
		ui_mouse_intr[DEV] = mouse_op(DEV, MOUSE_OP_INTR, ui_mouse);
		ui_mouse_mode[DEV] = mouse_op(DEV, MOUSE_OP_MODE, 1);
		}
	    else if (mouse_open(DEV, ui_mouse, 1) == 0)
		return EINVAL;
	    if (ui_key_open[DEV] = key_op(DEV, KEY_OP_OPEN, 0))
		{
		ui_key_intr[DEV] = key_op(DEV, KEY_OP_INTR,ui_keyboard);
		ui_key_mode[DEV] = key_op(DEV, KEY_OP_MODE, KEY_ARAW);
		}
	    else if (key_open(DEV, ui_keyboard, KEY_ARAW) == 0)
		return EINVAL;
	    break;

	case UI_UNDEVICES:

	   /*
	    *	This restores the mouse and keyboard to their previous
	    *		owners (or closes them if they were not
	    *		previously in use)  We only actually do this
	    *		if no other layer still wants mouse/keyboard events.
	    */

	    if (!ui_devices[DEV] || (i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    lp = &ui_layer[DEV][i];
	    if (!(lp->l_state.wanted & A_WANTED))
		return EINVAL;		/* this proc never did UI_DEVICES */
	    lp->l_state.wanted &= ~A_WANTED;
	    for (i = 0,lp = &ui_layer[DEV][0]; i < NLAYERS; i++,lp++)
		if (lp->l_state.state == LS_INUSE &&
			(lp->l_state.wanted & A_WANTED))
		    return 0;
	    ui_remdevices(DEV);
	    break;

	case UI_DELAY:

	   /*
	    *	delay waits until lbolt reaches *addr and then returns the
	    *		current value of lbolt.  If a process wants to sleep
	    *		for n clock ticks, it should call UI_TICKCOUNT to find
	    *		the current value of lbolt, then call UI_DELAY to sleep
	    *		until lbolt reaches that value plus n.  We can't do it
	    *		all in one call because we want to sleep at PZERO+1 so
	    *		we can deliver signals.  That causes this call to be
	    *		restarted.  If the parameter to this call were a delta
	    *		instead of an absolute, we would start the delay over
	    *		again.  If enough signals came in (via setitimer?), we
	    *		would never return!
	    */

	    while (lbolt < * (unsigned long *) addr)
		{
		s = spl7();
		timeout(wakeup, (caddr_t)u.u_procp+1,
			* (unsigned long *) addr - lbolt);
		(void) sleep((caddr_t)u.u_procp+1, PZERO + 1);
		(void) splx(s);
		}

	    /* fall through */

	case UI_TICKCOUNT:

	   /*
	    *	this returns the current tickcount
	    */

	    *(unsigned long *)addr = lbolt;
	    break;

	case UI_DAEMON:

	   /*
	    *	This call establishes the caller as a device's "phantom"
	    *		process.  From now until he ceases to be the
	    *		phantom, which can only happen if he disposes of
	    *		his layer (perhaps by dying) he will be notified
	    *		of any layer deletions.  He will be notified by
	    *		having an event posted to his layer.  For now, we
	    *		have usurped the update event for this purpose.  If
	    *		the phantom ever decides to create windows, we will
	    *		have to change this.  The update event is special in
	    *		that only one gets 'queued' at a time, which is perfect
	    *		for us.  When the phantom gets this event, it should
	    *		respond by issuing UI_DAEMONGET and UI_DAEMONCLEAR calls
	    */

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER ||
			ui_phantom[DEV] != NOLAYER)
		return EINVAL;
	    ui_phantom[DEV] = i;
	    break;

	case UI_DAEMONGET:

	    /*
	     *	This call is made by the phantom when he receives an event
	     *		that says one or more layers have been deleted.  This
	     *		call returns to him an array telling him the status
	     *		of each layer.  He should then do whatever cleanup
	     *		work he wants to and call UI_DAEMONCLEAR which will
	     *		turn all of the LS_DONEs into LS_EMPTYs.
	     */

	    {
	    register struct state *ptr;

	    if (ui_phantom[DEV] == NOLAYER ||
		    UI_LAYER(u.u_user[1]) != ui_phantom[DEV])
		return EINVAL;
	    ptr = (struct state *) addr;
	    for (i = 0,lp = &ui_layer[DEV][0]; i < NLAYERS; i++,lp++)
		*ptr++ = lp->l_state;
	    break;
	    }

	case UI_DAEMONCLEAR:

	    /*
	     *	This call should be made by the phantom process after a
	     *		UI_DAEMONGET call.  It frees up the those layers
	     *		that are in LS_DONE limbo.
	     */

	    {
	    register struct state *ptr;

	    if (ui_phantom[DEV] == NOLAYER ||
		    UI_LAYER(u.u_user[1]) != ui_phantom[DEV])
		return EINVAL;
	    ptr = (struct state *) addr;
	    for (i = 0,lp = &ui_layer[DEV][0]; i < NLAYERS; i++,lp++)
		if ((ptr++)->state == LS_DONE)
		    if (lp->l_state.state == LS_DONE)/* should always be true */
			{
			lp->l_state.state = LS_EMPTY;
			wakeup(lp);
			}
	    break;
	    }

	case UI_POSTEVENT:

	   /*
	    *	This posts an event to the layer associated with the
	    *		current process
	    */

	    i = ui_postevent(DEV,UI_LAYER(u.u_user[1]),
		    ((struct postevent*)addr)->eventCode,
		    ((struct postevent*)addr)->eventMsg);
	    return i;

	case UI_LPOSTEVENT:

	   /*
	    *	This posts an event to the layer identified  by the
	    *		field 'layer' in the parameter
	    */

	    if ((i = ((struct lpostevent*)addr)->layer) < 0 ||
			i >= NLAYERS ||
			ui_layer[DEV][i].l_state.state != LS_INUSE)
		return EINVAL;
	    i = ui_postevent(DEV,i,
		    ((struct lpostevent*)addr)->eventCode,
		    ((struct lpostevent*)addr)->eventMsg);
	    return i;

	case UI_FLUSHEVENTS:

	   /*
	    *	This flushes events from the current processes
	    *		layer's queue
	    */

	    return ui_flushevents(DEV,((struct flushevents *)addr)->eventMask,
				      ((struct flushevents *)addr)->stopMask);

	case UI_GETOSEVENT:

	   /*
	    *	return the next event available from the event queue
	    */

	    return ui_getevent(
		    ((struct getosevent *) addr)->eventMask,
		    ((struct getosevent *) addr)->blocking,
		   &((struct getosevent *) addr)->theEvent);

	case UI_SETEVENTMASK:

	   /*
	    *	set the current layer's event mask
	    */

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    ui_layer[DEV][i].l_mask = *(short *)addr;
	    break;

	case UI_CREATELAYER:

	   /*
	    *	Create a new layer on the current device, if one is
	    *		not available return EAGAIN.
	    */

	    if ((uip = ui_addr[DEV]) == NULL ||
			UI_LAYER(u.u_user[1]) != NOLAYER)
		return EINVAL;
	    if ((i = ui_createlayer(DEV,uip)) < 0)
		return EAGAIN;
	    u.u_user[1] = UI_FLAG|UI_DL(DEV)|i;
	    u.u_rval1 = i;
	    break;

	case UI_UNLAYER:

	   /*
	    *	Dispose of a process's layer
	    */

	    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return EINVAL;
	    ui_killlayer(i,DEV);
	    u.u_user[1] = UI_FLAG|UI_DL(DEV)|NOLAYER;
	    break;

	case UI_SETLAYER:

	   /*
	    *	Set a named layer to be the current active one
	    *		for receiving new events
	    */

	    i = *(int *)addr;
	    if (i < 0 || i >= NLAYERS ||
			ui_layer[DEV][i].l_state.state != LS_INUSE)
		return EINVAL;
	    ui_active[DEV] = i;
	    break;

	case UI_GETVIDEODATA:	/* This call will probably go away when/if we */
				/* get a real slot manager. */
	    {
	    extern struct video *video_index[];
	    register struct getvideodata *g;

	    g = (struct getvideodata *) addr;
	    if (g->slot < 0 || g->slot > 15 || video_index[g->slot] == 0)
		return EINVAL;
	    g->video_data = video_index[g->slot]->video_data;
	    break;
	    }

	default:
	    return EINVAL;
	}
    return 0;
}

/*
    This routine creates a new layer.  If it succeeds, it returns the layer
number.  Otherwise, it returns -1.
*/

static ui_createlayer(dev,uip)
int dev;
register struct ui_interface *uip;
{
    register int i,l;
    register struct layer *lp;

			/* First, look for an unused layer */
    for (l = 0,lp = &ui_layer[DEV][0]; l < NLAYERS; l++,lp++)
	if (lp->l_state.state == LS_EMPTY)
	    break;
    if (l == NLAYERS)
	return -1;
			/* Found one, so initialize the new layer */
    lp->l_state.state = LS_INUSE;
    lp->l_state.wanted = NOTHING_WANTED;
    lp->l_sleep = 0;
    lp->l_select = 0;
    lp->l_down = 0;
    lp->l_timeout = 0;
    lp->l_mask = everyEvent & ~keyUpMask;
    lp->l_first = NULL;
    lp->l_last = NULL;
    lp->l_free = NULL;
    lp->l_rom = -1;
    for (i = 0; i < NSCREENS; i++)
	lp->l_screen[i] = -1;
    for (i = 0; i < NEVENTS; i++)
	{
	lp->l_events[i].next = lp->l_free;
	lp->l_free = &lp->l_events[i];
	}
    ui_owner[DEV][l] = u.u_procp;
    return l;
}

/*
    This routine destroys a layer.  If this device has a phantom process,
we wait until he acknowledges this action before moving on.  This allows
the phantom to do things upon layer destruction without worrying about
race conditions.  Note that anyone who tries to create a layer will also
have to wait until the phantom finishes with this one.
*/

static ui_killlayer(l,dev)
int l,dev;
{
    register struct layer *lp;
    int s;

    if (ui_owner[DEV][l] != u.u_procp)
	{
	printf("OUCH! process %d tried to delete %d's layer\n",
		u.u_procp->p_pid,ui_owner[DEV][l]->p_pid);
	printf("Bailiff, whack his pee pee!\n");
	return;
	}
    lp = &ui_layer[DEV][l];
    if (ui_active[DEV] == l)
	ui_active[DEV] = NOLAYER;
    if (l == ui_phantom[DEV])
	{				/* the phantom is relenquishing */
					/* his phantomhood, so wake up anyone */
					/* that was waiting for an ack */
	ui_phantom[DEV] = NOLAYER;
	lp->l_state.state = LS_EMPTY;
	for (l = 0,lp = &ui_layer[DEV][0]; l < NLAYERS; l++,lp++)
	    if (lp->l_state.state == LS_DONE)
		{
		lp->l_state.state = LS_EMPTY;
		wakeup(lp);
		}
	}
    else if (ui_phantom[DEV] == NOLAYER)
	{				/* no phantom to tell about this! */
	lp->l_state.state = LS_EMPTY;
	}
    else
	{
					/* post event to phantom */
					/* and wait for him to acknowledge */
	lp->l_state.state = LS_DONE;
	ui_postevent(DEV,ui_phantom[DEV],updateEvt,l);
	while (lp->l_state.state == LS_DONE)
	    (void) sleep((caddr_t) lp,PZERO);
	}
}

/*
    This routine disconnects a process from a user interface device.  If the
kill_layer flag is set, it also removes the caller's layer if one exists.
*/

static ui_unconnect(kill_layer)
int kill_layer;
{
    register int l;
#if NDEVICES != 1
    register int dev = UI_DEVICE(u.u_user[1]);
#endif NDEVICES

    if (UI_FLAG & u.u_user[1])
	{
	l = UI_LAYER(u.u_user[1]);
	ui_ioctl(DEV, UI_CLEAR, (caddr_t) 0, 0);
	ui_ioctl(DEV, UI_UNROM, (caddr_t) 0, 0);
	ui_ioctl(DEV, UI_UNSCREEN, (caddr_t) 0, 0);
#ifndef DO_FORK_STUFF
	if (l == NOLAYER || ui_owner[DEV][l] != u.u_procp)
	    kill_layer = 0;
#endif DO_FORK_STUFF
	if (kill_layer)
	    {
	    ui_ioctl(DEV, UI_UNCURSOR, (caddr_t) 0, 0);
	    ui_ioctl(DEV, UI_UNDEVICES, (caddr_t) 0, 0);
	    ui_ioctl(DEV, UI_UNLAYER, (caddr_t) 0, 0);
	    }
	u.u_user[1] = 0;
	}
}

/*
 *	This routine is called when a fork(2) occurs.  It undoes any
 *		mappings in effect, and dissassociates the new process
 *		from the layer.
 */

ui_fork()
{
#ifdef DO_FORK_STUFF
    ui_unconnect(0);
#endif DO_FORK_STUFF
}

/*
 *	This routine is called when an exec occurs ... layers are not
 *		inherited across exec(2)s
 */

ui_exec()
{
    ui_unconnect(1);
}

/*
 *	This routine is called whenever a process exit(2)s.
 */

ui_exit()
{
    ui_unconnect(1);
}

/*
 *	This routine posts events to layer l in device dev.
 *	Special processing is done for key down so that autokey events are
 *	delivered to processes.
 *	Update events are not queued but flagged instead
 */

static ui_postevent(dev, l, what, message)
int dev,l,what,message;
{
    register struct ui_interface *uip;
    register struct layer *lp;
    register struct event *ep;
    int s;

    if ((uip = ui_addr[DEV]) == NULL || l == NOLAYER)
	return EINVAL;
    lp = &ui_layer[DEV][l];
    if (!(lp->l_mask&ui_mask[what]))
	return 0;
    s = spl1();
    switch (what)
	{
	case keyDown:
	    if (!lp->l_down)
		lp->l_down = 1;
	    else if (lp->l_timeout)
		{
		untimeout(ui_catch_timeout, lp);
		lp->l_timeout = 0;
		}
	    lp->l_time = lbolt+uip->c_keythres;
	    lp->l_char = message;
				/* fall through... */

	case keyUp:
	    message <<= 8;
	    break;

	case updateEvt:
	    lp->l_update = 1;
	    goto out;
	}
    if (lp->l_free == NULL)
	{
	ep = lp->l_first;
	lp->l_first = ep->next;
	}
    else
	{
	ep = lp->l_free;
	lp->l_free = ep->next;
	}
    ui_seter(&ep->event,uip,what,message);
    ep->next = NULL;
    if (lp->l_first == NULL)
	lp->l_first = ep;
    else
	lp->l_last->next = ep;
    lp->l_last = ep;
out:
    if (lp->l_sleep&ui_mask[what])
	ui_wakeup(lp);
    splx(s);
    return 0;
}

/*
 *	This returns events from the caller's layer
 *		update events are returned
 *			with out any message (and only one of
 *			these is 'queued' at any one time)
 *		the keyUp/keyDown events do not return an
 *			encoded character value in the low byte
 *			of the message, only a key number
 *			in the next byte.
 *		the blocking parameter can be one of
 *			BLOCK   - block until an event is available
 *			NOBLOCK - return if no events are present
 *			AVAIL	- return if no events are present
 *				  but, don't remove event from queue
 *			MBLOCK	- block until an event is available or
 *				  the mouse moves -- note that the caller
 *				  supplies his idea of where the mouse
 *				  currently is in erp->where
 */

static ui_getevent(mask,blocking,erp)
int mask,blocking;
EventRecord *erp;
{
    register struct ui_interface *uip;
    register struct layer *lp;
    int l,s;
    register struct event *ep, *last;
#if NDEVICES != 1
    int dev = UI_DEVICE(u.u_user[1]);
#endif NDEVICES

    if ((uip = ui_addr[DEV]) == NULL)
	return EINVAL;
    if ((l = UI_LAYER(u.u_user[1])) == NOLAYER)
	return EINVAL;

    lp = &ui_layer[DEV][l];
    mask &= lp->l_mask;
    for(;;)
	{
	last = 0;
	s = spl1();
	for (ep = lp->l_first; ep != 0; ep = ep->next)
	    {
	    if (ui_mask[ep->event.what]&mask)
		{
		*erp = ep->event;		/* found one in the queue */
		if (blocking != AVAIL)
		    {				/* remove it from the queue */
		    if (last == 0)
			lp->l_first = ep->next;
		    else
			last->next = ep->next;
		    if (lp->l_last == ep)
			lp->l_last = last;
		    ep->next = lp->l_free;
		    lp->l_free = ep;
		    }
		splx(s);
		return 0;
		}
	    last = ep;
	    }
						/* nothing in the queue */
	if (lp->l_down && lp->l_time <= lbolt && (mask&autoKeyMask))
	    {					/* time for an auto-key */
	    if (blocking != AVAIL)
		lp->l_time = lbolt + uip->c_keyrate;
	    ui_seter(erp,uip,autoKey,lp->l_char<<8);
	    splx(s);
	    return 0;
	    }

	if (lp->l_update && (mask&updateMask))
	    {					/* there's an update event */
	    if (blocking != AVAIL)
		lp->l_update = 0;
	    ui_seter(erp,uip,updateEvt,0);
	    splx(s);
	    return 0;
	    }
						/* no events, now either */
						/* block or return null */
	switch(blocking)
	    {
	    case MBLOCK:
		if (uip->c_mx != erp->where.x || uip->c_my != erp->where.y)
		    goto null;
		lp->l_sleep |= mouseMoveMask;
						/* fall through... */
	    case BLOCK:
		if (lp->l_down && (mask&autoKeyMask))
		    {
		    lp->l_timeout = 1;
		    timeout(ui_catch_timeout, lp, lp->l_time - lbolt);
		    }
		lp->l_sleep |= mask;
		lp->l_select = 0;
		(void) sleep((caddr_t) lp, PZERO+1);
		break;

	    default:				/* should be NOBLOCK or AVAIL */
null:
		ui_seter(erp,uip,nullEvent,0);
		splx(s);
		return 0;
	    }
	}
}

static ui_flushevents(dev,eventMask,stopMask)
int dev;
register short eventMask, stopMask;
{
    register struct event *ep, *last, **parent;
    register struct layer *lp;
    register int i,s;

    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
	return EINVAL;
    lp = &ui_layer[DEV][i];
    parent = &lp->l_first;
    s = spl1();
    last = 0;
    while ((ep = *parent) != 0)
	{
	i = ui_mask[ep->event.what];
	if (i&stopMask)
	    break;
	if (i&eventMask)
	    {			/* skip over this event */
	    last = ep;
	    parent = &ep->next;
	    }
	else
	    {			/* delete this event */
	    *parent = ep->next;
	    if (lp->l_last == ep)
		lp->l_last = last;
	    ep->next = lp->l_free;
	    lp->l_free = ep;
	    }
	}
    splx(s);
    return 0;
}

/*
 *	This routine fills in the fields of an event record.  The what and
 *	message parameters are taken as arguments.  The other fields are
 *	taken from the current state of the machine.
 */

static ui_seter(erp,uip,what,message)
EventRecord *erp;
struct ui_interface *uip;
{
    erp->what = what;
    erp->message = message;
    erp->when = lbolt;
    erp->where.x = uip->c_mx;
    erp->where.y = uip->c_my;
    erp->modifiers = uip->c_modifiers;
}

/*
 *	This routine finds an empty phys id, and attempts to call dophys for
 *		that id.  Upon success, it returns the phys id.  Upon failure,
 *		-1 is returned.  u.u_error will not be affected by this routine.
 */

static int ui_phys(virtaddr,size,physaddr)
caddr_t virtaddr,physaddr;
unsigned int size;
{
    int u_error;
    register int i;
    register struct phys *p;

    u_error = u.u_error;
    u.u_error = 0;
    p = &u.u_phys[NPHYS - 1];
    for (i = NPHYS; --i >= 0; p--)
	if (p->u_phsize == 0)
	    {
	    dophys(i,virtaddr,size,physaddr);
	    if (u.u_error != 0)
		i = -1;
	    break;
	    }
    u.u_error = u_error;
    return i;
}

/*
 *	This routine is called from the keyboard driver in ARAW (ascii raw) mode
 *		dev 	device
 *		cmd 	(ignored)
 *		c	character code (bit 7 on means key up)
 *		next	(ignored)
 *	Basicly we locate the current layer and post the appropriate event. If
 *	a key up occurs, cancel any autokey events on any layers that have
 *	timeouts waiting for them.  Note that keyDown events are posted to the
 *	active layer, but keyUp events are posted to the layer that received
 *	the keyDown event for that key.
 */

static ui_keyboard(dev, cmd, c, next)
int dev, cmd, c, next;
{
    register struct ui_interface *uip;
    register int l;
    register struct layer *lp;

    if ((uip = ui_addr[DEV]) == NULL)
	return;
    if (c&0x80)
	{					/* The key went up */
	c &= 0x7f;
	l = uip->c_key[c];
	if (l != NOLAYER)
	    {
	    uip->c_key[c] = NOLAYER;
	    lp = &ui_layer[DEV][l];
	    if (lp->l_state.state == LS_INUSE && lp->l_down && lp->l_char == c)
		{
		lp->l_down = 0;
		if (lp->l_timeout)
		    {
		    untimeout(ui_catch_timeout, lp);
		    lp->l_timeout = 0;
		    }
		}
	    }
	switch(c)
	    {
	    case 0x36:
		uip->c_modifiers &= ~cntlKey;
		break;
	    case 0x37:
		uip->c_modifiers &= ~cmdKey;
		break;
	    case 0x38:
		uip->c_modifiers &= ~shiftKey;
		break;
	    case 0x39:
		uip->c_modifiers &= ~alphaLock;
		break;
	    case 0x3a:
		uip->c_modifiers &= ~optionKey;
		break;
	    default:
	    	ui_postevent(DEV, l, keyUp, c);
	    }
	}
    else
	{						/* The key went down */
	l = ui_active[DEV];
	uip->c_key[c] = l;
	switch(c)
	    {
	    case 0x36:
		uip->c_modifiers |= cntlKey;
		break;
	    case 0x37:
		uip->c_modifiers |= cmdKey;
		break;
	    case 0x38:
		uip->c_modifiers |= shiftKey;
		break;
	    case 0x39:
		uip->c_modifiers |= alphaLock;
		break;
	    case 0x3a:
		uip->c_modifiers |= optionKey;
		break;
	    default:
		ui_postevent(DEV, l, keyDown, c);
	    }
	}
}

/*
 *	Mouse events are passed in to the appropriate layer. If the layer is
 *	in mouse blocking mode and the mouse has moved wake it.
 *		dev	device
 *		cmd	(ignore)
 *		b	raw mouse data from the device (ignore)
 *		change	bit 0 -> button change
 *			bit 1 -> displacement change
 */

static ui_mouse(dev, cmd, b, change)
int dev, cmd, b, change;
{
    struct ui_interface *uip;
    struct layer *lp;
    register int l;

    if ((uip = ui_addr[DEV]) == NULL)
	return;
    uip->c_button = mouse_button[DEV];
    l = ui_active[DEV];
    if (l == NOLAYER)
	return;
    lp = &ui_layer[DEV][l];
    if ((change&2) && (lp->l_sleep&mouseMoveMask))
	ui_wakeup(lp);
    if (change&1)
	{
	if (uip->c_button)
	    {
	    uip->c_modifiers &= ~btnState;
	    ui_postevent(DEV, l, mouseDown, 0);
	    }
	else
	    {
	    uip->c_modifiers |= btnState;
	    ui_postevent(DEV, l, mouseUp, 0);
	    }
	}
}

/*
 *	Page in and lock down a page in a shared memory section
 *		in a user's address space. Max of 1 page per 'device'
 */

static ui_lockit(dev, base)
int dev;
caddr_t base;
{
    register reg_t *rp;
    register pte_t *pt;
    register struct ui_interface *uip;
    register struct video *vp;
    int x, error;

    if (ui_rp[DEV] || poff(base))
	return EINVAL;
    rp = findreg(u.u_procp, (caddr_t)base);
    if (rp == NULL)
	return EINVAL;
#ifndef NEW_PMMU
    rp->r_flags |= RG_NOSWAP;
#endif
    regrele(rp);
    if (rp->r_type != RT_SHMEM)
	{
	error = EINVAL;
	goto out;
	}
#ifndef lint
    pt = (pte_t *) segvptbl(&u.u_stbl[snum(base)]) + pnum(base);
#else
    pt = (pte_t *)0;
#endif
    x = fubyte((caddr_t)base);
    if(x == -1)
	{
	error = EINVAL;
	goto out;
	}
    if(subyte((caddr_t)base, x) == -1)
	{
	error = EINVAL;
	goto out;
	}
    memlock();
    pg_setlock(pt);

#ifdef NEW_PMMU
    {
    struct pfdat *pf2 = pftopfd(pt->pgm[0].pg_pfn);
    if (! pf2->pf_rawcnt++ && --availrmem < tune.t_minarmem)
	{
	pf2->pf_rawcnt--;
	++availrmem;
	memunlock();
	error = EAGAIN;
	goto out;
	}
    }
#else
    pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt++;
#endif NEW_PMMU

    memunlock();
    reglock(rp);
#ifndef NEW_PMMU
    rp->r_flags &= ~RG_NOSWAP;
#endif
    rp->r_refcnt++;
    regrele(rp);
    ui_rp[DEV] = rp;
    ui_pt[DEV] = pt;
    uip = (struct ui_interface *)ptob(pt->pgm[0].pg_pfn);
    uip->c_button = mouse_button[DEV];
    uip->c_mx = 0;
    uip->c_my = 0;
    uip->c_cx = 0;
    uip->c_cy = 0;
    vp = video_desc[DEV];
    uip->c_ssx = vp->video_scr_x;
    uip->c_ssy = vp->video_scr_y;
    uip->c_smx = vp->video_mem_x;
    uip->c_smy = vp->video_mem_y;
    ui_addr[DEV] = uip;
    return 0;
out:
#ifndef NEW_PMMU
    reglock(rp);
    rp->r_flags &= ~RG_NOSWAP;
    regrele(rp);
#endif
    return error;
}

/*
 *	Unlock the locked down page
 */

static ui_unlockit(dev)
int dev;
{
    register reg_t *rp;
    register pte_t *pt;

    if (ui_rp[DEV] == 0)
	return 0;
    ui_addr[DEV] = (struct ui_interface *)0;
    rp = ui_rp[DEV];

#ifndef NEW_PMMU
    reglock(rp);
    rp->r_flags |= RG_NOSWAP;
    regrele(rp);
#endif NEW_PMMU

    pt = ui_pt[DEV];
    memlock();
#ifdef NEW_PMMU
    {
    struct pfdat *pf2 = pftopfd(pt->pgm[0].pg_pfn);
    if (--pf2->pf_rawcnt == 0)
	{
	pg_clrlock(pt);
	availrmem++;
	}
    }
#else
    if (--pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt == 0)
	pg_clrlock(pt);
#endif
    memunlock();
    reglock(rp);
#ifndef NEW_PMMU
    rp->r_flags &= ~RG_NOSWAP;
#endif
    rp->r_refcnt--;
    if (rp->r_refcnt == 0)		/* if this is the last reference */
	freereg(rp);			/* remove it */
    else
	regrele(rp);
    ui_rp[DEV] = 0;
    return 0;
}

/*
 *	Called from the vertical retrace interrupt
 *	Update the current mouse position
 *	If required redraw the cursor
 *		1)	See if we need to (or are allowed to)
 *		2)	write back the old data
 *		3)	save the new data
 *		4)	write the cursor
 *		5)	update the cursor position
 */

static ui_display(vp)
struct video *vp;
{
    register struct ui_interface *uip;
#if NDEVICES != 1
    register int dev = vp->video_off;
#endif NDEVICES
    register short i, x, y, mx, my;
    register unsigned short *sp;

    if (ui_vtrace[DEV])
	(*ui_vtrace[DEV])(vp);
    if ((uip = ui_addr[DEV]) == NULL)
	return;
    if ((x = mouse_x[DEV] - ui_mx[DEV]) < 0)	/* calc the movement deltas */
	mx = -x;
    else
	mx = x;
    if ((y = mouse_y[DEV] - ui_my[DEV]) < 0)
	my = -y;
    else
	my = y;
    ui_mx[DEV] = mouse_x[DEV];
    ui_my[DEV] = mouse_y[DEV];
    if (mx > my)				/* calc. movement dist. */
	i = mx + (my>>1);
    else
	i = my + (mx>>1);
    if (i >= 256)
	i = 255;
    sp = &uip->c_mlookup[0];			/* <- do mouse acc. */
    if (*sp)
	{
	mx = x;
	my = y;
	while (i > *sp)
	    {
	    x += mx;
	    y += my;
	    sp++;
	    }
	}
    if (x < 0)
	{					/* update the mouse position */
	if ((-x) > uip->c_mx)
	    uip->c_mx = 0;
	else
	    uip->c_mx += x;
	}
    else
	{
	uip->c_mx += x;
	if (uip->c_mx >= uip->c_ssx)
	    uip->c_mx = uip->c_ssx - 1;
	}
    if (y < 0)
	{
	if ((-y) > uip->c_my)
	    uip->c_my = 0;
	else
	    uip->c_my += y;
	}
    else
	{
	uip->c_my += y;
	if (uip->c_my >= uip->c_ssy)
	    uip->c_my = uip->c_ssy - 1;
	}

    /*
    *	decide if the cursor must be redrawn
    */

    if (uip->c_lock || !uip->c_visible)
	return;
    if (uip->c_draw)
	uip->c_draw = 0;
    else if (uip->c_mx == uip->c_cx && uip->c_my == uip->c_cy)
	return;

    /*
    *	draw the cursor
    */

    switch (uip->c_style)
	{
	case CUR_SMALL1:
	    ui_small1(vp, uip);
	    break;
	case CUR_SMALL2:
	    ui_small2(vp, uip);
	    break;
	case CUR_SMALL4:
	    ui_small4(vp, uip);
	    break;
	case CUR_SMALL8:
	    ui_small8(vp, uip);
	    break;
	}

    /*
    *	update the cursor position
    */

    uip->c_cx = uip->c_mx;
    uip->c_cy = uip->c_my;
}

/*
 *	remove the cursor display routine from the VTRACE interrupt
 */

static ui_remcursor(dev)
int dev;
{
    register int s;
    register struct video *vp;

    ui_cursor[DEV] = 0;
    vp = video_desc[DEV];
    s = spl1();
    vp->video_intr = ui_vtrace[DEV];
    if (vp->video_intr == NULL)
	(*vp->video_func)(vp, VF_DISABLE, 0);
    splx(s);
}

/*
 *	restore owership of the mouse/keyboard to their previous users
 *		(or close them if they were previously free)
 */

static ui_remdevices(dev)
int dev;
{
    register int s;

    ui_devices[DEV] = 0;
    s = spl1();
    if (!ui_mouse_open[DEV])
	mouse_close(DEV);
    else
	{
	mouse_op(DEV, MOUSE_OP_MODE, ui_mouse_mode[DEV]);
	mouse_op(DEV, MOUSE_OP_INTR, ui_mouse_intr[DEV]);
	}
    if (!ui_key_open[DEV])
	key_close(DEV);
    else
	{
	key_op(DEV, KEY_OP_MODE, ui_key_mode[DEV]);
	key_op(DEV, KEY_OP_INTR, ui_key_intr[DEV]);
	}
    splx(s);
}

/*
    This routine gets called via timeout when it is time to deliver an
auto-key event to a sleeping process.
*/

static ui_catch_timeout(lp)
struct layer *lp;
{
    lp->l_timeout = 0;
    ui_wakeup(lp);
}

/*
 *	If a process is sleeping waiting for an event in lp, this
 *		routine wakes it up.  It may be called via ui_catch_timeout
 *		for an auto-key event.
 */

static ui_wakeup(lp)
struct layer *lp;
{
    register struct proc *p;

    if (lp->l_sleep != 0)
	{
	lp->l_sleep = 0;
	if ((p = lp->l_select) == 0)
	    wakeup(lp);
	else
	    {
	    lp->l_select = 0;
	    selwakeup(p,0);
	    }
	}
}

/*
 *	Handle our part of a select system call.  Return 1 if there are
 *		events in this proc's queue.  Otherwise, arrange to have
 *		selwakeup called if events arrive and return 0.
 */

ui_select(dev,rw)
dev_t dev;
int rw;
{
    int i;
    EventRecord er;
    struct layer *lp;

#if NDEVICES != 1
    dev = minor(dev);
#endif NDEVICES
    if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
	return 0;
    ui_getevent(everyEvent,AVAIL,&er);
    if (er.what != nullEvent)
	return 1;
    lp = &ui_layer[DEV][i];
    if (lp->l_timeout)
	{
	untimeout(ui_catch_timeout,lp);
	lp->l_timeout = 0;
	}
    lp->l_sleep = lp->l_mask;
    lp->l_select = u.u_procp;
    if (lp->l_down && (lp->l_mask&autoKeyMask))
	{
	lp->l_timeout = 1;
	timeout(ui_catch_timeout, lp, lp->l_time - lbolt);
	}
    return 0;
}

