#ifndef lint	/* .../sys/psn/io/key.c */
#define _AC_NAME key_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.4 87/11/19 18:02:00}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of key.c on 87/11/19 18:02:00";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)key.c	UniPlus VVV.2.1.12	*/
/*
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include <sys/param.h>
#include <sys/fdb.h>
#include <sys/key.h>
#include <sys/debug.h>


/*
 *	This is the max number of keyboards supported. It must remain 1 until
 *	someone solves the problem of how to associate a particular keyboard
 *	with a particular screen.
 */

#define NDEVICES	1


extern int nulldev();

typedef int (*procedure_t)();

short key_r0[NDEVICES], key_r1[NDEVICES];	/* keyboard register shadows */
static unsigned short key_buff[NDEVICES];	/* buffer for fdb IO */
static char 	*key_save;			/* pointer to asci strings */
static char 	key_buffer[NDEVICES*8];		/* last key sequence */
static int 	key_length[NDEVICES];		/* it's length */
static procedure_t 	key_call[NDEVICES];	/* higher level interrupt
						   service routine */
static char 	key_last[NDEVICES];		/* the last keycode received */
static char 	key_down[NDEVICES];		/* there is currently a key down
						   (and a candidate for
						   repeating) */
static unsigned short key_control[NDEVICES];	/* The current state of the
						   keyboard modifier keys
						   (shift, apple etc) */
static char 	key_defwait[NDEVICES];		/* How long to wait before
						   repeating */
static char 	key_defgap[NDEVICES];		/* How long between repeated
						   characters */
static int	key_mode[NDEVICES];		/* the keyboard opoerating mode
						   (ascii/raw) */
static int	key_state[NDEVICES];		/* the device state, used for
						   the lower level fdb FSM */
static int	key_opened[NDEVICES];		/* the keyboard is open */
static int	key_keypad[NDEVICES];		/* the keyboard is in keypad
						   mode */

long key_op();
int key_open();
int key_close();

struct key_data key_data = {
	key_open,
	key_close,
	key_op,
};

static int key_intr();
static int key_send();
static int key_repeat();
static int key_ascii();
static int key_timeout();

/*
 *	Values for key_state
 */

#define	STATE_INIT	0	/* non active state */
#define	STATE_REG0	1	/* idle state waiting for poll/intr */
#define	STATE_REG2	2	/* reading register 2 */
#define	STATE_OPEN	3	/* waiting for the initial open */
#define	STATE_ACTIVE	5	/* currently running a talk to reg 0 */
#define	STATE_LISTEN	6	/* waiting for the first listen (turn att on) */

/*
 *	This is the basic keycode to ascii map. It is indexed by the least
 *		significant 7 bits if the key code. It returns a 3 bit key type
 *		and a 5 bit number (whose meaning depends on the key type).
 *		All ascii processing comes through here first before using the
 *		other maps for later processing.
 */

