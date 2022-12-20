#ifndef lint	/* .../appletalk/at/util/at_nbpd.c */
#define _AC_NAME at_nbpd_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:02:28}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_nbpd.c on 87/11/11 21:02:28";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)???.c	UniPlus VVV.2.1.9	*/
/*
 * (C) 1987 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 *	This file contains the Appletalk NBP name binding daemon for
 *	the streams based NBP.
 */

#define PROTECT		/* implement UID based name protection */

#include <sys/types.h>
#include "appletalk.h"
#include <fcntl.h>
#include <sys/stropts.h>

char buff[1000];	/* where incoming commands go to */

struct strioctl sreq,	/* used to receive commands */
		srep;	/* used to send replies */

/*
 *	This structure contains the data structure used for each
 *	name stored in the database
 *
 *	Each name is stored in two binary trees, sorted by the
 *	object and type fields resp. (I new there must be a reason I
 *	did Computer Science ....)
 */

struct entry {
	char		object[33];	/* object name */
	struct entry	*oleq;		/* object child pointer */
	struct entry	*ogtr;		/* object child pointer */
	struct entry	*oparent;	/* pointer back up */
	char		type[33];	/* type name */
	struct entry	*tleq;		/* type child pointer */
	struct entry	*tgtr;		/* type child pointer */
	struct entry	*tparent;	/* pointer back up */
	int		net;		/* our net number */
	int		node;		/* our node number */
	int		socket;		/* our socket number */
	int		number;		/* the 'enumerator' */
#ifdef PROTECT
	int		uid;		/* the user who registered it */
#endif PROTECT
};

at_ddp_cfg_t cfg;			/* the curent DDP configuration */

struct entry *locate();
struct entry *buildentry();
int lookup_reply();

struct entry *ohead = NULL,		/* head of the tree sorted by object name */
	     *thead = NULL;		/* head of the tree sorted by type name */

int waiting;				/* true if there is a partially complete message
					   not yet all sent */
int repioctl;				/* the ioctl to use when replying */
at_nbp_tuple_hdr *tuple;		/* the current tuple header */
at_nbp *xnbp;				/* the current NBP header */
at_ddp_t *xddp;				/* the current DDP header */
int space_left;				/* how many bytes left in the message */
int fd;					/* the NBP socket's file desc. */
int bad;				/* true if the message just parsed was invalid */

struct entry *idtab[256];		/* pointers to the entries indexed by id number*/
int idind = 0;				/* used for allocating id numbers */

char ob[1000];				/* the buffer used for assembling output
					   messages */

