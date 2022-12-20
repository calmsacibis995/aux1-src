#ifndef lint	/* .../appletalk/at/ddp/at_ddp.c */
#define _AC_NAME at_ddp_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:56:58}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_ddp.c on 87/11/11 20:56:58";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)at_ddp.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *	The ddp module is multiplexing which means, there is one connection
 *	downstream (lap type 1 and 2) and multiple connection upstream. The
 *	conections upstream are called demultiplexing queues and are viewed from
 *	the user interface as files in the appletalk directory with
 *	the name "socket1" thru "socket254".
 *
 *	There are two parts to the lap module code. That which is common to
 *	all ddp modules on the unix machine, no matter which network it is 
 *	connected to. And, that which is specific to a particular network
 *      This file contains the common routines that all ddp modules execute.
 *	As in other multiplexing or multiply occuring streams, the only
 *	difference is the durring the open. The open allocates space and must
 *	be seperately compiled, and thus is in a seperate file, at_ddpopen.c.
 *
 *	This file is organized into the following sections:
 *		defines and storage
 *		ddp stream module
 */


#include <sys/types.h>
#include <errno.h>
#include <sys/reg.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/param.h>
#include <appletalk.h>
#ifdef  FEP
#include <fwdicp.h>
#endif  FEP
#include <at_ddp.h>
#include <sys/debug.h>
extern int T_ddp;

extern	ddp_specifics_t	ddp_specifics;


/************************************************************************/
/*									*/
/*									*/
/*				support routines			*/
/*									*/
/*									*/
/************************************************************************/

/*
 *	zero out the stats structure. I know this is the lazy way, but it is
 *	very immune to changes in the structure's definition.
 */

static void
init_stats(ddpp)
ddp_specifics_t *ddpp;
{
	caddr_t		p;
	u32		i;

	for (p = (caddr_t) &ddpp->cfg.stats, i = sizeof(at_ddp_stats_t);
	     i--;
	     *p++ = 0
	    );
}

/*+-------------------------------------------------------------------------+
  |     C A L C U L A T E   D D P    C H E C K S U M    R O U T I N E       |
  +-------------------------------------------------------------------------+
  |
  |  Function:
  |     calculate_ddp_chksum(data,length)
  |
  |  Arguments:
  |     dgp	pointer to the lap_ddp_t.
  |
  |  Returns:
  |     Unsigned 16-bit DDP chekcsum value.
  |
  |  Description:
  |     Implements the DataGram chekcsum as defined in
  |     II.7 (7/20/84) of ``Inside AppleTalk''.
  |
  |  Algorithm:
  |     Add the unsigned bytes into an unsigned 16-bit accumulator.
  |     After each add, rotate the sign bit into the low order bit of
  |     the accumulator. When done, if the checksum is 0, changed into 0xFFFF.
  |
  |  History:
  |     21-Aug-85: PKR. Created.
  +*/
static u16
calculate_ddp_chksum(m, offset)
register mblk_t *m;
{
	register  at_ddp_t	*dgp;
        register  u8	*data;
        register  int   length;
        register  u16	checksum;

	data = (unsigned char *)m->b_rptr + offset;
	if (data >= (unsigned char *)m->b_wptr) {
		m = m->b_cont;
		data = (unsigned char *)m->b_rptr;
	}
	dgp =  (at_ddp_t *)data;
        length = R16(dgp->length) - AT_DDP_CHECKSUM_FUDGE;
	data = &dgp->dst_net[0];
        checksum = 0;
        while (length--)
            {
	    while (data >= (unsigned char *)m->b_wptr) {
		m = m->b_cont;
		if (m == NULL)
			return(0);
		data = (unsigned char *)m->b_rptr;
	    }
            checksum += *data++;
            if (checksum & 0x8000)
                {
                checksum = (checksum << 1) | 1;
                }
            else
                {
                checksum <<= 1;
                }
            }
        if (checksum == 0)
            {
            checksum = 0xffff;
            }
        return(checksum);
}

/************************************************************************/
/*									*/
/*									*/
/*				module code				*/
/*									*/
/*									*/
/************************************************************************/

/*
 *	This is the timeout function that is called after 90 seconds, if no bridge
 *	packets come in. That way we won't send extended frames to something that is
 *	not there. Untimeout is called if an RTMP packet comes in so this routine
 *	will not be called.
 */