unsigned char key_map1[] = {
	0	|KEY_CAPS,		/* a */
	18	|KEY_CAPS,		/* s */
	3	|KEY_CAPS,		/* d */
	5	|KEY_CAPS,		/* f */
	7	|KEY_CAPS,		/* h */
	6	|KEY_CAPS,		/* g */
	25	|KEY_CAPS,		/* z */
	23	|KEY_CAPS,		/* x */
	2	|KEY_CAPS,		/* c */
	21	|KEY_CAPS,		/* v */
	KEY_EMPTY,
	1	|KEY_CAPS,		/* b */
	16	|KEY_CAPS,		/* q */
	22	|KEY_CAPS,		/* w */
	4	|KEY_CAPS,		/* e */
	17	|KEY_CAPS,		/* r */
	24	|KEY_CAPS,		/* y */
	19	|KEY_CAPS,		/* t */
	1	|KEY_SHIFT,		/* 1 */
	2	|KEY_SHIFT,		/* 2 */
	3	|KEY_SHIFT,		/* 3 */
	4	|KEY_SHIFT,		/* 4 */
	6	|KEY_SHIFT,		/* 6 */
	5	|KEY_SHIFT,		/* 5 */
	11	|KEY_SHIFT,		/* = */
	9	|KEY_SHIFT,		/* 9 */
	7	|KEY_SHIFT,		/* 7 */
	10	|KEY_SHIFT,		/* - */
	8	|KEY_SHIFT,		/* 8 */
	0	|KEY_SHIFT,		/* 0 */
	13	|KEY_SHIFT,		/* ] */
	14	|KEY_CAPS,		/* o */
	20	|KEY_CAPS,		/* u */
	12	|KEY_SHIFT,		/* [ */
	8	|KEY_CAPS,		/* i */
	15	|KEY_CAPS,		/* p */
	3	|KEY_PLAIN,		/* return */
	11	|KEY_CAPS,		/* l */
	9	|KEY_CAPS,		/* j */
	14	|KEY_SHIFT,		/* ' */
	10	|KEY_CAPS,		/* k */
	15	|KEY_SHIFT,		/* ; */
	16	|KEY_SHIFT,		/* \ */
	17	|KEY_SHIFT,		/* , */
	20	|KEY_SHIFT,		/* / */
	13	|KEY_CAPS,		/* n */
	12	|KEY_CAPS,		/* m */
	18	|KEY_SHIFT,		/* . */
	2	|KEY_PLAIN,		/* tab */
	1	|KEY_PLAIN,		/* space */
	19	|KEY_SHIFT,		/* ` */
	0	|KEY_PLAIN,		/* Delete */
	KEY_EMPTY,
	4	|KEY_PLAIN,		/* Escape */
	0	|KEY_SPECIAL,		/* Control */
	1	|KEY_SPECIAL,		/* Open Apple */
	2	|KEY_SPECIAL,		/* Shift */
	3	|KEY_SPECIAL,		/* Caps Lock */
	4	|KEY_SPECIAL,		/* Option */
	5	|KEY_SPECIAL,		/* Left */
	6	|KEY_SPECIAL,		/* Right */
	7	|KEY_SPECIAL,		/* Down */
	8	|KEY_SPECIAL,		/* Up */
	KEY_EMPTY,
	KEY_EMPTY,
	10	|KEY_KEYPAD,		/* . */
	KEY_EMPTY,
	11	|KEY_KEYPAD,		/* * */
	KEY_EMPTY,
	12	|KEY_KEYPAD,		/* + */
	KEY_EMPTY,
	9	|KEY_SPECIAL,		/* Clear */
	KEY_EMPTY,
	KEY_EMPTY,
	KEY_EMPTY,
	13	|KEY_KEYPAD,		/* / */
	16	|KEY_KEYPAD,		/* Enter */
	KEY_EMPTY,
	14	|KEY_KEYPAD,		/* - */
	KEY_EMPTY,
	KEY_EMPTY,
	15	|KEY_KEYPAD,		/* = */
	0	|KEY_KEYPAD,		/* 0 */
	1	|KEY_KEYPAD,		/* 1 */
	2	|KEY_KEYPAD,		/* 2 */
	3	|KEY_KEYPAD,		/* 3 */
	4	|KEY_KEYPAD,		/* 4 */
	5	|KEY_KEYPAD,		/* 5 */
	6	|KEY_KEYPAD,		/* 6 */
	7	|KEY_KEYPAD,		/* 7 */
	KEY_EMPTY,
	8	|KEY_KEYPAD,		/* 8 */
	9	|KEY_KEYPAD,		/* 9 */
	KEY_EMPTY,
	KEY_EMPTY,
	KEY_EMPTY,
	5	|KEY_FUNCTION,		/* F5 */
	6	|KEY_FUNCTION,		/* F6 */
	7	|KEY_FUNCTION,		/* F7 */
	3	|KEY_FUNCTION,		/* F3 */
	8	|KEY_FUNCTION,		/* F8 */
	9	|KEY_FUNCTION,		/* F9 */
	KEY_EMPTY,
	11	|KEY_FUNCTION,		/* F11 */
	KEY_EMPTY,
	13	|KEY_FUNCTION,		/* F13 */
	KEY_EMPTY,
	14	|KEY_FUNCTION,		/* F14 */
	KEY_EMPTY,
	10	|KEY_FUNCTION,		/* F10 */
	KEY_EMPTY,
	12	|KEY_FUNCTION,		/* F12 */
	KEY_EMPTY,
	15	|KEY_FUNCTION,		/* F15 */
	19	|KEY_FUNCTION,		/* help */
	20	|KEY_FUNCTION,		/* home */
	21	|KEY_FUNCTION,		/* page up */
	16	|KEY_FUNCTION,		/* del */
	4	|KEY_FUNCTION,		/* F4 */
	17	|KEY_FUNCTION,		/* end */
	2	|KEY_FUNCTION,		/* F2 */
	18	|KEY_FUNCTION,		/* page down */
	1	|KEY_FUNCTION,		/* F1 */
	2	|KEY_SPECIAL,		/* Shift */
	4	|KEY_SPECIAL,		/* Option */
	0	|KEY_SPECIAL,		/* Control */
	KEY_EMPTY,
	KEY_EMPTY,
};

