#ifndef lint	/* .../appleprint/iw/daiw/iw2.c */
#define _AC_NAME iw2_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:34}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _iw2_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of iw2.c on 87/11/11 21:44:34";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

static int	 count_bands ();
static void	 print_band ();
static void	 skip_lines ();
static int	 band_size ();
static int	 is_empty_band ();
static void	 pr_band ();
static void	 pr_1band ();
static void	 bld_band ();
static BitMap   *new_bitmap ();
static void	 clr_bitmap ();

#define MAX_BAND_DATA	2048

struct dev_band {
    int		 	 dbnd_first;
    int		 	 dbnd_last;
    unsigned char	 dbnd_data[MAX_BAND_DATA];
};

void
open_device ()
{
    int		res;
    int		paperlength;
    int		paperwidth;

    res = dev.res / dev_scale;
    paperlength = dev.paperlength / dev_scale;
    paperwidth = dev.paperwidth / dev_scale;
    if ((iw2 = open_printer ("iw2", res, paperlength, paperwidth)) == NULL)
	harderr (209, "Can't open printer");
    if ((gp = open_prport (iw2)) == NULL)
	harderr (210, "Can't create printer port");
    SetPort (gp);
    clear_page (iw2, gp);
    init_printer (iw2);
}

/*
 * open_printer - builds a device description record
 */

struct iw2 *
open_printer (name, res, bit_height, bit_width)
    char	*name;
    short	 res;
    short	 bit_height;
    short	 bit_width;
{
    struct iw2	*dev;

    if ((dev = NEW (struct iw2)) != NULL) {
	dev->d_name = strsave (name);
	dev->d_fp = stdout;
	dev->d_res = res;
	dev->d_vert = bit_height;
	dev->d_hor = bit_width;
    }
    return dev;
}

void
init_printer (dev)
    struct iw2	*dev;
{
    switch (dev->d_res) {
    case IW2_HIGH_RES:
	fputs ("\033L000", dev->d_fp);  /* left margin to 0 */
	fputs ("\033<", dev->d_fp);	/* bidirectional */
	fputs ("\033p", dev->d_fp);	/* 144 dpi */
	fputs ("\033T01", dev->d_fp);	/* line feed set to 01/144 */
	break;
    case IW2_FASTER_RES:
    default:
	fputs ("\033L000", dev->d_fp);  /* left margin to 0 */
	fputs ("\033<", dev->d_fp);	/* bidirectional */
	fputs ("\033n", dev->d_fp);	/* 72 dpi = 'n' 80 = 'N'*/
	fputs ("\033T01", dev->d_fp);	/* line feed set to 01/144 */
	break;
    }
}


GrafPtr
open_prport (dev)
    struct iw2  *dev;
{
    BitMap	*bm;
    GrafPort	*gp;
    short	 row_bytes;
    Region	*cr;

#ifdef notdef
    fprintf (stderr, "printer_port (%d, %d, %d)\n", dev->d_res, dev->d_vert, dev->d_hor);
#endif
    if ((bm = new_bitmap (dev->d_vert, dev->d_hor)) == NULL)
	return NULL;
    if ((gp = NEW(GrafPort)) == NULL)
	return NULL;
    OpenPort (gp);
    SetPortBits (bm);
    PortSize (dev->d_hor, dev->d_vert);
#ifdef notdef
    fprintf (stderr, "bm bounds top %d bottom %d left %d right %d\n",
	     gp->portBits.bounds.top,
	     gp->portBits.bounds.bottom,
	     gp->portBits.bounds.left,
	     gp->portBits.bounds.right);
    cr = *gp->clipRgn;
    fprintf (stderr, "printer_port before clip top %d bottom %d left %d right %d\n",
	     cr->rgnBBox.top,
	     cr->rgnBBox.bottom,
	     cr->rgnBBox.left,
	     cr->rgnBBox.right);
#endif
    ClipRect (&bm->bounds);
#ifdef notdef
    fprintf (stderr, "printer_port top %d bottom %d left %d right %d\n",
	     gp->portRect.top, gp->portRect.bottom, gp->portRect.left,
	     gp->portRect.right);
    fprintf (stderr, "printer_port clip top %d bottom %d left %d right %d\n",
	     (*(gp->clipRgn))->rgnBBox.top,
	     (*(gp->clipRgn))->rgnBBox.bottom,
	     (*(gp->clipRgn))->rgnBBox.left,
	     (*(gp->clipRgn))->rgnBBox.right);
    fprintf (stderr, "printer_port vis top %d bottom %d left %d right %d\n",
	     (*(gp->visRgn))->rgnBBox.top,
	     (*(gp->visRgn))->rgnBBox.bottom,
	     (*(gp->visRgn))->rgnBBox.left,
	     (*(gp->visRgn))->rgnBBox.right);
#endif