ddp_age_bridge(int_ddpp, time)
{
	TRACE(T_ddp, ("ddp_age_bridge: no contact from a_bridge for 90 seconds\n"));
	/* assume that there is no bridge */
	((ddp_specifics_t *) int_ddpp)->cfg.a_bridge = 0;
}



/*
 * well lap has brought the network down, dup and send it upstream to 
 * everyone who is open.
 */

ddp_hangup(ddpp, time)
ddp_specifics_t *ddpp;
{

	at_socket	socket;
	ddp_socket_t	*socketp;
	mblk_t		*m;
	mblk_t		*m2;
	queue_t		*upstreamq;

	if (!(m = allocb(0, BPRI_MED)))
	{
	    /* we are out of buffers, wait for more */
	    timeout(ddp_hangup, ddpp, HZ/4);
	    return;
	}
	m->b_datap->db_type = M_HANGUP;
	for (socket = 0, socketp = ddpp->socket0p;
	     socket < ddpp->nsockets;
	     socket++, socketp++
	    )
	    if (upstreamq = socketp->upstreamq)
	    {
		/* there is an upstream! */
		if (m2 = dupb(m))
		{
		    if (canput(upstreamq->q_next))
			putnext(upstreamq, m2);
		    else
			/* no room on the next q next q! */
			(*upstreamq->q_qinfo->qi_putp) (upstreamq, m2);
		}
		else
		{
		    /* we are out of buffers, wait for more */
		    timeout(ddp_hangup, ddpp, HZ/4);
		    break;
		}
	    }
	freemsg(m);
	return;
}

ddp_close(q, flag)
queue_t *q;
{
	ddp_socket_t *ddpp;
	mblk_t *m;
	register int s;
	extern wakeup();
 
	/*
	 * Send close notification to the Name Information Socket.
	 */
	if (SOCKET(q)->close) {
		ddpp = ddp_specifics.socket0p;
		for (;;) {
			if (ddpp[(short)2].upstreamq == NULL)
				break;
			m = allocb(sizeof(char), BPRI_HI);
			if (m) {
				m->b_datap->db_type = M_CTL;
				*m->b_wptr++ = SOCKET(q)->number;
				putnext(ddpp[(short)2].upstreamq, m);
				break;
			}
			s = splclk();
			timeout(wakeup, q, HZ/8);
			(void)sleep(q, PZERO);
			splx(s);
		}
	}
	SOCKET(q)->upstreamq = NULL;
}


/*
 *	we are at a demultiplexer queue, and would only get here only if the 
 *	upstream q was blocked when there was data for it, and it is now not.
 *	Normally data is passed on beyond the demultiplexer q's.
 */

ddp_rsrvc(q)
queue_t *q;
{
	mblk_t		*mp;

	while ((mp = getq(q)) != NULL) 
	{
	    TRACE(T_ddp, ("ddp_rsrvc: message = 0x%x\n", mp->b_datap->db_type));
	    if  (canput(q->q_next))
		putnext(q, mp);
	    else
	    {
		putbq(q, mp);
		break;
	    }
	} /* while ((mp = getq(q)) != NULL) */
}