/*
 *	Map of unshifted 'shift' type keys
 */

char key_map2[] = {
	'0',		/* 0 */
	'1',		/* 1 */
	'2',		/* 2 */
	'3',		/* 3 */
	'4',		/* 4 */
	'5',		/* 5 */
	'6',		/* 6 */
	'7',		/* 7 */
	'8',		/* 8 */
	'9',		/* 9 */
	'-',		/* - */
	'=',		/* = */
	'[',		/* [ */
	']',		/* ] */
	'\'',		/* ' */
	';',		/* ; */
	'\\',		/* \ */
	',',		/* , */
	'.',		/* . */
	'`',		/* ` */
	'/',		/* / */
};

/*
 *	Map of shifted 'shift' type keys
 */

char key_map3[] = {
	')',		/* 0 */
	'!',		/* 1 */
	'@',		/* 2 */
	'#',		/* 3 */
	'$',		/* 4 */
	'%',		/* 5 */
	'^',		/* 6 */
	'&',		/* 7 */
	'*',		/* 8 */
	'(',		/* 9 */
	'_',		/* - */
	'+',		/* = */
	'{',		/* [ */
	'}',		/* ] */
	'"',		/* ' */
	':',		/* ; */
	'|',		/* \ */
	'<',		/* , */
	'>',		/* . */
	'~',		/* ` */
	'?',		/* / */
};

/*
 *	This map contains characters that never change 
 */

char key_map4[] = {
	0x7f,		/* Delete */
	' ',		/* Space */
	'\t',		/* Tab */
	'\r',		/* Return */
	0x1b,		/* Escape */
};

/*
 *	Key map for Keypad keys
 */

char key_map5[] = {
	'0',		/* 0 */
	'1',		/* 1 */
	'2',		/* 2 */
	'3',		/* 3 */
	'4',		/* 4 */
	'5',		/* 5 */
	'6',		/* 6 */
	'7',		/* 7 */
	'8',		/* 8 */
	'9',		/* 9 */
	'.',		/* . */
	'*',		/* * */
	'+',		/* + */
	'/',		/* / */
	'-',		/* - */
	'=',		/* = */
	'\r',		/* Enter */
};

/*
 *	Map for last character of keypad string in keymap mode (ANSI)
 */

char key_map6[] = {
	'p',		/* 0 */
	'q',		/* 1 */
	'r',		/* 2 */
	's',		/* 3 */
	't',		/* 4 */
	'u',		/* 5 */
	'v',		/* 6 */
	'w',		/* 7 */
	'x',		/* 8 */
	'y',		/* 9 */
	'n',		/* . */
	0,		/* * */
	0,		/* + */
	0,		/* / */
	'm',		/* - */
	0,		/* = */
	'M',		/* Enter */
};