    (*(gp->visRgn))->rgnBBox.top = gp->portRect.top;
    (*(gp->visRgn))->rgnBBox.bottom = gp->portRect.bottom;
    (*(gp->visRgn))->rgnBBox.left = gp->portRect.left;
    (*(gp->visRgn))->rgnBBox.right = gp->portRect.right;
#ifdef notdef
    fprintf (stderr, "printer_port vis top %d bottom %d left %d right %d\n",
	     (*(gp->visRgn))->rgnBBox.top,
	     (*(gp->visRgn))->rgnBBox.bottom,
	     (*(gp->visRgn))->rgnBBox.left,
	     (*(gp->visRgn))->rgnBBox.right);
#endif
    return gp;
}

void
clear_page (dev, gp)
    struct iw2		*dev;
    GrafPtr		 gp;
{
    clr_bitmap (&gp->portBits);
}


void
print_page (dev, gp)
    struct iw2		*dev;
    GrafPtr		 gp;
{
    int			 total_bands;
    int			 band;
    int			 bandsize;
    int			 lines_to_skip;

    total_bands = count_bands (dev, gp);
#ifdef notdef
    fprintf (stderr, "print_page:total bands = %d\n", total_bands);
#endif
    bandsize = band_size (dev);
    lines_to_skip = 0;
    for (band = 0; band < total_bands; band++) {
	if (is_empty_band (gp, band, bandsize)) {
	    lines_to_skip += bandsize;
#ifdef notdef
	    fprintf (stderr, "skip = %d band %d is empty\n", lines_to_skip, band);
#endif
	} else {
	    if (lines_to_skip > 0) {
		skip_lines (dev, lines_to_skip);
		lines_to_skip = 0;
	    }
#ifdef notdef
	    fprintf (stderr, "print_band %d skipping %d\n", band, lines_to_skip);
#endif
	    print_band (dev, gp, band, bandsize);
	}
    }
    if (lines_to_skip > 0)
	skip_lines (dev, lines_to_skip);
}

static int
count_bands (dev, port)
    struct iw2		*dev;
    GrafPtr		 port;
{
    int			 band_height;
    int			 page_size;

    band_height = band_size (dev);
    page_size = port->portRect.bottom - port->portRect.top;
    return (page_size + band_height - 1) / band_height;
}

static void
print_band (dev, gp, band, bandsize)
    struct iw2		*dev;
    GrafPtr		 gp;
{
    struct dev_band	 band1;
    struct dev_band	 band2;
    char		*data;
    int			 length;
    int			 row_bytes;

    length = bandsize * gp->portBits.rowBytes;
    data = &gp->portBits.baseAddr[length * band];
    row_bytes = gp->portBits.rowBytes;
    bld_band (dev, data, bandsize, row_bytes, &band1, &band2);
    pr_band (dev, &band1, &band2);
}

static void
bld_band (dev, data, bandsize, row_bytes, band1, band2)
    struct iw2		*dev;
    unsigned char	*data;
    struct dev_band	*band1;
    struct dev_band	*band2;
{
    int 		 	 byte_ix;
    int			 	 bit_ix;
    register unsigned	 	 work1;
    register unsigned		 work2;
    register int	 	 i;
    register int	 	 byte;
    register unsigned		 bit_mask;
    register unsigned char	*p;

    band1->dbnd_first =	dev->d_hor;
    band1->dbnd_last  = -1;
    band2->dbnd_first = dev->d_hor;
    band2->dbnd_last  = -1;

    if (IS_HIGH_RES(dev)) {
	for (bit_ix = 0; bit_ix < dev->d_hor; bit_ix++) {
	    work1 = 0;
	    work2 = 0;
	    byte_ix = bit_ix >> 3;
	    byte_ix += (bandsize - 1) * row_bytes;
	    bit_mask = (1 << (7 - (bit_ix & 07)));
	    p = &data[byte_ix];
/* fprintf (stderr, "bitix = %d,byteix = %d,mask=%x,*p = ", bit_ix, byte_ix,bit_mask);	    */
	    for (i = 0; i < bandsize; i++) {
		byte = *p;
/*fprintf (stderr, "%x(%x),", *p,work);		*/
		if (is_odd (i)) {
		    work2 <<= 1;
		    if (byte & bit_mask)
			work2 |= 1;
		} else {
		    work1 <<= 1;
		    if (byte & bit_mask)
			work1 |= 1;
		}
		p -= row_bytes;
	    }
	    if (work1 != 0) {
		band1->dbnd_first = min(band1->dbnd_first, bit_ix);
		band1->dbnd_last  = max(band1->dbnd_last, bit_ix);
	    }
	    if (work2 != 0) {
		band2->dbnd_first = min(band2->dbnd_first, bit_ix);
		band2->dbnd_last  = max(band2->dbnd_last, bit_ix);
	    }
	    band1->dbnd_data[bit_ix] = (unsigned char)work1;
	    band2->dbnd_data[bit_ix] = (unsigned char)work2;
/*fprintf (stderr, "\n");	    */
	}
    } else {
	for (bit_ix = 0; bit_ix < dev->d_hor; bit_ix++) {
	    work1 = 0;
	    byte_ix = bit_ix >> 3;
	    byte_ix += (bandsize - 1) * row_bytes;
	    bit_mask = (1 << (7 - (bit_ix & 07)));
	    p = &data[byte_ix];
/* fprintf (stderr, "bitix = %d,byteix = %d,mask=%x,*p = ", bit_ix, byte_ix,bit_mask);	    */
	    for (i = 0; i < bandsize; i++) {
		byte = *p;
/*fprintf (stderr, "%x(%x),", *p,work1);		*/
		work1 <<= 1;
		if (byte & bit_mask)
		    work1 |= 1;
		p -= row_bytes;
	    }
	    if (work1 != 0) {
		band1->dbnd_first = min(band1->dbnd_first, bit_ix);
		band1->dbnd_last  = max(band1->dbnd_last, bit_ix);
	    }
	    band1->dbnd_data[bit_ix] = (unsigned char)work1;
/*fprintf (stderr, "\n");	    */
	}
    }
#ifdef notdef
	for (bit_ix = 0; bit_ix < dev->d_hor; bit_ix++) {
	    fprintf (stderr, "%3x", band1->dbnd_data[bit_ix]);
	    if ((bit_ix + 1) % 25 == 0)
		fprintf (stderr, "\n");
	}
	fprintf (stderr, "\n");
#endif
}

