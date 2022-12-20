#ifndef lint	/* .../appleprint/iw/daiw/do_D.c */
#define _AC_NAME do_D_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:33}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_D_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_D.c on 87/11/11 21:43:33";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"
#include <math.h>

static void draw_line ();
static void draw_circle ();
static void draw_ellipse ();
static void draw_arc ();
static void draw_wig ();

void
do_D (ifile)
    FILE   *ifile;
{
    char	buf[256];
    int		x, y, x1, y1;

    eatline (ifile, buf);
    switch (buf[0]) {
    case 'l': 	/* draw a line */
	sscanf (buf + 1, "%d %d", &x, &y);
	draw_line (x, y);
	break;
    case 'c': 	/* circle */
	sscanf (buf + 1, "%d", &x);
	draw_circle (x);
	break;
    case 'e': 	/* ellipse */
	sscanf (buf + 1, "%d %d", &y, &x);
	draw_ellipse (y, x);
	break;
    case 'a': 	/* arc */
	sscanf (buf + 1, "%d %d %d %d", &x, &y, &x1, &y1);
	draw_arc (x, y, x1, y1);
	break;
    case '~': 	/* wiggly line */
	draw_wig (buf + 1);
	break;
    default:
	badinput (buf[0], "do_D: unknown drawing function", ifile);
	break;
    }
}

static void
draw_line (x, y)
{
    PenNormal ();
    MoveTo ((short)pos_horz, (short)pos_vert);
    tpos_horz += x;
    pos_horz = tpos_horz / dev_scale;
    tpos_vert += y;
    pos_vert = tpos_vert / dev_scale;
    LineTo (pos_horz, pos_vert);
}

static void
draw_circle (d)
{
    Rect	the_rect;
    int		scale_d = d / dev_scale;

    SetRect (&the_rect, pos_horz, pos_vert - scale_d / 2,
	     pos_horz + scale_d, pos_vert + scale_d / 2);
    FrameOval (&the_rect);
    tpos_horz += d;
    pos_horz = tpos_horz / dev_scale;
}

static void
draw_ellipse (dx, dy)
{
    Rect	the_rect;
    int		scale_dx = dx / dev_scale;
    int		scale_dy = dy / dev_scale;

    SetRect (&the_rect, pos_horz, pos_vert - scale_dy / 2,
	     pos_horz + scale_dx, pos_vert + scale_dy / 2);
    FrameOval (&the_rect);
    tpos_horz += dx;
    pos_horz = tpos_horz / dev_scale;
}

static void
draw_arc (x, y, u, v)
{
    Rect	the_rect;
    short	start_angle;
    short	end_angle;
    short	arc_angle;
    Point	the_point;
    short	x0, y0;		/* centers coordinates */
    short	x1, y1;		/* ending point of arc's coordinates */
    short	r;		/* the arcs radius */
    int		scale_x = x / dev_scale;
    int		scale_y = y / dev_scale;
    int	        scale_u = u / dev_scale;
    int		scale_v = v / dev_scale;

    x0 = pos_horz + scale_x;
    y0 = pos_vert + scale_y;
    x1 = x0 + scale_u;
    y1 = y0 + scale_v;
    r = (short)(sqrt ((double)(scale_x * scale_x)
		      + (double)(scale_y * scale_y)) + 0.5);
    SetRect (&the_rect, x0 - r, y0 - r, x0 + r, y0 + r);
    SetPt (&the_point, pos_horz, pos_vert);
    PtToAngle (&the_rect, &the_point, &start_angle);
    SetPt (&the_point, x1, y1);
    PtToAngle (&the_rect, &the_point, &end_angle);
    arc_angle = (start_angle - end_angle);
    if (arc_angle < 0) arc_angle += 360;
    arc_angle *= -1;
    FrameArc (&the_rect, start_angle, arc_angle);
    tpos_horz += x + u;
    pos_horz = tpos_horz / dev_scale;
    tpos_vert += y + v;
    pos_vert = tpos_vert / dev_scale;
    
}

static void
draw_wig (buf)
    char	*buf;
{
    int		 argc;
    char	*argv[50];
    int		 x[50], y[50];
    int		 n, i, j;
    float	 t1, t2, t3, w, w2;
    int		 x_pt, y_pt;
    int		 prev_x_pt, prev_y_pt;
    int		 ndots;

    argc = line2av (buf, argv);
    n = 2;
    for (i = 0; i < argc; i += 2) {
	x[n] = atoi (argv[i]);
	y[n] = atoi (argv[i+1]);
	n++;
    }
    x[0] = x[1] = tpos_horz;
    y[0] = y[1] = tpos_vert;
    for (i = 1; i < n; i++) {
	x[i+1] += x[i];
	y[i+1] += y[i];
    }
    x[n] = x[n - 1];
    y[n] = y[n - 1];
    prev_x_pt = prev_y_pt = -9999;
    for (i = 0; i < n - 1; i++) {
	ndots = (dist (x[i], y[i], x[i+1], y[i+1]) +
		 dist (x[i+1], y[i+1], x[i+2], y[i+2])) / 2;
	for (j = 0; j < ndots; j++) {
	    w = (float) j / ndots;
	    t1 = 0.5 * w * w;
	    w = w - 0.5;
	    t2 = 0.75 - w * w;
	    w = w - 0.5;
	    t3 = 0.5 * w * w;
	    x_pt = t1 * x[i+2] + t2 * x[i+1] + t3 * x[i] + 0.5;
	    y_pt = t1 * y[i+2] + t2 * y[i+1] + t3 * y[i] + 0.5;
	    if (x_pt != prev_x_pt || y_pt != prev_y_pt) {
		draw_line (x_pt - pos_horz, y_pt - pos_vert);
		prev_x_pt = x_pt;
		prev_y_pt = y_pt;
	    }
	}
    }
}

dist (x1, y1, x2, y2)
{
    float	dx, dy;

    dx = x2 - x1;
    dy = y2 - y1;
    return sqrt (dx * dx + dy * dy) + 0.5;
}