/*
 *	To initialize - initialize the keyboard's globals and then
 *		call fdb_open to access the keyboard and start the FSM
 */

key_init()
{
	register int i;

	for (i = 0; i < NDEVICES; i++) {
		key_call[i] =	nulldev;
		key_state[i] =	STATE_INIT;
		key_mode[i] =	KEY_ASCII;
		key_opened[i] =	KEY_CLOSED;
		key_control[i] = 0xffff;
		fdb_open(FDB_KEYBOARD, i, key_intr);
	}
}

/*
 *	Key_open sets global variables (including the ISR). It only succeeds
 *		if the device exists and has successfully initialised.
 */

key_open(id, intr, mode)
procedure_t intr;
{
	register int s;

	if (id < 0 || id >= NDEVICES || key_state[id] == STATE_INIT)
		return(0);
	key_keypad[id] =	0;
	key_call[id] =		intr;
	key_mode[id] =		mode;
	key_defwait[id] =	KEY_DEFWAIT;
	key_defgap[id] =	KEY_DEFGAP;
	key_opened[id] =	KEY_OPEN;
	return(1);
}

/*
 *	Close simply marks it as closed and cancels any timeouts
 */

key_close(id)
{
	untimeout(key_timeout, id);
	key_call[id] = nulldev;
	key_opened[id] = KEY_CLOSED;
}

/*
 *	This routine is put in the callout queue when keyboard repeats are
 *	required in ascii mode. If calls key_repeat to repeat the character
 *	(string) and then puts itself back into the queue
 */

static
key_timeout(id)
register int id;
{
	register int s;

	s = spl1();
	if (key_down[id]) {
		if (key_mode[id] == KEY_ASCII) {
			key_repeat(id);
			timeout(key_timeout, id, key_defgap[id]);
		}
	}
	splx(s);
}

/*
 *	key_op is able to be called by the next higher level. It is normally
 *		used to save the current status of the device when someone
 *		wishes to take controll of it. It allows one to set keyboard
 *		options, and at the same time recover their previous values.
 */

long
key_op(id, op, x)
long x;
{
	int s;
	long t;
	
	switch(op) {
	case KEY_OP_KEYPAD:
		t = key_keypad[id];
		key_keypad[id] = x;
		return(t);;

	case KEY_OP_MODE:
		t = key_mode[id];
		key_mode[id] = x;
		return(t);

	case KEY_OP_WAIT:
		t = key_defwait[id];
		if (x < 1) {
			key_defwait[id] = 1;
		} else {
			key_defwait[id] = x;
		}
		return(t);
		break;

	case KEY_OP_GAP:
		t = key_defgap[id];
		if (x < 1) {
			key_defgap[id] = 1;
		} else {
			key_defgap[id] = x;
		}
		return(t);

	case KEY_OP_INTR:
		s = splhi();
		t = (long)key_call[id];
		key_call[id] = (procedure_t)x;
		splx(s);
		return(t);

	case KEY_OP_OPEN:
		return(key_opened[id]);
	}
}

/*
 *	This is the ISR from the fdb driver (refer to the extensive
 *	documentation in fdb.c for the detail on how this works). It forms a
 *	FSM that handles device initialization and responds to device events
 *	reported by the lower level. It works in 3 modes ascii/ascii-raw/raw.
 *
 *	On initialisation the key driver runs the following state machine:
 *
 *	Event				Action
 *	=====				======
 *
 *	initialisation			fdb_open()
 *					/
 *		------------------------
 *		v
 *	FDB_EXISTS (timeout)		quit .... no device
 *		v
 *	FDB_EXISTS (no timeout)		fdb_flush()
 *					/
 *		------------------------
 *		v
 *	FDB_FLUSH			fdb_listen()
 *					/
 *		------------------------
 *		v
 *	FDB_LISTEN			fdb_talk() (register = 2)
 *					/
 *		------------------------
 *		v
 *	FDB_TALK			fdb_talk() (register = 0)
 *					(now in active/normal state)
 *
 *	Once it gets to this state it responds to 4 events
 *
 *	FDB_TALK (timeout)		go to sleep, wait for more events
 *
 *	FDB_TALK (no timeout)		process received characters
 *					fdb_talk() (register = 0)
 *
 *	FDB_POLL			process received characters
 *
 *	FDB_INT				if no talk pending fdb_talk()
 *
 *	FDB_UNINT			mark device idle
 */