static void
pr_band (dev, band1, band2)
    struct iw2		*dev;
    struct dev_band	*band1;
    struct dev_band	*band2;
{
    int			 i, j;
    int			 line_length;

    if (IS_HIGH_RES (dev)) {
	pr_1band (dev, 1, band2);
	pr_1band (dev, 15, band1);
    } else {
	pr_1band (dev, 16, band1);
    }
}

static void
pr_1band (dev, skew, band)
    struct iw2		*dev;
    struct dev_band	*band;
{
    int			 i, j;
    int			 line_length;

    if (band->dbnd_last == -1) {
	fprintf (dev->d_fp, "\033T%02d", skew); /* sets vertical res */
	fprintf (dev->d_fp, "\n");
    } else {
#ifdef notdef
	fprintf (stderr, "horizontal move to %d\n", band->dbnd_first);
#endif
	fprintf (dev->d_fp, "\033F%04d", band->dbnd_first); /* horiz move */
	fprintf (dev->d_fp, "\033T%02d", skew);  /* sets vertical res */
	j = 0;
	
	line_length = band->dbnd_last - band->dbnd_first + 1;
	if ((line_length % 8) == 0)
	    fprintf (dev->d_fp, "\033g%03d", line_length / 8);
	else
	    fprintf (dev->d_fp, "\033G%04d", line_length);
	
	for (i = band->dbnd_first; i <= band->dbnd_last; i++) {
	    putc (band->dbnd_data[i], dev->d_fp);
#ifdef notdef
	    fprintf (stderr, "%3x", band->dbnd_data[i]);
	    if ((++j % 25) == 0)
		fprintf (stderr, "\n");
#endif
	}
#ifdef notdef
	fprintf (stderr, "\n");	/* REMOVE */
#endif
	fprintf (dev->d_fp, "\n");
	
    }
}

static void
skip_lines (dev, nlines)
    struct iw2		*dev;
{
    int			 lines;
    int			 inc;
    int			 oinc;
    
    if (IS_HIGH_RES (dev))
	lines = nlines;
    else
	lines = nlines * 2;
    oinc = -1;
    while (lines > 0) {
	inc = min (lines, 99);
	if (inc != oinc) {
	    fprintf (dev->d_fp, "\033T%02d", inc);
	    oinc = inc;
	}
	fprintf (dev->d_fp, "\n");
	lines -= inc;
    }
}

static int
band_size (dev)
    struct iw2	*dev;
{
    int		 band_size;

    if (IS_HIGH_RES (dev))
	band_size = 16;
    else
	band_size = 8;
    return band_size;
}


static int
is_empty_band (gp, band, bandsize)
    GrafPtr		gp;
{
    int			 offset;
    int			 bytes_in_band;
    register char	*p;
    register char	*start;
    register char	*end;

    bytes_in_band = bandsize * gp->portBits.rowBytes;
    start = &gp->portBits.baseAddr[bytes_in_band * band];
    end = start + bytes_in_band;
    for (p = start; p < end; p++)
	if (*p)
	    return 0;
    return 1;
}


static BitMap *
new_bitmap (bit_height, bit_width)
    short	 bit_height;
    short	 bit_width;
{
    BitMap	*bm;
    short	 row_bytes;

    if ((bm = NEW(BitMap)) == NULL)
	return NULL;
    row_bytes = (bit_width + 7) / 8; /* round up */
    if (is_odd (row_bytes))
	row_bytes++; /* must be even */
    if ((bm->baseAddr = malloc (bit_height * row_bytes)) == NULL)
	return NULL;
    bm->rowBytes = row_bytes;
    SetRect (&bm->bounds, 0, 0, bit_width, bit_height);
    return bm;
}

static void
clr_bitmap (bm)
    BitMap	*bm;
{
    register char	*p;
    register int	 size;

    if (bm != NULL) {
	size = (bm->bounds.bottom - bm->bounds.top) * bm->rowBytes;
	p = bm->baseAddr;
	while (size-- > 0)
	    *p++ = 0;
    }
}

