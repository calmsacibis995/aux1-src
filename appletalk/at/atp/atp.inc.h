#ifndef lint	/* .../appletalk/at/atp/atp.inc.h */
#define _AC_NAME atp_inc_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:55:43}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of atp.inc.h on 87/11/11 20:55:43 */
#define static

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/stream.h>
#include <sys/var.h>
#include <sys/debug.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include "../appletalk.h"

/*
 *	streams interface structure
 */

static int atp_open();
static int atp_close();
static int atp_rputq();
static int atp_rsrvc();
static int atp_wputq();
static int atp_wsrvc();
extern int nulldev();

#ifdef ATP_DECLARE
static struct 	module_info atp_minfo = { 98, "at_atp", 0, 256, 256, 256, NULL };
static struct	qinit atprdata = { atp_rputq, atp_rsrvc, atp_open, atp_close,
			nulldev, &atp_minfo, NULL};
static struct	qinit atpwdata = { atp_wputq, atp_wsrvc, atp_open, atp_close, 
			nulldev, &atp_minfo, NULL};
struct	streamtab atp_info = {&atprdata, &atpwdata, NULL, NULL};
#endif ATP_DECLARE

/*
 *	Stuff for accessing protocol headers 
 */

#define AT_DDP_HDR(m) ((at_ddp_t *)(m->b_rptr))
#define AT_ATP_HDR(m) ((at_atp *)(&((at_ddp_t *)(m->b_rptr))->data[0]))

#define ATP_MAXWAIT	10
/*
 *	Masks for accessing/manipulating the bitmap field in atp headers
 */

#ifdef ATP_DECLARE
unsigned char atp_mask [] = {
	0x01, 0x02, 0x04, 0x08, 
	0x10, 0x20, 0x40, 0x80, 
};

unsigned char atp_lomask [] = {
	0x00, 0x01, 0x03, 0x07, 
	0x0f, 0x1f, 0x3f, 0x7f, 
	0xff
};
#else
extern unsigned char atp_mask [];
extern unsigned char atp_lomask [];
#endif ATP_DECLARE

/*
 *	doubly linked queue types and primitives
 */

#define ATP_Q_ENTER(hdr, object, entry) {					\
		if ((hdr).head) {						\
			(hdr).head->entry.prev = (object);			\
			(object)->entry.next = (hdr).head;			\
		} else {							\
			(hdr).tail = (object);					\
			(object)->entry.next = NULL;				\
		}								\
		(object)->entry.prev = NULL;					\
		(hdr).head = (object);						\
	}

#define ATP_Q_APPEND(hdr, object, entry) {					\
		if ((hdr).head) {						\
			(hdr).tail->entry.next = (object);			\
			(object)->entry.prev = (hdr).tail;			\
		} else {							\
			(hdr).head = (object);					\
			(object)->entry.prev = NULL;				\
		}								\
		(object)->entry.next = NULL;					\
		(hdr).tail = (object);						\
	}

#define ATP_Q_REMOVE(hdr, object, entry) {					\
		if ((object)->entry.prev) {					\
			(object)->entry.prev->entry.next = (object)->entry.next;\
		} else {							\
			(hdr).head = (object)->entry.next;			\
		}								\
		if ((object)->entry.next) {					\
			(object)->entry.next->entry.prev = (object)->entry.prev;\
		} else {							\
			(hdr).tail = (object)->entry.prev;			\
		}								\
	}

struct atp_rcb_qhead {
	struct atp_rcb 	*head;
	struct atp_rcb 	*tail;
};

struct atp_rcb_q {
	struct atp_rcb *prev;
	struct atp_rcb *next;
};

struct atp_trans_qhead {
	struct atp_trans *head;
	struct atp_trans *tail;
};

struct atp_trans_q {
	struct atp_trans *prev;
	struct atp_trans *next;
};

/*
 *	Locally saved remote node address
 */

struct atp_socket {
	u16		net;
	at_node		node;
	at_socket	socket;
};

/*
 *	transaction control block (local context at requester end)
 */

struct atp_trans {
	struct atp_trans_q	tr_list;		/* trans list */
	struct atp_state	*tr_queue;		/* state data structure */
	mblk_t			*tr_xmt;		/* message being sent */
	mblk_t			*tr_rcv[8];		/* message being rcvd */
	unsigned int		tr_retry;		/* # retries left */
	unsigned int		tr_timeout;		/* timer interval */
	int			tr_state;		/* current state */
	int			tr_xo;			/* execute once transaction */
	int			tr_tid;			/* transaction id */
	unsigned char		tr_bitmap;		/* requested bitmask */
	struct atp_socket	tr_socket;		/* the remote socket id */
	struct atp_trans_q	tr_snd_wait;		/* list of transactions waiting
							   for space to send a msg */
};

#define	TRANS_TIMEOUT		0			/* waiting for a reply */
#define	TRANS_REQUEST		1			/* waiting to send a request */
#define	TRANS_RELEASE		2			/* waiting to send a release */
#define	TRANS_DONE		3			/* done - waiting for a poll to 
							   complete */
