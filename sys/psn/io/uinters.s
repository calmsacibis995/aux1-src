	#	@(#)Copyright Apple Computer 1987	Version 1.3 of uinters.s on 87/11/11 21:39:12
/*	@(#)uinters.s	UniPlus VVV.2.1.3	*/
/*
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */
/*
 *	Warning!!! ----> this only runs on 68020s ......
 */

#define	vp	8(%fp)
#define	uip	12(%fp)

#define	video_addr	0x00
#define	video_mem_x	0x04
#define	video_mem_y	0x08
#define	video_scr_x	0x0c
#define	video_scr_y	0x10

#define	c_mx		0x0000
#define	c_my		0x0004
#define	c_cx		0x0008
#define	c_cy		0x000c
#define	c_smx		0x0010
#define	c_smy		0x0014
#define	c_ssx		0x0018
#define	c_ssy		0x001c
#define	c_hpx		0x0020
#define	c_hpy		0x0024
#define	c_cursor	0x0028
#define	c_data		0x0128
#define	c_mask		0x0228
#define	c_saved		0x0248

	global ui_small1
ui_small1:
	link	%fp, &0
	movm.l	&0x3ff8, -(%sp)
	mov.l	vp, %a0			/* video structure pointer */
	mov.l	uip, %a1		/* user interface structure pointer */
	mov.l	video_addr(%a0), %a2	/* screen address */
	mov.l	video_mem_x(%a0), %d0	/* screen row increment */
	tst.b	c_saved(%a1)		/* only restore if saved */
	beq	skip11%

	/*
    	 *	First restore the old data
	 */

	lea.l	c_data(%a1), %a3
	mov.l	c_cy(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_cx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop11%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next11%		/* 	screen skip on past */
		mov.w	(%a3)+, %d1	/* Now move the data */
		bfins	%d1, (%a2){%d2:&16} 
next11%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop11%

	/*
    	 *	First save the new data
	 */

skip11%:
	st	c_saved(%a1)
	lea.l	c_data(%a1), %a3
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop12%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next12%		/* 	screen skip on past */
		bfextu	(%a2){%d2:&16} ,%d1
		mov.w	%d1, (%a3)+	/* Now move the data */
next12%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop12%

	/*
	 *	Now move the cursor
	 */

	lea.l	c_cursor(%a1), %a3
	lea.l	c_mask(%a1), %a4
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	add.l	%d1, %d2		/* now we have the pixel address */
	clr.l	%d1
	clr.l	%d3
	mov.l	&15, %d4		/* initialise loop counter */
loop13%:
		mov.w	(%a3)+, %d1	/* Get the data */
		mov.w	(%a4)+, %d3	/* and now the mask */
		beq	next13%		/* if zero draw nothing */
		mov.l	%d2, %d5	/* get the pixel pointer */
		blt	next13%		/* if before the beginning of the */
					/*    screen skip */
loop14%:
			rol.w	&1, %d1	/* put the msb of the data in bit 0 */
			lsl.w	&1, %d3 /* test the mask msb */
			bcc	next14%	/* if set move the data msg */
			bfins	%d1, (%a2){%d5:&1} 

next14%:		add.l	&1, %d5	/* increment the pixel pointer */
			tst.w	%d3	/* are we finished (mask empty?) */
			bne	loop14%
next13%:
		add.l	%d0, %d2	/* increment the row address */
	dbra	%d4, loop13%

	movm.l	(%sp)+, &0x1ffc
	unlk	%fp
	rts

	global ui_small2
ui_small2:
	link	%fp, &0
	movm.l	&0x3ff8, -(%sp)
	mov.l	vp, %a0			/* video structure pointer */
	mov.l	uip, %a1		/* user interface structure pointer */
	mov.l	video_addr(%a0), %a2	/* screen address */
	mov.l	video_mem_x(%a0), %d0	/* screen row increment */
	lsl	&1, %d0			/* make it a bit increment */
	tst.b	c_saved(%a1)		/* only restore if saved */
	beq	skip21%

	/*
    	 *	First restore the old data
	 */

	lea.l	c_data(%a1), %a3
	mov.l	c_cy(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_cx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	lsl	&1, %d2			/* make it a bit increment */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop21%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next21%		/* 	screen skip on past */
		mov.l	(%a3)+, %d1	/* Now move the data */
		bfins	%d1, (%a2){%d2:&32} 
next21%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop21%

	/*
    	 *	First save the new data
	 */

skip21%:
	st	c_saved(%a1)
	lea.l	c_data(%a1), %a3
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	lsl	&1, %d2			/* make it a bit increment */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop22%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next22%		/* 	screen skip on past */
		bfextu	(%a2){%d2:&32} ,%d1
		mov.l	%d1, (%a3)+	/* Now move the data */
next22%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop22%

	/*
	 *	Now move the cursor
	 */

	lea.l	c_cursor(%a1), %a3
	lea.l	c_mask(%a1), %a4
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	lsl	&1, %d2			/* make it a bit increment */
	add.l	%d1, %d2		/* now we have the pixel address */
	clr.l	%d3
	mov.l	&15, %d4		/* initialise loop counter */
loop23%:
		mov.l	(%a3)+, %d1	/* Get the data */
		mov.w	(%a4)+, %d3	/* and now the mask */
		beq	next23%		/* if zero draw nothing */
		mov.l	%d2, %d5	/* get the pixel pointer */
		blt	next23%		/* if before the beginning of the */
					/*    screen skip */
loop24%:
			rol.l	&2, %d1	/* put the msb of the data in bit 0 */
			lsl.w	&1, %d3 /* test the mask msb */
			bcc	next24%	/* if set move the data msg */
			bfins	%d1, (%a2){%d5:&2} 

next24%:		add.l	&2, %d5	/* increment the pixel pointer */
			tst.w	%d3	/* are we finished (mask empty?) */
			bne	loop24%
next23%:
		add.l	%d0, %d2	/* increment the row address */
	dbra	%d4, loop23%

	movm.l	(%sp)+, &0x1ffc
	unlk	%fp
	rts

	global ui_small4
ui_small4:
	link	%fp, &0
	movm.l	&0x3ff8, -(%sp)
	mov.l	vp, %a0			/* video structure pointer */
	mov.l	uip, %a1		/* user interface structure pointer */
	mov.l	video_addr(%a0), %a2	/* screen address */
	mov.l	video_mem_x(%a0), %d0	/* screen row increment */
	lsl	&2, %d0			/* make it a bit increment */
	tst.b	c_saved(%a1)		/* only restore if saved */
	beq	skip41%

	/*
    	 *	First restore the old data
	 */

	lea.l	c_data(%a1), %a3
	mov.l	c_cy(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_cx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	lsl	&2, %d2			/* make it a bit increment */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop41%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next41%		/* 	screen skip on past */
		mov.l	(%a3)+, %d1	/* Now move the data */
		bfins	%d1, (%a2){%d2:&32} 
		mov.l	(%a3)+, %d1	/* Now move the data */
		bfins	%d1, 4(%a2){%d2:&32} 
next41%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop41%

	/*
    	 *	First save the new data
	 */

skip41%:
	st	c_saved(%a1)
	lea.l	c_data(%a1), %a3
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	lsl	&2, %d2			/* make it a bit increment */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop42%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next42%		/* 	screen skip on past */
		bfextu	(%a2){%d2:&32} ,%d1
		mov.l	%d1, (%a3)+	/* Now move the data */
		bfextu	4(%a2){%d2:&32} ,%d1
		mov.l	%d1, (%a3)+	/* Now move the data */
next42%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop42%

	/*
	 *	Now move the cursor
	 */

	lea.l	c_cursor(%a1), %a3
	lea.l	c_mask(%a1), %a4
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	lsl	&2, %d2			/* make it a bit increment */
	add.l	%d1, %d2		/* now we have the pixel address */
	clr.l	%d3
	mov.l	&15, %d4		/* initialise loop counter */
loop43%:
		mov.l	(%a3)+, %d1	/* Get the data */
		mov.l	&8, %d6
		mov.l	&1, %d7
		mov.w	(%a4)+, %d3	/* and now the mask */
		beq	next43%		/* if zero draw nothing */
		mov.l	%d2, %d5	/* get the pixel pointer */
		blt	next43%		/* if before the beginning of the */
					/*    screen skip */
loop44%:
			sub.l	&1, %d6	/* get the next word */
			bne	next47%
			mov.l	&8, %d6
			clr.l	%d7
			mov.l	(%a3)+, %d1
next47%:		rol.l	&4, %d1	/* put the msb of the data in bit 0 */
			lsl.w	&1, %d3 /* test the mask msb */
			bcc	next44%	/* if set move the data msg */
			bfins	%d1, (%a2){%d5:&4} 

next44%:		add.l	&4, %d5	/* increment the pixel pointer */
			tst.w	%d3	/* are we finished (mask empty?) */
			bne	loop44%
next43%:	tst.l	%d7
		beq	next48%		/* increment the remainder */
		add.l	&4, %a3
next48%:	add.l	%d0, %d2	/* increment the row address */
	dbra	%d4, loop43%

	movm.l	(%sp)+, &0x1ffc
	unlk	%fp
	rts

	global ui_small8
ui_small8:
	link	%fp, &0
	movm.l	&0x3ff8, -(%sp)
	mov.l	vp, %a0			/* video structure pointer */
	mov.l	uip, %a1		/* user interface structure pointer */
	mov.l	video_addr(%a0), %a2	/* screen address */
	mov.l	video_mem_x(%a0), %d0	/* screen row increment */
	tst.b	c_saved(%a1)		/* only restore if saved */
	beq	skip81%

	/*
    	 *	First restore the old data
	 */

	lea.l	c_data(%a1), %a3
	mov.l	c_cy(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_cx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop81%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next81%		/* 	screen skip on past */
		mov.l	(%a3)+, (0,  %a2, %d2.l*1)	/* Now move the data */
		mov.l	(%a3)+, (4,  %a2, %d2.l*1)
		mov.l	(%a3)+, (8,  %a2, %d2.l*1)
		mov.l	(%a3)+, (12, %a2, %d2.l*1)
next81%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop81%

	/*
    	 *	First save the new data
	 */

skip81%:
	st	c_saved(%a1)
	lea.l	c_data(%a1), %a3
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	add.l	%d1, %d2		/* now we have the pixel address */
	mov.l	&15, %d4		/* initialise loop counter */
loop82%:
		tst.l	%d2		/* if before the beginning of the */
		blt	next82%		/* 	screen skip on past */
		mov.l	(0,  %a2, %d2.l*1), (%a3)+	/* Now move the data */
		mov.l	(4,  %a2, %d2.l*1), (%a3)+
		mov.l	(8,  %a2, %d2.l*1), (%a3)+
		mov.l	(12, %a2, %d2.l*1), (%a3)+
next82%:	add.l	%d0, %d2	/* increment the screen address */
	dbra	%d4, loop82%

	/*
	 *	Now move the cursor
	 */

	lea.l	c_cursor(%a1), %a3
	lea.l	c_mask(%a1), %a4
	mov.l	c_my(%a1), %d1		/* Get the screen row in %d1 */
	sub.l	c_hpy(%a1), %d1		/* And get the hot point offset */
	mulu.l	%d0, %d1		/* Now get the row pixel address */
	mov.l	c_mx(%a1), %d2		/* now get the screen col */
	sub.l	c_hpx(%a1), %d2		/* And get the hot point offset */
	add.l	%d1, %d2		/* now we have the pixel address */
	clr.l	%d1
	clr.l	%d3
	mov.l	&15, %d4		/* initialise loop counter */
loop83%:
		mov.l	&15, %d6	/* initialise loop counter */
		mov.w	(%a4)+, %d3	/* and now the mask */
		beq	next83%
		mov.l	%d2, %d5	/* get the pixel pointer */
		blt	next83%		/* if before the beginning of the */
					/*    screen skip */
loop84%:
			lsl.w	&1, %d3 /* test the mask msb */
			bcc	next84%	/* if set move the data msg */
			mov.b	(%a3)+, (0, %a2, %d5.l*1)
			br	next85%
next84%:		add.l	&1, %a3
next85%:		add.l	&1, %d5	/* increment the pixel pointer */
			dbra	%d6, loop84%
		bra	next86%
next83%:	add.l	&16, %a3
next86%:	add.l	%d0, %d2	/* increment the row address */
	dbra	%d4, loop83%

	movm.l	(%sp)+, &0x1ffc
	unlk	%fp
	rts