static
key_intr(id, cmd, tim)
register id;
{
	switch(cmd) {
	case FDB_UNINT:

		/*
		 *	A device poll was canceled, mark the device as inactive
		 */

		if (key_state[id] == STATE_ACTIVE)
			key_state[id] = STATE_REG0;
		break;

	case FDB_INT:

		/*
		 *	The lower level requested a device poll, if we are not
		 *	already in the process of running an fdb transaction
		 *	then start one and mark us as busy
		 */

		if (key_state[id] == STATE_REG0) {
			fdb_talk(FDB_KEYBOARD, id, 0, &key_buff[id]);
			key_state[id] = STATE_ACTIVE;
			return(1);
		}
		return(0);

	case FDB_POLL:

		/*
		 *	A hardware poll succeeded - fake out the buffer and the
		 *	timeout to that it looks as if an fdb_talk() succeeded.
		 *	Then fall through into the FDB_TALK handler
		 */

		if (key_state[id] != STATE_REG0 && 
		    key_state[id] != STATE_ACTIVE)
			break;
		key_buff[id] = tim;
		tim = 0;
		/* Fall through */
	case FDB_TALK:

		/*
		 *	If a timeout occured go to the idle state waiting for a
		 *	FDB_POLL or FDB_INT. Otherwise process the incoming 
		 *	data. If we really got an FDB_TALK (not a poll) then
		 *	start another one to force us to be the hardware poller.
		 *	We always read register 2 first (the modifier state) to
		 *	initialise the local copy of its state. After that we
		 *	read register 0 for character data.
		 */

		if (!tim) {		/* there is no message */
			switch (key_mode[id]) {
			case KEY_ASCII:
				if (key_state[id] == STATE_REG2) {
					key_control[id] = key_buff[id];
					fdb_talk(FDB_KEYBOARD, id, 0,
							&key_buff[id]);
					key_state[id] = STATE_ACTIVE;
					return;
				} else {

					/*
					 *	For each key movement call
					 *	key_ascii to convert it to
					 *	ascii. If a key is still down
					 *	then start autorepeat. Otherwise
					 *	stop it.
					 */

					tim = (key_buff[id]&0xff) != 0xff;
					if ((key_buff[id]&0xff00) != 0xff00)
						key_ascii(id,
						   (key_buff[id]>>8)&0xff, 
						    !(key_buff[id]&0x80));
					if (tim) key_ascii(id,
						   key_buff[id]&0xff, 0);
				}
				break;
			case KEY_ARAW:
				if (key_state[id] == STATE_REG2) {
					fdb_talk(FDB_KEYBOARD, id, 0,
						&key_buff[id]);
					key_state[id] = STATE_ACTIVE;
					return;
				} else {

					/*
					 *	In this mode simply pass back
					 *	the 8 bit character code as if
					 *	it were an ascii character.
					 */

					if (key_buff[id] == 0x7f7f) {
						(*key_call[id])(id,
								KC_CHAR,
								0x7f,
								1);
						(*key_call[id])(id,
								KC_CHAR,
								0x7f,
								0);
					} else {
						tim = (key_buff[id]&0xff) !=
								0xff;
						if ((key_buff[id]&0xff00) !=
								0xff00)
						    (*key_call[id])(id,
							KC_CHAR,
							(key_buff[id]>>8)&0xff,
							tim);
						if (tim)
						    (*key_call[id])(id,
						    	KC_CHAR,
						    	key_buff[id]&0xff,
						    	0);
					}
				}
				break;
			default:
				if (key_state[id] == STATE_REG2) {
					key_r1[id] = key_buff[id];
					key_state[id] = STATE_REG0;
					(*key_call[id])(id, KC_RAW2,
							key_buff[id], 0);
					fdb_talk(FDB_KEYBOARD, id, 0,
							&key_buff[id]);
					key_state[id] = STATE_ACTIVE;
					return;
				} else {

					/*
					 *	In this mode pass back the 
					 *	register contents returned
					 *	from the device
					 */

					key_r0[id] = key_buff[id];
					(*key_call[id])(id, KC_RAW0,
							key_buff[id], 0);
				}
			}
			if (cmd != FDB_POLL) {
				fdb_talk(FDB_KEYBOARD, id, 0, &key_buff[id]);
				key_state[id] = STATE_ACTIVE;
			}
		} else {
			key_state[id] = STATE_REG0;
		}
		break;

	case FDB_LISTEN:

		/*
		 *	listen to enable service requests is done. Now do a
		 *	fdb_talk() to register 2 in order to read the keyboard
		 *	modifiers.
		 */

		fdb_talk(FDB_KEYBOARD, id, 2, &key_buff[id]);
		key_state[id] = STATE_REG2;
		break;

	case FDB_EXISTS:

		/*
		 *	The device exists (if it didn't time out). If so then
		 *	run a flush transaction to clear out its buffers
		 */

		if (!tim) {
			fdb_flush(FDB_KEYBOARD, id);
		}
		break;

	case FDB_FLUSH:

		/*
		 *	A flush transaction completed. Now do a listen to set
	 	 *	the device's service enable request bit.
		 */

		key_state[id] = STATE_LISTEN;
		key_buff[id] = 0x2000 | (FDB_KEYBOARD<<8) | 1;
		fdb_listen(FDB_KEYBOARD, id, 3, &key_buff[id], 2);
		break;

	case FDB_RESET:
		return;
	}
}