main()
{
	register struct entry *ep, *nep;
	register int i, j;
	register int uid;
	char c;

	/*
	 *	First open the NBP socket and push the NBPD module on it.
	 *	Next get the system's net and node numbers
	 */

	fd = at_open_socket(2, O_RDWR);
	if (fd < 0) {
		perror("NBPD open failed");
		exit(2);
	}
	i = ioctl(fd, I_PUSH, "at_nbpd");
	if (i < 0) {
		perror("NBPD open failed");
		exit(2);
	}
	get_cfg();

	/*
	 *	Next fork the daemon and then get ot to close all it's
	 *		files
	 */

	i = fork();
	if (i > 0)
		exit(0);
	if (i < 0) {
		perror("NBPD Fork failed");
		exit(2);
	}
	for (i = 0;i<50;i++) 
	if (i != fd)
		close(i); 

	/*
	 *	Now set up the request packet and loop getting requests from
	 *		the stream's module
	 */

	sreq.ic_cmd = AT_NBPD_GET_NEXT;
	sreq.ic_dp = buff;
	sreq.ic_timout = -1;
	srep.ic_timout = -1;
	for (;;) {
		sreq.ic_len = 0;
		i = ioctl(fd, I_STR, &sreq);
		if (i == -1) {
			(void) ioctl(fd, I_FLUSH, FLUSHRW);
			break;
		}

		/*
		 *	The returned value has a request code in the lower 8 bits 
		 *	and the uid of the requester in the next 8, and the current net number
		 *	in the highest 16
		 */

		cfg.this_net = (i>>16)&0xffff;
		uid = (i>>8)&0xff;
		i &= 0xff;

		/*
		 *	Do what is requested
		 */

		switch (i) {
		case NBPD_CLOSE:

			/*
			 *	If a socket on which a name was once registered
			 *	was closed then remove all names associated
			 *	with it from the names table and delete them
			 */

			j = (unsigned char)buff[0];
			for (i = 0; i < 256; i++) 
			if ((ep = idtab[i]) != NULL && ep->socket == j) {
				delete(ep);
				free(ep);
			}
			break;

		case NBPD_REGISTER:

			/*
			 *	when registering:
			 *		- check the name
			 *		- see if it is in the table already (might
			 *		  have been a timing hole)
			 *		- pack it all into an entry structure,
			 *		- check to see it is actually open (and mark it
			 *		  so we get told when it is closed)
			 *		- allocate an enumerator (if none are available
			 *		  can out)
			 *		- finally insert it into the trees and return
			 *		  the enumerator.
			 *
			 *	Note: if we return no data it is assumed that an
		 	 *		error occured during registration
			 */

			ep = locate(NBP_NBP(buff), NULL);
			if (ep || bad) {
				srep.ic_len = 0;
			} else {
				ep = buildentry(NBP_NBP(buff)->at_nbp_tuples, uid);
				if (ep) {
					c = ep->socket;
					srep.ic_len = 1;
					srep.ic_dp = &c;
					srep.ic_cmd = AT_DDP_IS_OPEN;
					if (ioctl(fd, I_STR, &srep) < 0 ||
					    alloc_number(ep)) {
						srep.ic_len = 0;
						free(ep);
					} else {
						insert(ep);
						srep.ic_len = sizeof(int);
						*(int *)buff = ep->number;
					}
				} else {
					srep.ic_len = 0;
				}
			}
			srep.ic_dp = buff;
			srep.ic_cmd = AT_NBPD_REGISTER_DONE;
			(void) ioctl(fd, I_STR, &srep);
			break;

		case NBPD_DELETE_NAME:

			/*
			 *	When deleting by name:
			 *		- check the name exists and is valid
			 *		- next check the uid on it must be
			 *		  root or owner to delete
			 *		- finally delete and free the entry
			 *
			 *	Note: returning data is recognised as a success
			 *	      no data is error
			 */

			ep = locate(NBP_NBP(buff), NULL);
			if (ep == NULL || bad
#ifdef PROTECT
				|| (uid != 0 && ep->uid != uid)
#endif PROTECT
							) {
				srep.ic_len = 0;
			} else {
				srep.ic_len = 1;
				srep.ic_dp = buff;
				delete(ep);
				free(ep);
			}
			srep.ic_dp = buff;
			srep.ic_cmd = AT_NBPD_DELETE_DONE;
			(void) ioctl(fd, I_STR, &srep);
			break;

		case NBPD_DELETE:

			/*
			 *	When deleting:
			 *		- check the id number being used for the
			 *		  delete
			 *		- next check it is actually valid
			 *		- next check the uid on it must be
			 *		  root or owner to delete
			 *		- finally delete and free the entry
			 *
			 *	Note: returning data is recognised as a success
			 *	      no data is error
			 */

			j = *(int *)buff;
			if (j < 0 || j >= 256 ||
			    (ep = idtab[j]) == NULL 
#ifdef PROTECT
				|| (uid != 0 && ep->uid != uid)
#endif PROTECT
							) {
				srep.ic_len = 0;
			} else {
				srep.ic_len = 1;
				srep.ic_dp = buff;
				delete(ep);
				free(ep);
			}
			srep.ic_dp = buff;
			srep.ic_cmd = AT_NBPD_DELETE_DONE;
			(void) ioctl(fd, I_STR, &srep);
			break;

		case NBPD_LOCAL_LOOKUP:

			/*
			 *	Local lookup is done for local processes that are doing
			 *	lookups. It is reliable and so only needs to be done
			 *	once.
			 *
			 *		- clear buffer and set up it's header
			 *		- search the trees calling the action routine
			 *		  for every entry found
			 *		- send any data still waiting to be sent
			 *		- finally send a message to say we are done
			 */

			(void)set_up(buff, AT_NBPD_LOCAL);
			ep = locate(NBP_NBP(buff), lookup_reply);
			if (waiting)
				send_rest();
			srep.ic_len = 0;
			srep.ic_cmd = AT_NBPD_LOCAL_DONE;
			i = ioctl(fd, I_STR, &srep);
			break;

		case NBPD_REMOTE_LOOKUP:

			/*
			 *	Remote lookup is basicly the same as local lookup
			 *	except we don't have to acknowledge it's completion
			 *	and we use a different ioctl. It is NOT reliable
			 */

			if (!set_up(buff, AT_NBPD_REMOTE))
				break;
			ep = locate(NBP_NBP(buff), lookup_reply);
			if (waiting)
				send_rest();
			break;

		case NBPD_SHUTDOWN:

			/*
			 *	Shutdown just replies and the quits
			 */

			srep.ic_len = 0;
			srep.ic_cmd = AT_NBPD_SHUTDOWN_DONE;
			(void) ioctl(fd, I_STR, &srep);
			exit(0);
		}
	}
}