ddp_rputq(q, m)
queue_t 	*q;
mblk_t		*m;
{
	struct iocblk 	*iocbp;
	ddp_specifics_t *ddpp;
	at_ddp_cfg_t	*cfgp;
	at_length	length;
	at_socket	socket;
	mblk_t		*m2;
	lap_ddp_t	*lap_dgp;
	lap_ddpx_t	*lap_dgxp;
	at_rtmp		*rtmpp;
	queue_t		*upstreamq;

	
	ddpp = SOCKET(q)->ddp;
	switch (m->b_datap->db_type) 
	{
	case M_DATA:
	    lap_dgxp = (lap_ddpx_t *) m->b_rptr;
	    /*----------------------------------*/
	    /* Is this a short header DataGram? */
	    /*----------------------------------*/
	    if (lap_dgxp->lap_type == AT_LAP_TYPE_DDP)
	    {
		/*------------------------------------------------------*/
		/* careful, lap_dgp is overlayed the datagram		*/
		/*------------------------------------------------------*/
		TRACE(T_ddp, ("ddp_rputq %d: short from %x\n",
			       q-queue, lap_dgxp->lap_src_node
			     )
		     );
		if  ((m->b_rptr - m->b_datap->db_base) < 
		     (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE - AT_LAP_HDR_SIZE))
		{
		    /* there is no room to store the lap header */
		    TRACE(T_ddp,
			("ddp_rputp: no room to extend short 0x%x 0x%x\n",
				m->b_rptr, m->b_datap->db_base));
		    freemsg(m);
		    break;
		}
		lap_dgp = (lap_ddp_t *) lap_dgxp;
		lap_dgxp = (lap_ddpx_t *) ((caddr_t) lap_dgp - 
					  (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE));
		m->b_rptr = ((caddr_t) lap_dgxp) + AT_LAP_HDR_SIZE;
		/* no need to check size, because lap did */
		/* do length now, since source of info will be overwritten */
		W16(lap_dgxp->length, (R16(lap_dgp->length)&0x3ff) +
					 (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE));
		/* sockets and type fields are already filled in */
		lap_dgxp->src_node = lap_dgp->lap_src_node;
		lap_dgxp->dst_node = lap_dgp->lap_dst_node;
	        W16(lap_dgxp->dst_net, ddpp->cfg.this_net);
	        W16(lap_dgxp->src_net, ddpp->cfg.this_net);
		lap_dgxp->checksum[0] = 0;
		lap_dgxp->checksum[1] = 0;
		/* lap node fields need not be filled in since they will loped*/
	    } else {
		TRACE(T_ddp, ("ddp_rputq %d: extended from %x\n",
			       q-queue, lap_dgxp->lap_src_node
			     )
		     );
	    	m->b_rptr += AT_LAP_HDR_SIZE;
		if (m->b_rptr >= m->b_wptr) {
			m2 = m->b_cont;
			freeb(m);
			m = m2;
	    		lap_dgxp = (lap_ddpx_t *) (m->b_rptr - AT_LAP_HDR_SIZE);
		}
	    }

	    /*---------------------------------------------------------*/
	    /* if frame wasn't, it is now an extended header DataGram. */
	    /*---------------------------------------------------------*/
	    if (!(    AT_DDP_SOCKET_1st_RESERVED <= lap_dgxp->dst_socket
		  &&  lap_dgxp->dst_socket <= AT_DDP_SOCKET_LAST)
	       )
	    {
		TRACE(T_ddp,("ddp_rputq: socket range error %x\n",lap_dgxp->dst_socket));
		ddpp->cfg.stats.rcv_socket_outrange++;
		freemsg(m);
		break;
	    }
	    /* if the checksum is true, then upstream wants us to calc */
	    if  (   R16(lap_dgxp->checksum)
		 && R16(lap_dgxp->checksum) != calculate_ddp_chksum(m, 0))
	    {
		ddpp->cfg.stats.rcv_checksum_errors++;
		TRACE(T_ddp,("ddp_rputq: checksum error\n"));
		freemsg(m);
		break;
	    }
	    if  ((m->b_wptr-m->b_rptr) !=
		 ((R16(lap_dgxp->length) & 0x3ff)))
	    {
		ddpp->cfg.stats.rcv_length_errors++;
		TRACE(T_ddp,("ddp_rputq: length error actual=0x%x, said=0x%x\n",
				m->b_wptr-m->b_rptr, R16(lap_dgxp->length) & 0x3ff
			    )
		     );
		freemsg(m);
		break;
	    }
	    /*
	     * demultiplex and send to proper q.
	     */
	    socket = lap_dgxp->dst_socket;
	    TRACE(T_ddp, ("ddp_rputq: to socket = 0x%x\n", socket));
	    if (socket == AT_DDP_SOCKET_RTMP && lap_dgxp->type == AT_DDP_TYPE_RTMP)
	    {
		/* An RTMP broadcast, get the source for a_bridge and Net for
		 * this_network
		 */
		rtmpp = (at_rtmp *) lap_dgxp->data;
		ddpp->cfg.this_net = R16(rtmpp->at_rtmp_this_net);
		if (rtmpp->at_rtmp_id_length  == 8)
		{
		    ddpp->cfg.a_bridge = rtmpp->at_rtmp_id[0];
		    untimeout(ddp_age_bridge,ddpp,90*HZ);
		    timeout(ddp_age_bridge,ddpp,90*HZ);

		    /*
		     *	Try and tell NBPD about it
		     */

		    m2 = dupmsg(m);
		    if (m2) {
	    		if (ddpp->socket0p && (upstreamq = ddpp->socket0p[AT_DDP_SOCKET_NIS].upstreamq)!=NULL)
	    		{
			    /* there is an upstream! */
			    if (canput(upstreamq->q_next))
			        putnext(upstreamq, m2);
			    else
			    /* no room on the next q next q! */
			    putq(upstreamq, m2);
			}
			else
			{
			    freemsg(m2);
			}
		    }
		}
		else
		{
		    TRACE(T_ddp, ("ddp_rputq: rtmp_length, 0x%x, not equal 8\n",
				   rtmpp->at_rtmp_id_length
				 )
			 );
		}
	    }
	    if (ddpp->socket0p && (upstreamq = ddpp->socket0p[socket].upstreamq)!=NULL)
	    {
		/* there is an upstream! */
		if (canput(upstreamq->q_next))
		    putnext(upstreamq, m);
		else
		    /* no room on the next q next q! */
		    putq(upstreamq, m);
	    }
	    else
	    {
		/* there is not an upstream! either gone away, or we confused */
		TRACE(T_ddp, ("ddp_rputq: no upstreamq\n"));
		freemsg(m);
	    }
	    break;
	case M_HANGUP:
	    /*
	     * well lap has brought the network down, dup and send it upstream to 
	     * everyone who is open. Must get rid of this queue, LAP has gotten rid
	     * of its pair at the lap level. This is ok since queuerun has already
	     * removed this q from the schedule list.
	     */
	    flushq(q, FLUSHALL);
	    freeq(q);
	    freemsg(m);
	    ddp_hangup(ddpp, 0);
	    break;
	case M_FLUSH:
	    /*
	     * if flush is for write queue then flush it.  The first
	     * byte of the message contains the flush flags.
	     */
	    printf("ddp_rputq: should not be getting M_FLUSH\n");
	    freemsg(m);
	    break;
	default:
	    TRACE(T_ddp, ("ddp_rputq: passing something through\n"));
	    m->b_wptr -= sizeof(ddp_tags_t);
	    socket = DDP_IOCTAGS(m)->socket;
	    if (socket >= ddpp->nsockets)
	    {
		ddpp->cfg.stats.socket_unregistered++;
		return;
	    }
	    /*
	     * demultiplex and send to proper q. Note: no canput since if one 
	     * upstream q is blocked it would stop all read traffic!
	     */
	    upstreamq = ddpp->socket0p[(unsigned short)socket].upstreamq;
	    if (upstreamq)
		/* there is an upstream! */
		if (canput(upstreamq->q_next))
		    putnext(upstreamq, m);
		else
		    /* no room on the next q next q! */
		    (*upstreamq->q_qinfo->qi_putp) (upstreamq, m);
	    else
		/* there is not an upstream! either gone away, or we confused */
		freemsg(m);
	    break;
	} /* of switch (m->b_datap->db_type) */
}