/*
 *	This routine uses the character code passed to it to look up the
 *	ascii tables and map it into one or more ascii characters.
 */

static
key_ascii(id, r0, flag)
register int id;
register int r0;
int flag;
{
	register int c;
	register int len;

	/*
	 *	If the code represents a key up code then if it is a
	 *	modifier keep track of it otherwise if it was the last
	 *	key to go down mark it as up.
	 */

	if (r0&0x80) {
		r0 &= 0x7f;
		if (((c = key_map1[r0])&KEY_TYPE) == KEY_SPECIAL) {
			switch (c&KEY_VALUE) {
			case 0:		/* Control */
				key_control[id] |= KEY_R1_CONTROL;
				return;
			case 1:		/* Open Apple */
				key_control[id] |= KEY_R1_OAPPLE;
				return;
			case 2:		/* Shift */
				key_control[id] |= KEY_R1_SHIFT;
				return;
			case 3:		/* Caps Lock */
				key_control[id] |= KEY_R1_CAPSLOCK;
				return;
			case 4:		/* Option */
				key_control[id] |= KEY_R1_OPTION;
				return;
			default:
				break;
			}
		}
		if (r0 != key_last[id]) 
			return;
		if (key_down[id])
			untimeout(key_timeout, id);
		key_down[id] = 0;
		return;
	}

	/*
	 *	Set up the key save buffer (this is where we put the latest
	 *	character string produced so that it can be repeated)
	 */

	len = 1;
	key_save = &key_buffer[8*id];

	/*
	 *	Check to see if the character code is one we know of
	 *	if not ignore it
	 */

	if ((c = key_map1[r0]) == KEY_EMPTY) {
		return;
	}

	/*
	 *	Now decode the character depending on its type from the main
	 *	table (key_map1)
	 */

	switch (c&KEY_TYPE) {
	case KEY_CAPS:

		/*
		 *	If it is a character that respoinds to capslock (all
		 *	letters) then calculate the characters value and bias
		 *	it if either capslock or shift is down. If it is a
		 *	controll character process it also.
		 */

		c &= KEY_VALUE;
		if (!(key_control[id]&KEY_R1_CONTROL)) {
			c++;
		} else
		if ((key_control[id]&(KEY_R1_CAPSLOCK|KEY_R1_SHIFT)) != 
		        (KEY_R1_CAPSLOCK|KEY_R1_SHIFT)) {
			c += 'A';
		} else {
			c += 'a';
		}
		if ((key_control[id]&KEY_R1_OPTION) == 0) 
			c |= 0x80;
		break;

	case KEY_SHIFT:

		/*
		 *	process characters that have shifted values that cant
		 *	be easily calculated from their unshifted ones
		 */

		if (!(key_control[id]&KEY_R1_SHIFT)) {
			c = key_map3[c&KEY_VALUE];
		} else {
			c = key_map2[c&KEY_VALUE];
		}
		if ((key_control[id]&KEY_R1_CONTROL) == 0) 
			c &= 0x1f;
		if ((key_control[id]&KEY_R1_OPTION) == 0) 
			c |= 0x80;
		break;

	case KEY_PLAIN:

		/*
		 *	these keys never change
		 */

		c = key_map4[c&KEY_VALUE];
		break;

	case KEY_KEYPAD:

		/*
		 *	keypad keys operate in two modes .... either as the
		 *	key that they represent or in ansi mode where they
		 *	generate an escape sequence
		 */

		if (key_keypad[id] && key_map6[c&KEY_VALUE]) {
			key_send(id, 0x1b, 1);
			key_send(id, 'O', 1);
			len = 3;
			c = key_map6[c&KEY_VALUE];
		} else {
			c = key_map5[c&KEY_VALUE];
		}
		break;

	case KEY_SPECIAL:

		/*
		 *	special keys fall into two main types ... cursor
		 *	movement keys ... which are mapped to escape sequences
		 *	and modifier keys which are kept track of.
		 */

		switch (c&KEY_VALUE) {
		case 0:		/* Control */
			key_control[id] &= ~KEY_R1_CONTROL;
			return;
		case 1:		/* Open Apple */
			key_control[id] &= ~KEY_R1_OAPPLE;
			return;
		case 2:		/* Shift */
			key_control[id] &= ~KEY_R1_SHIFT;
			return;
		case 3:		/* Caps Lock */
			key_control[id] &= ~KEY_R1_CAPSLOCK;
			return;
		case 4:		/* Option */
			key_control[id] &= ~KEY_R1_OPTION;
			return;
		case 5:		/* Left */
			c = 'D';
			goto movement;
		case 6:		/* Right */
			c = 'C';
			goto movement;
		case 7:		/* Down */
			c = 'B';
			goto movement;
		case 8:		/* Up */
			c = 'A';
		movement:
			key_send(id, 0x1b, 1);
			key_send(id, 'O', 1);
			len = 3;
			break;
		case 9:		/* Clear */
			return;
		}
		break;
	case KEY_FUNCTION:
		c &= KEY_VALUE;
		if (!(key_control[id]&KEY_R1_SHIFT)) {
			c += 'a' - 'A';
		}
		key_send(id, 0x01, 1);
		key_send(id, c+'@'-1, 1);
		c = 0x0d;
		len = 3;
		break;
	}
	key_last[id] = r0;
	key_send(id, c, flag);
	key_length[id] = len;
	if (key_down[id])
		untimeout(key_timeout, id);
	timeout(key_timeout, id, key_defwait[id]);
	key_down[id] = 1;
}

/*
 *	This routine passes back a character to the next higher level.
 *	and saves its value for later repeating
 */

static
key_send(id, c, flag)
{
	(*key_call[id])(id, KC_CHAR, c, flag);
	*key_save++ = c;
}

/*
 *	This routine repeats a key by sending its saved representation out to
 *	the next higher level
 */

static
key_repeat(id)
{
	register int i;
	register char *cp;

	cp = &key_buffer[8*id];
	for (i = 0; i < key_length[id]; i++) {
		(*key_call[id])(id, KC_CHAR, *cp++, i != key_length[id]-1);
	}
}