/*
 *	Locate searches for aone or more entries in the tree(s). There are two interfaces
 *	one for actions that just need to check for existance, the other for those who
 *	need all possible entries (wild card searches). Actions:
 *
 *		- parse the message, if error set bad and return, set up the
 *		  name pointers, uppercase everything and append NULLs (buildentry
 *		  depends on these side effects)
 *		- next decide the type of search to be done:
 *			- if both object and type are wild do a tree walk (doesn't
 *			  matter which one)
 *			- otherwise search one of the non-wild carded trees for equality
 *			  (and check the other for wildcardedness or equality)
 *
 */

struct entry *
locate(nbp, func)
at_nbp *nbp;
int (*func)();
{
        at_nbp_tuple_hdr *tp;
	register struct entry * ep;
	char *object, *type, *zone;
	int owild, twild;
	register int zlen, len, i;
	register char *cp;
	
	bad = 0;
	owild = 0;
	twild = 0;
	tp = (at_nbp_tuple_hdr *)nbp->at_nbp_tuples;
	object = (char *)tp->at_nbp_tuple_data;
	len = *object;
	if (len < 0 || len > 32) {
		bad = 1;
		return(NULL);
	}
	object += 1;
	if (len == 1 && *object == '=')
		owild = 1;
	type = object+len;
	cp = object+len;
	len = *type;
	*type = '\0';
	if (len < 0 || len > 32) {
		bad = 1;
		return(NULL);
	}
	type += 1;
	if (len == 1 && *type == '=')
		twild = 1;
	zone = type + len;
	cp = type + len;
	len = *zone;
	*zone++ = '\0';
	if (len < 0 || len > 32) {
		bad = 1;
		return(NULL);
	}
	cp = zone + len;
	zlen = len;
	*cp = '\0';
	if (zlen < 0 || zlen > 1 || (zlen == 1 && *zone != '*')) {
		bad = 1;
		return(NULL);
	}
	if ((owild || twild) && func == NULL) {
		bad = 1;
		return(NULL);
	}
	if (twild) {
		if (owild) {
			depth(ohead, func);
		} else {
			ep = ohead;
			while (ep) {
				i = strupcmp(object, ep->object);
				if (i == 0) {
					for (;;) {
						(*func)(ep);
						ep = ep->oleq;
						if (ep == NULL ||
						    strupcmp(object, ep->object))
							break;
					}
					break;
				} else
				if (i < 0) {
					ep = ep->oleq;
				} else {
					ep = ep->ogtr;
				}
			}
		}
	} else {
		ep = thead;
		while (ep) {
			i = strupcmp(type, ep->type);
			if (i == 0) {
				for (;;) {
					if (owild || strupcmp(object, ep->object) == 0)
					if (func) {
						(*func)(ep);
					} else {
						return(ep);
					}
					ep = ep->tleq;
					if (ep == NULL || strupcmp(type, ep->type))
						break;
				}
				break;
			} else
			if (i < 0) {
				ep = ep->tleq;
			} else {
				ep = ep->tgtr;
			}
		}
	}
	return(NULL);
}