ddp_wputq(q, m)
queue_t 	*q;
mblk_t		*m;
{
	struct iocblk 	*iocbp;
	ddp_specifics_t *ddpp;
	at_ddp_cfg_t	*cfgp;
	int		length;
	at_ddp_t	*dgp;
	lap_ddp_t	*lap_dgp;
	lap_ddpx_t	*lap_dgxp;
	mblk_t		*m2;
	lap_ddpx_t	*m2_lap_dgxp;

	
	ddpp = SOCKET(q)->ddp;
	switch (m->b_datap->db_type) 
	{
	case M_DATA:
	    dgp = (at_ddp_t *) m->b_rptr;
	    if (!(    AT_DDP_SOCKET_1st_RESERVED <= dgp->dst_socket
		  &&  dgp->dst_socket <= (AT_DDP_SOCKET_LAST+1))
	       )
	    {
		data_error(ENOTSOCK, m, q);
		break;
	    }
	    if (dgp->dst_node == 0)
	    {
		data_error(EADDRNOTAVAIL, m, q);
		break;
	    }
	    /*----------------------------------*/
	    /* Is this a short header DataGram? */
	    /*----------------------------------*/
	    if (R16(dgp->dst_net) == 0 || R16(dgp->dst_net) == ddpp->cfg.this_net)
	    {
		/*------------------------------------------------------*/
		/* careful, lap_dgp is overlayed the datagram		*/
		/*------------------------------------------------------*/
		TRACE(T_ddp, ("ddp_wputq: convert to short\n"));
		lap_dgp = (lap_ddp_t *) ((caddr_t) dgp +
		      (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE - AT_LAP_HDR_SIZE));
		m->b_rptr = (caddr_t) lap_dgp;
		/* LAP cannot check the length becasue < max LAP */
		if  ((length = msgdsize(m)) >
			 (AT_LAP_SIZE - (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE)))
		{
		    /* the packet is too large, barf */
		    data_error(EMSGSIZE, m, q);
		    break;
		}
		/* fill in this field first, since source of is overwritten */
		lap_dgp->lap_dst_node = dgp->dst_node;
		lap_dgp->src_socket = SOCKET(q)->number;
		W16(lap_dgp->length, length - AT_LAP_HDR_SIZE);
		lap_dgp->lap_type = AT_LAP_TYPE_DDP;
		TRACE(T_ddp, ("ddp_wputq: from socket %x to %x %x\n",
			       SOCKET(q)->number, lap_dgp->lap_dst_node, dgp->dst_socket
			     )
		     );
	    }

	    /*--------------------------------------*/
	    /* this is an extended header DataGram. */
	    /*--------------------------------------*/
	    else  /* of if (dgp->dst_net == 0) */
	    {
		/*------------------------------------------------------*/
		/* careful, lap_dgxp is overlayed the datagram		*/
		/*------------------------------------------------------*/
		/* LAP will check the length */
		if  ((m->b_rptr - m->b_datap->db_base) < AT_LAP_HDR_SIZE)
		{
		    TRACE(T_ddp, ("ddp_wputq: no room for lap header\n"));
		    /*
		     * there is no room to store the lap header try and allocate
		     * space for it, fill in the header first. NOTE: the lap header
		     * is now not contiguous with the ddp header, that is why we
		     * have the two pointers here m2_... and .... LAP will "pullup"
		     */
		    m2 = allocb(AT_LAP_HDR_SIZE, BPRI_MED);
		    if (m2 == NULL) {
		    	ddpp->cfg.stats.tag_room_errors++;
		    	data_error(ERANGE, m, q);
		    	break;
		    }
		    m2->b_wptr += AT_LAP_HDR_SIZE;
		    m2->b_cont = m;
		    m = m2;
		    m2_lap_dgxp = (lap_ddpx_t *) m->b_rptr;
		    m2_lap_dgxp->lap_type = AT_LAP_TYPE_DDP_X;
		    /*----------------------------------------*/
		    /* If we have a bridge around, we want to */
		    /* send extended header DataGrams to it.  */
		    /*----------------------------------------*/
		    if (ddpp->cfg.a_bridge != 0)
		    {
		        m2_lap_dgxp->lap_dst_node = ddpp->cfg.a_bridge;
		    } else
		    /*-----------------------------------------------------*/
		    /* No bridge, but this is an extended header DataGram, */
		    /* so let's try to send it out on the local net.       */
		    /*-----------------------------------------------------*/
		    {
		        m2_lap_dgxp->lap_dst_node = lap_dgxp->dst_node;
		    }
		    /* already checked dst_socket */
		    lap_dgxp = (lap_ddpx_t *) (((caddr_t) dgp) - AT_LAP_HDR_SIZE);
		} else {
		    lap_dgxp = (lap_ddpx_t *) (((caddr_t) dgp) - AT_LAP_HDR_SIZE);
		    m->b_rptr = (caddr_t) lap_dgxp;
		    lap_dgxp->lap_type = AT_LAP_TYPE_DDP_X;
		    /*----------------------------------------*/
		    /* If we have a bridge around, we want to */
		    /* send extended header DataGrams to it.  */
		    /*----------------------------------------*/
		    if (ddpp->cfg.a_bridge != 0)
		    {
		        lap_dgxp->lap_dst_node = ddpp->cfg.a_bridge;
		    } else
		    /*-----------------------------------------------------*/
		    /* No bridge, but this is an extended header DataGram, */
		    /* so let's try to send it out on the local net.       */
		    /*-----------------------------------------------------*/
		    {
		        lap_dgxp->lap_dst_node = lap_dgxp->dst_node;
		    }
		    /* already checked dst_socket */
		}
		if  ((length = msgdsize(m)) >
			 (AT_LAP_SIZE - (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE)))
		{
		    /* the packet is too large, barf */
		    data_error(EMSGSIZE, m, q);
		    break;
		}
		lap_dgxp->src_node = ddpp->cfg.this_node;
		lap_dgxp->src_socket = SOCKET(q)->number;
		/* already checked dst_node */
	        W16(lap_dgxp->src_net, ddpp->cfg.this_net);
		/* if the checksum is true, then upstream wants us to calc */
		W16(lap_dgxp->length, 0x3ff & (length - AT_LAP_HDR_SIZE));
                if  (R16(lap_dgxp->checksum))
		{
		     W16(lap_dgxp->checksum, calculate_ddp_chksum(m, AT_LAP_HDR_SIZE));
		}
		TRACE(T_ddp, ("ddp_wputq: from socket %x to %x %x %x\n",
			       SOCKET(q)->number, R16(lap_dgxp->dst_net),
			       lap_dgxp->dst_node, lap_dgxp->dst_socket
			     )
		     );
	    }	/* of else  if (dgp->dst_net == 0) */
	    if	(ddpp->downstreamq != NULL) {
	        (*ddpp->downstreamq->q_qinfo->qi_putp) (ddpp->downstreamq, m);
	    } else {
		data_error(ENETDOWN, m, q);
	    }
	    break;
	case M_IOCTL:
	    iocbp = (struct iocblk *) m->b_rptr;
	    switch (iocbp->ioc_cmd) 
	    {
	    default:
		/* I don't know what it is, so pass it on and leave trail */
	    case AT_SYNC:
		if  (m->b_datap->db_lim - m->b_wptr < sizeof(ddp_tags_t))
		{
		    /* there is no room to store the multiplexer tag */
		    ddpp->cfg.stats.tag_room_errors++;
		    data_error(ERANGE, m, q);
		    break;
		}
		DDP_IOCTAGS(m)->socket = SOCKET(q)->number;
		m->b_wptr += sizeof(ddp_tags_t);
		if	(ddpp->downstreamq != NULL)
		    (*ddpp->downstreamq->q_qinfo->qi_putp) (ddpp->downstreamq, m);
		else
		    ioc_ack(ENETDOWN, m, q);
		break;
	    case  AT_DDP_GET_CFG:
		if  (m->b_cont)
		{
		    iocbp->ioc_count = 0;
		    freemsg(m->b_cont);
		    m->b_cont = NULL;
		}
		if ((m->b_cont = allocb(sizeof(at_ddp_cfg_t), BPRI_MED)) == NULL)
		{
		    ioc_ack(ENOBUFS, m, q);
		    break;
		}
		cfgp = ((at_ddp_cfg_t *) m->b_cont->b_rptr);
		*cfgp = ddpp->cfg;
		m->b_cont->b_wptr += sizeof(at_ddp_cfg_t);
		iocbp->ioc_count = sizeof(at_ddp_cfg_t);
		m->b_datap->db_type = M_IOCACK;
		qreply(q, m);
		break;
	    case AT_DDP_IS_OPEN:
		if (iocbp->ioc_count != sizeof(char)) {
		    ioc_ack(EINVAL, m, q);
		    break;
		}
		{
			int dev;
	
			dev = *(unsigned char *)m->b_cont->b_rptr;
			if (ddp_specifics.socket0p[(unsigned short)dev].upstreamq == NULL) {
		    		ioc_ack(ENODEV, m, q);
		    		break;
			}
			ddp_specifics.socket0p[(unsigned short)dev].close = 1;
			m->b_datap->db_type = M_IOCACK;
			qreply(q, m);
		}
		break;
	    }   /* of switch (iocbp->ioc_cmd) */
	    break;
	case M_FLUSH:
	    /*
	     * if flush is for write queue then flush it.
	     */
	    if (*m->b_rptr & FLUSHW)
	    {
		flushq(q, FLUSHALL);
		*m->b_rptr &= ~FLUSHW;
	    }
	    if (*m->b_rptr & FLUSHR)
	    {
		flushq(RD(q), FLUSHALL);
	    }
	    putnext(RD(q), m);
	    break;
	default:
	    if  (m->b_wptr - m->b_datap->db_lim < sizeof(ddp_tags_t))
	    {
		/* there is no room to store the multiplexer tag */
		ddpp->cfg.stats.tag_room_errors++;
		data_error(ERANGE, m, q);
		break;
	    }
	    DDP_IOCTAGS(m)->socket = SOCKET(q)->number;
	    m->b_wptr += sizeof(ddp_tags_t);
	    if	(ddpp->downstreamq != NULL)
	        (*ddpp->downstreamq->q_qinfo->qi_putp) (ddpp->downstreamq, m);
	    else
		data_error(ENETDOWN, m, q);
	    break;
	} /* of switch (m->b_datap->db_type) */
}