#define	TRANS_FAILED		4			/* done - waiting for a poll to
							   report failure */
#define	TRANS_BUILD		5			/* done - waiting msg space */

/*
 *	reply control block (local context at repling end)
 */

struct atp_rcb {
	struct atp_rcb_q	rc_list;		/* rcb list */
	struct atp_state	*rc_queue;		/* state data structure */
	mblk_t			*rc_xmt[8];		/* replys being sent */
	mblk_t			*rc_ioctl;		/* waiting ioctl */
	char			rc_snd[8];		/* replys actually to be sent */
	int			rc_state;		/* current state */
	int			rc_xo;			/* execute once transaction */
	unsigned short		rc_tid;			/* transaction id */
	int			rc_rep_waiting;		/* in the reply wait list */
	int			rc_timer;		/* reply timer */
	int			rc_note;		/* send data when ready */
	struct atp_rcb_q	rc_rep_wait;		/* list of replies */
	unsigned char		rc_bitmap;		/* replied bitmask */
	unsigned char		rc_not_sent_bitmap;	/* replied bitmask */
	struct atp_socket	rc_socket;		/* the remote socket id */
};

#define RCB_IOCTL		1		/* waiting for incoming request */
#define RCB_RESPONDING		2	      	/* waiting all of response from process*/
#define RCB_RESPONSE_FULL	3	      	/* got all of response ... waiting for
						   timeout */
#define RCB_RELEASED		4	      	/* got our release */
#define RCB_IOCTL_NW		5		/* ioctl notified, but not waiting */
#define RCB_IOCTL_FULL		6		/* a no wait rcb is full */

/*
 *	socket state (per module data structure)
 */

struct atp_state {
	struct atp_trans_qhead	atp_trans_wait;		/* pending transaction list */
	struct atp_trans_qhead	atp_trans_snd;		/* list of transactions waiting
							   for space to send a msg */
	struct atp_state	*atp_trans_waiting;	/* list of atps waiting for a
							   free transaction */
	int			atp_trans_wait_flag;	/* we are waiting for a */
							/* transaction record */
	queue_t			*atp_q;			/* read q */
	unsigned int		atp_retry;		/* retry count */
	unsigned int		atp_timeout;		/* retry timeout */
	struct atp_state	*atp_rcb_waiting;	/* list of atps waiting for a
							   free rcb */
	int			atp_rcb_wait_flag;	/* we are waiting for a 
							   rcb record */
	struct atp_rcb_qhead	atp_rcb;		/* active rcbs */
	struct atp_rcb_qhead	atp_rep_wait;		/* replies waiting to be sent 
							   to ioctls */
	struct atp_rcb_qhead	atp_unattached;		/* rcb's waiting for requests */
	unsigned char 		atp_mwait;		/* we are waiting for mblks to
							   be free */
	mblk_t 			*atp_finish;		/* send a message when we are
							   clear */
	int			atp_lasttid;		/* last used tid */
	int			atp_nwait;		/* number of waiting RCBs */
};

/*
 *	tcb/rcb/state allocation queues
 */

#ifdef ATP_DECLARE
static struct atp_trans *atp_trans_free_list = NULL;	/* free transactions */
static struct atp_state *atp_state_trans_waiting = NULL;/* trans waiting sessions */
static struct atp_rcb *atp_rcb_free_list = NULL;	/* free rcbs */
static struct atp_state *atp_state_rcb_waiting = NULL;	/* rcb waiting sessions */
static struct atp_state *atp_free_list = NULL;		/* free atp states */

static struct atp_trans atp_trans_data[NATP_TRANS];
static struct atp_rcb atp_rcb_data[NATP_RCB];
static struct atp_state atp_state_data[NATP_STATE];

static int atp_ioctl_id = 987654321;

static atp_req_timeout();
static atp_rcb_timer();
static atp_wwake();
static atp_clear();
static unsigned short atp_short();
static atp_x_done();
static struct atp_rcb *atp_rcb_alloc();
static struct atp_trans *atp_trans_alloc();
#else
extern struct atp_trans *atp_trans_free_list;		/* free transactions */
extern struct atp_state *atp_state_trans_waiting;	/* trans waiting sessions */
extern struct atp_rcb *atp_rcb_free_list;		/* free rcbs */
extern struct atp_state *atp_state_rcb_waiting;		/* rcb waiting sessions */
extern struct atp_state *atp_free_list;			/* free atp states */

extern struct atp_trans atp_trans_data[];
extern struct atp_rcb atp_rcb_data[];
extern struct atp_state atp_state_data[];

extern int atp_ioctl_id;

extern atp_req_timeout();
extern atp_rcb_timer();
extern atp_wwake();
extern atp_clear();
extern unsigned short atp_short();
extern atp_x_done();
extern struct atp_rcb *atp_rcb_alloc();
extern struct atp_trans *atp_trans_alloc();
#endif ATP_DECLARE

#ifdef DEBUG
extern T_atp;
extern T_atp_rep;
extern T_atp_req;
extern T_atp_alloc;
#endif DEBUG