/*
 *	depth does a tree walk for double wild cards
 */

depth(ep, func)
register struct entry *ep;
register int (*func)();
{
	if (ep) {
		(*func)(ep);
		if (ep->oleq)
			depth(ep->oleq, func);
		if (ep->ogtr)
			depth(ep->ogtr, func);
	}
}

/*
 *	Insert adds an entry to both trees
 */

insert(ep)
register struct entry *ep;
{
	register struct entry *exp, *enp;
	register int n;

	if (ohead == NULL) {			/* the first entry does at the head */
		thead = ohead = ep;
		ep->ogtr = ep->oleq = NULL;
		ep->tgtr = ep->tleq = NULL;
		ep->oparent = ep->tparent = NULL;
		return;
	}
	exp = ohead;
	while (exp) {				/* search down for a place to insert */
		n = strupcmp(ep->object, exp->object);
		if (n == 0) {				/* if equal insert it where */
			if (exp->oparent == NULL) {	/* the old entry was and hang */
				ohead = ep;		/* the old one under it */
				ep->oparent = NULL;
			} else {
				if (exp->oparent->oleq == exp) {
					exp->oparent->oleq = ep;
				} else {
					exp->oparent->ogtr = ep;
				}
				ep->oparent = exp->oparent;
			}
			exp->oparent = ep;
			ep->oleq = exp;
			ep->ogtr = NULL;
			break;
		} else 
		if (n > 0) {				/* otherwise if > and there is */
			if (exp->ogtr == NULL) {	/* nothing below insert it */
				exp->ogtr = ep;
				ep->oparent = exp;
				ep->ogtr = ep->oleq = NULL;
				break;
			}
			exp = exp->ogtr;		/* if there is something below */
							/* recurse downwards */
		} else {
			if (exp->oleq == NULL) {	/* do the same for < */
				exp->oleq = ep;
				ep->oparent = exp;
				ep->ogtr = ep->oleq = NULL;
				break;
			}
			exp = exp->oleq;
		}
	}

	/*
	 *	Now repeat the other for the other tree
	 */

	exp = thead;
	while (exp) {
		n = strupcmp(ep->type, exp->type);
		if (n == 0) {
			if (exp->tparent == NULL) {
				thead = ep;
				ep->tparent = NULL;
			} else {
				if (exp->tparent->tleq == exp) {
					exp->tparent->tleq = ep;
				} else {
					exp->tparent->tgtr = ep;
				}
				ep->tparent = exp->tparent;
			}
			exp->tparent = ep;
			ep->tleq = exp;
			ep->tgtr = NULL;
			break;
		} else 
		if (n > 0) {
			if (exp->tgtr == NULL) {
				exp->tgtr = ep;
				ep->tparent = exp;
				ep->tgtr = ep->tleq = NULL;
				break;
			}
			exp = exp->tgtr;
		} else {
			if (exp->tleq == NULL) {
				exp->tleq = ep;
				ep->tparent = exp;
				ep->tgtr = ep->tleq = NULL;
				break;
			}
			exp = exp->tleq;
		}
	}
}

/*
 *	delete removes an entry from both trees and the id table
 */

delete(ep)
register struct entry *ep;
{
	register struct entry *enp, *exp;

	/*
	 *	First do the types tree
	 */

	/*
	 *	If there is a LHS hoist it up
	 */

	if (ep->oleq) {
		if (ep->oparent) {
			if (ep->oparent->oleq == ep) {
				ep->oparent->oleq = ep->oleq;
			} else {
				ep->oparent->ogtr = ep->oleq;
			}
		} else {
			ohead = ep->oleq;
		}
		ep->oleq->oparent = ep->oparent;

		/*
		 *	If there is also a RHS find somewhere to put it in the tree
		 */

		if (enp = ep->ogtr) {
			exp = ep->oleq;
			for (;;) {
				if (exp->ogtr == NULL) {
					exp->ogtr = enp;
					enp->oparent = exp;
					break;
				}
				exp = exp->ogtr;
			}
		}
	} else  {

		/*
		 *	Otherwise hoist the RHS (if present)
		 */

		if (ep->oparent) {
			if (ep->oparent->oleq == ep) {
				ep->oparent->oleq = ep->ogtr;
			} else {
				ep->oparent->ogtr = ep->ogtr;
			}
		} else {
			ohead = ep->ogtr;
		}
		if (ep->ogtr)
			ep->ogtr->oparent = ep->oparent;
	}

	/*
	 *	do the same for the types tree
	 */

	if (ep->tleq) {
		if (ep->tparent) {
			if (ep->tparent->tleq == ep) {
				ep->tparent->tleq = ep->tleq;
			} else {
				ep->tparent->tgtr = ep->tleq;
			}
		} else {
			thead = ep->tleq;
		}
		ep->tleq->tparent = ep->tparent;

		/*
		 *	If there is also a RHS find somewhere to put it in the tree
		 */

		if (enp = ep->tgtr) {
			exp = ep->tleq;
			for (;;) {
				if (exp->tgtr == NULL) {
					exp->tgtr = enp;
					enp->tparent = exp;
					break;
				}
				exp = exp->tgtr;
			}
		}
	} else  {

		/*
		 *	Otherwise hoist the RHS (if present)
		 */

		if (ep->tparent) {
			if (ep->tparent->tleq == ep) {
				ep->tparent->tleq = ep->tgtr;
			} else {
				ep->tparent->tgtr = ep->tgtr;
			}
		} else {
			thead = ep->tgtr;
		}
		if (ep->tgtr)
			ep->tgtr->tparent = ep->tparent;
	}
	idtab[ep->number] = NULL;
}

/*
 *	fill_tuple fills in a tuple, returning the space used in bytes
 */

fill_tuple(tp, net, node, socket, number, object, type)
register at_nbp_tuple_hdr *tp;
register char *object, *type;
{
	register int len, tlen;
	register char *cp;

	W16(tp->at_nbp_tuple_net, net);
        tp->at_nbp_tuple_node = node;
        tp->at_nbp_tuple_socket = socket;
        tp->at_nbp_tuple_enumerator = number;
	tlen = 4 + AT_NBP_TUPLE_HDR_SIZE;
	cp = (char *)tp->at_nbp_tuple_data;
	*cp++ = len = strlen(object);
	strcpy(cp, object);
	cp += len;
	tlen += len;
	*cp++ = len = strlen(type);
	strcpy(cp, type);
	cp += len;
	*cp++ = 1;
	*cp++ = '*';
	return(len+tlen);
}

/*
 *	buildentry allocates an entry data structure and packs a register packet
 *	into it. It assumes that the processing done by locate() has been done to
 *	the buffer
 */

struct entry *
buildentry(tp, uid)
register at_nbp_tuple_hdr *tp;
{
	register struct entry *ep;
	register int len;
	register char *cp, *xcp;

	ep = (struct entry *)malloc(sizeof(struct entry));
	if (ep == NULL)
		return(NULL);
	ep->net = cfg.this_net;
	ep->node = cfg.this_node;
	ep->socket = tp->at_nbp_tuple_socket;
	ep->number = tp->at_nbp_tuple_enumerator;
#ifdef PROTECT
	ep->uid = uid;
#endif PROTECT
	cp = (char *)tp->at_nbp_tuple_data;
	cp++;
	xcp = cp;
	while (*xcp)
		xcp++;
	xcp++;
	strcpy(ep->object, cp);
	cp = xcp;
	while (*xcp)
		xcp++;
	strcpy(ep->type, cp);
	return(ep);
}

/*
 *	alloc_number allocates an enumerator for an entry and adds it to the
 *		enumerator table. It returns 0 on success and -1 if all the
 *		ids are used up
 */

alloc_number(ep)
register struct entry *ep;
{
	register int i, j;

	j = idind;
	for (;;) {
		i = ++idind;
		if (idind > 256)
			idind = 0;
		if (i == j)
			return(-1);
		if (idtab[i] == NULL) {
			ep->number = i;
			idtab[i] = ep;
			return(0);
		}
	}
}

/*
 *	setup sets up the buffers etc for a lookup reply
 */

set_up(buff, type)
register char *buff;
{
	register at_ddp_t *ddp;
	register at_nbp *nbp;
	register at_nbp_tuple_hdr *tp;

	ddp = NBP_DDP(buff);
	nbp = NBP_NBP(buff);
	repioctl = type;
	waiting = 0;
	space_left = 570;
	xnbp = NBP_NBP(ob);
	xddp = NBP_DDP(ob);
	tp = (at_nbp_tuple_hdr *)nbp->at_nbp_tuples;
	if (type == AT_NBPD_REMOTE &&
	    cfg.this_node == tp->at_nbp_tuple_node &&
	    cfg.this_net == R16(tp->at_nbp_tuple_net))
		return(0);
	xddp->dst_socket = tp->at_nbp_tuple_socket;
	xddp->dst_node = tp->at_nbp_tuple_node;
	C16(xddp->dst_net, tp->at_nbp_tuple_net);
	xddp->type = AT_DDP_TYPE_NBP;
	xnbp->at_nbp_ctl = AT_NBP_LKUP_REPLY;
	xnbp->at_nbp_tuple_count = 0;
	xnbp->at_nbp_id = nbp->at_nbp_id;
	tuple = (at_nbp_tuple_hdr *)xnbp->at_nbp_tuples;
	return(1);
}

/*
 *	send_rest sends the reply to a lookup request
 */

send_rest()
{
	srep.ic_cmd = repioctl;
	srep.ic_len = (char *)tuple - ob;
	srep.ic_dp = ob;
	(void) ioctl(fd, I_STR, &srep);
}

/*
 *	lookup_reply is called from locate() (indirectly) to put a tuple in a
 *		reply packet, if the packet is full it is sent and a new one started
 */

lookup_reply(ep)
register struct entry *ep;
{
	register int len;

	len = fill_tuple(tuple, cfg.this_net, ep->node, ep->socket,
			        ep->number, ep->object, ep->type);
	if (len > space_left) {
		srep.ic_cmd = repioctl;
		srep.ic_len = (char *)tuple - ob;
		srep.ic_dp = ob;
		(void) ioctl(fd, I_STR, &srep);

		space_left = 570;
		xnbp->at_nbp_tuple_count = 1;
		tuple = (at_nbp_tuple_hdr *)xnbp->at_nbp_tuples;
		(void)fill_tuple(tuple, ep->net, ep->node, ep->socket,
			        	ep->number, ep->object, ep->type);
	} else {
		xnbp->at_nbp_tuple_count++;
	}
	waiting = 1;
	space_left -= len;
	tuple = (at_nbp_tuple_hdr *)(((char *)tuple) + len);
}

int 
strupcmp(cp1, cp2)
register char *cp1, *cp2;
{
	register char c1, c2;

	for (;;) {
		c1 = *cp1++;
		c2 = *cp2++;
		if (c1 >= 'a' && c1 <= 'z')
			c1 += 'A' - 'a';
		if (c2 >= 'a' && c2 <= 'z')
			c2 += 'A' - 'a';
		if (c1 < c2)
			return(-1);
		if (c1 > c2)
			return(1);
		if (c1 == 0)
			return(0);
	}
}

get_cfg()
{
	struct strioctl sreq;
	int i;

	sreq.ic_timout = -1;
	sreq.ic_cmd = AT_DDP_GET_CFG;
	sreq.ic_dp = (char *)&cfg;
	sreq.ic_len = 0;
	i = ioctl(fd, I_STR, &sreq);
	if (i < 0) {
		perror("NBPD get cfg failed");
		exit(2);
	}
}

