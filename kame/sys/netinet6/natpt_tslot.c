/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: natpt_tslot.c,v 1.2 1999/12/25 02:35:32 fujisawa Exp $
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/systm.h>

#include <net/if.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/udp.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ip6.h>
#if !defined(__NetBSD__) && (!defined(__FreeBSD__) || (__FreeBSD__ < 3))
#include <netinet6/tcp6.h>
#endif

#include <netinet6/natpt_defs.h>
#include <netinet6/natpt_list.h>
#include <netinet6/natpt_soctl.h>
#include <netinet6/natpt_var.h>


/*
 *
 */

static	Cell	*_insideHash [NATPT_MAXHASH];
static	Cell	*_outsideHash[NATPT_MAXHASH];

static	Cell	*tSlotEntry;
static	int	 tSlotEntryMax;
static	int	 tSlotEntryUsed;

static	time_t	 tSlotTimer;
static	time_t	 maxTTLany;
static	time_t	 maxTTLicmp;
static	time_t	 maxTTLudp;
static	time_t	 maxTTLtcp;

static	time_t	 _natpt_TCPT_2MSL;
static	time_t	 _natpt_tcp_maxidle;

extern	struct in6_addr	 faith_prefix;
extern	struct in6_addr	 faith_prefixmask;


static struct _pat	*fillupOutgoingV6local	__P((struct _cSlot *, struct _cv *, struct _pat *));
static struct _pat	*fillupOutgoingV6Remote	__P((struct _cSlot *, struct _cv *, struct _pat *));

static struct _tSlot	*registTSlotEntry	__P((struct _tSlot *));
static void	 _expireTSlot			__P((void *));
static void	 _expireTSlotEntry		__P((struct timeval *));
static void	 _removeTSlotEntry		__P((struct _cell *, struct _cell *));
static int	 _removeHash			__P((struct _cell *(*table)[], int, caddr_t));

static int	 _hash_ip4			__P((struct _cv *));
static int	 _hash_ip6			__P((struct _cv *));
static int	 _hash_pat4			__P((struct _pat *));
static int	 _hash_pat6			__P((struct _pat *));
static int	 _hash_sockaddr4		__P((struct sockaddr_in *));
static int	 _hash_sockaddr6		__P((struct sockaddr_in6 *));
static int	 _hash_pjw			__P((u_char *, int));


#if defined(__FreeBSD__) && __FreeBSD__ >= 3
static	MALLOC_DEFINE(M_NATPT, "NATPT", "Network Address Translation - Protocol Translation");
#endif


/*
 *
 */

struct _tSlot	*
lookingForIncomingV4Hash(struct _cv *cv)
{
    register	Cell		*p;
    register	struct _tSlot	*ats;
    register	struct ip	*ip4;

    int		hv = _hash_ip4(cv);

    for (p = _outsideHash[hv]; p; p = CDR(p))
    {
	ats = (struct _tSlot *)CAR(p);

	if ((ats->remote.ip_p != IPPROTO_IPV4)
	    || (cv->ip_payload != ats->ip_payload))			continue;

	if ((cv->ip_payload == IPPROTO_TCP)
	    || (cv->ip_payload == IPPROTO_UDP))
	{
	    if (cv->_payload._tcp4->th_sport!= ats->remote.dport)	continue;
	    if (cv->_payload._tcp4->th_dport!= ats->remote.sport)	continue;
	}
	
	ip4 = cv->_ip._ip4;
	if ((ip4->ip_src.s_addr == ats->remote.dst.u.in4.s_addr)
	    && (ip4->ip_dst.s_addr == ats->remote.src.u.in4.s_addr))
	    return (ats);
    }

    return (NULL);
}


struct _tSlot	*
lookingForOutgoingV4Hash(struct _cv *cv)
{
    register	Cell		*p;
    register	struct _tSlot	*ats;
    register	struct ip	*ip4;

    int		hv = _hash_ip4(cv);

    for (p = _insideHash[hv]; p; p = CDR(p))
    {
	ats = (struct _tSlot *)CAR(p);

	if ((ats->local.ip_p != IPPROTO_IPV4)
	    || (cv->ip_payload != ats->ip_payload))	continue;

	if ((cv->ip_payload == IPPROTO_TCP)
	    || (cv->ip_payload == IPPROTO_UDP))
	{
	    if (cv->_payload._tcp4->th_sport != ats->local.dport)	continue;
	    if (cv->_payload._tcp4->th_dport != ats->local.sport)	continue;
	}
	
	ip4 = cv->_ip._ip4;
	if ((ip4->ip_src.s_addr == ats->local.dst.u.in4.s_addr)
	    && (ip4->ip_dst.s_addr == ats->local.src.u.in4.s_addr))
	    return (ats);
    }

    return (NULL);
}


struct _tSlot	*
lookingForIncomingV6Hash(struct _cv *cv)
{
    register	Cell		*p;
    register	struct _tSlot	*ats;
    register	struct ip6_hdr	*ip6;

    int		hv = _hash_ip6(cv);
    
    for (p = _outsideHash[hv]; p; p = CDR(p))
    {
	ats = (struct _tSlot *)CAR(p);

	if ((ats->remote.ip_p != IPPROTO_IPV6)
	    || (cv->ip_payload != ats->ip_payload))	continue;

	if ((cv->ip_payload == IPPROTO_TCP)
	    || (cv->ip_payload == IPPROTO_UDP))
	{
	    if (cv->_payload._tcp6->th_sport != ats->remote.dport)	continue;
	    if (cv->_payload._tcp6->th_dport != ats->remote.sport)	continue;
	}

	ip6 = cv->_ip._ip6;
	if ((IN6_ARE_ADDR_EQUAL(&ip6->ip6_src, &ats->remote.dst.u.in6))
	    && (IN6_ARE_ADDR_EQUAL(&ip6->ip6_dst, &ats->remote.src.u.in6)))
	    return (ats);
    }

    return (NULL);
}


struct _tSlot	*
lookingForOutgoingV6Hash(struct _cv *cv)
{
    register	Cell		*p;
    register	struct _tSlot	*ats;
    register	struct ip6_hdr	*ip6;

    int		hv = _hash_ip6(cv);
    
    for (p = _insideHash[hv]; p; p = CDR(p))
    {
	ats = (struct _tSlot *)CAR(p);

	if ((ats->local.ip_p != IPPROTO_IPV6)
	    || (cv->ip_payload != ats->ip_payload))			continue;

	if ((cv->ip_payload == IPPROTO_TCP)
	    || (cv->ip_payload == IPPROTO_UDP))
	{
	    if (cv->_payload._tcp6->th_sport != ats->local.dport)	continue;
	    if (cv->_payload._tcp6->th_dport != ats->local.sport)	continue;
	}

	ip6 = cv->_ip._ip6;
	if ((IN6_ARE_ADDR_EQUAL(&ip6->ip6_src, &ats->local.dst.u.in6))
	    && (IN6_ARE_ADDR_EQUAL(&ip6->ip6_dst, &ats->local.src.u.in6)))
	    return (ats);
    }

    return (NULL);
}


struct _tSlot	*
internIncomingV4Hash(int sess, struct _cSlot *acs, struct _cv *cv4)
{
    int			 s, hv4, hv6;
    struct _pat		*local, *remote;
    struct _tSlot	*ats;

    MALLOC(ats, struct _tSlot *, sizeof(struct _tSlot), M_TEMP, M_NOWAIT);
    if (ats == NULL)
    {
	printf("ENOBUFS in internIncomingV4Hash %d\n", __LINE__);
	return (NULL);
    }

    bzero(ats, sizeof(struct _tSlot));

    local = &ats->local;
    local->ip_p = IPPROTO_IPV6;
    if ((cv4->ip_payload == IPPROTO_TCP)
	|| (cv4->ip_payload == IPPROTO_UDP))
    {
	local->sport = cv4->_payload._tcp4->th_sport;
	local->dport = cv4->_payload._tcp4->th_dport;
    }
    local->src.u.in6.s6_addr32[3] = cv4->_ip._ip4->ip_src.s_addr;
    local->dst.u.in6 = acs->local.in6;
    local->src.sa_family = local->dst.sa_family = AF_INET6;

    remote = &ats->remote;
    remote->ip_p = IPPROTO_IPV4;
    if ((cv4->ip_payload == IPPROTO_TCP)
	|| (cv4->ip_payload == IPPROTO_UDP))
    {
	remote->sport = cv4->_payload._tcp4->th_dport;
	remote->dport = cv4->_payload._tcp4->th_sport;
    }
    remote->src.u.in4 = acs->remote.in4;
    remote->dst.u.in4 = cv4->_ip._ip4->ip_src;
    remote->src.sa_family = remote->dst.sa_family = AF_INET;

    ats->ip_payload = cv4->ip_payload;
    ats->session = sess;
    registTSlotEntry(ats);						/* XXX	*/

    hv6 = _hash_pat6(local);
    hv4 = _hash_pat4(remote);

    s = splnet();
    LST_hookup_list(&_insideHash [hv6], ats);
    LST_hookup_list(&_outsideHash[hv4], ats);
    splx(s);

    return (ats);
}


struct _tSlot	*
internOutgoingV4Hash(int sess, struct _cSlot *acs, struct _cv *cv4)
{
    int			 s, hv4, hv6;
    struct _pat		*local, *remote;
    struct _tSlot	*ats;

    MALLOC(ats, struct _tSlot *, sizeof(struct _tSlot), M_TEMP, M_NOWAIT);
    if (ats == NULL)
    {
	printf("ENOBUFS in internOutgoingV4Hash %d\n", __LINE__);
	return (NULL);
    }

    bzero(ats, sizeof(struct _tSlot));

    local = &ats->local;
    local->ip_p = IPPROTO_IPV4;
    local->src.sa_family = local->dst.sa_family = AF_INET;
    if ((cv4->ip_payload == IPPROTO_TCP)
	|| (cv4->ip_payload == IPPROTO_UDP))
    {
	local->sport = cv4->_payload._tcp4->th_dport;
	local->dport = cv4->_payload._tcp4->th_sport;
    }

    if (acs->c.flags == NATPT_FAITH)
    {
	local->src.u.in4 = cv4->_ip._ip4->ip_dst;
	local->dst.u.in4 = cv4->_ip._ip4->ip_src;
    }
    else
    {
	local->src.u.in4 = acs->local.in4;
	local->dst.u.in4 = cv4->_ip._ip4->ip_src;
    }

    remote = &ats->remote;
    remote->ip_p = IPPROTO_IPV6;
    remote->src.sa_family = remote->dst.sa_family = AF_INET6;
    if ((cv4->ip_payload == IPPROTO_TCP)
	|| (cv4->ip_payload == IPPROTO_UDP))
    {
	remote->sport = cv4->_payload._tcp4->th_sport;
	remote->dport = cv4->_payload._tcp4->th_dport;
    }

    if (acs->c.flags == NATPT_FAITH)
    {
	struct in6_ifaddr	*ia6;

	remote->dst.u.in6.s6_addr32[0] = faith_prefix.s6_addr32[0];
	remote->dst.u.in6.s6_addr32[1] = faith_prefix.s6_addr32[1];
	remote->dst.u.in6.s6_addr32[3] = cv4->_ip._ip4->ip_dst.s_addr;

	ia6 = in6_ifawithscope(natpt_ip6src, &remote->dst.u.in6);
	remote->src.u.in6 = ia6->ia_addr.sin6_addr;
    }
    else
    {
	remote->src.u.in6.s6_addr32[3] = cv4->_ip._ip4->ip_src.s_addr;
	remote->dst.u.in6 = acs->remote.in6;
    }

    ats->ip_payload = cv4->ip_payload;
    ats->session = sess;
    registTSlotEntry(ats);						/* XXX	*/

    hv4 = _hash_pat4(local);
    hv6 = _hash_pat6(remote);

    s = splnet();
    LST_hookup_list(&_insideHash [hv4], ats);
    LST_hookup_list(&_outsideHash[hv6], ats);
    splx(s);

    return (ats);
}


struct _tSlot	*
internIncomingV6Hash(int sess, struct _cSlot *acs, struct _cv *cv6)
{
    int			 s, hv4, hv6;
    struct _pat		*local, *remote;
    struct _tSlot	*ats;

    MALLOC(ats, struct _tSlot *, sizeof(struct _tSlot), M_TEMP, M_NOWAIT);
    if (ats == NULL)
    {
	printf("ENOBUFS in internIncomingV6Hash %d\n", __LINE__);
	return (NULL);
    }
    
    bzero(ats, sizeof(struct _tSlot));

    local = &ats->local;
    local->ip_p = IPPROTO_IPV4;
    if ((cv6->ip_payload == IPPROTO_TCP)
	|| (cv6->ip_payload == IPPROTO_UDP))
    {
	local->sport = cv6->_payload._tcp6->th_sport;
	local->dport = cv6->_payload._tcp6->th_dport;
    }
    local->src.u.in4 = acs->local.in4;
    local->dst.u.in4.s_addr = cv6->_ip._ip6->ip6_dst.s6_addr32[3];
    local->src.sa_family = local->dst.sa_family = AF_INET;

    remote = &ats->remote;
    remote->ip_p = IPPROTO_IPV6;
    if ((cv6->ip_payload == IPPROTO_TCP)
	|| (cv6->ip_payload == IPPROTO_UDP))
    {
	remote->sport = cv6->_payload._tcp6->th_dport;
	remote->dport = cv6->_payload._tcp6->th_sport;
    }
    remote->src.u.in6 = cv6->_ip._ip6->ip6_dst;
    remote->dst.u.in6 = acs->remote.in6;
    remote->src.sa_family = remote->dst.sa_family = AF_INET6;

    ats->ip_payload = cv6->ip_payload;
    ats->session = sess;
    registTSlotEntry(ats);						/* XXX	*/

    hv6 = _hash_pat6(remote);
    hv4 = _hash_pat4(local);

    s = splnet();
    LST_hookup_list(&_outsideHash[hv6], ats);
    LST_hookup_list(&_insideHash [hv4], ats);
    splx(s);

    return (ats);
}


struct _tSlot	*
internOutgoingV6Hash(int sess, struct _cSlot *acs, struct _cv *cv6)
{
    int			 s, hv4, hv6;
    struct _pat		*local, *remote;
    struct _tSlot	*ats;

    natpt_logIp6(LOG_DEBUG, cv6->_ip._ip6);

    MALLOC(ats, struct _tSlot *, sizeof(struct _tSlot), M_TEMP, M_NOWAIT);
    if (ats == NULL)
    {
	printf("ENOBUFS in internOutgoingV6Hash %d\n", __LINE__);
	return (NULL);
    }
    
    bzero(ats, sizeof(struct _tSlot));

    local = fillupOutgoingV6local (acs, cv6, &ats->local);
    if ((remote = fillupOutgoingV6Remote(acs, cv6, &ats->remote)) == 0)
    {
	FREE(ats, M_TEMP);
	return (NULL);
    }

    ats->ip_payload = cv6->ip_payload;
    ats->session = sess;
    registTSlotEntry(ats);						/* XXX	*/

    hv6 = _hash_pat6(local);
    hv4 = _hash_pat4(remote);

    s = splnet();
    LST_hookup_list(&_insideHash [hv6], ats);
    LST_hookup_list(&_outsideHash[hv4], ats);
    splx(s);

    return (ats);
}


struct _tSlot *
checkTraceroute6Return(struct _cv *cv4)
{
    int			 hv;
    Cell		*p;
    struct ip		*icmpip4;
    struct udphdr	*icmpudp4;
    struct sockaddr_in	 src, dst;
    struct _tSlot	*ats;

    if ((cv4->ip_payload != IPPROTO_ICMP)
	|| ((cv4->_payload._icmp4->icmp_type != ICMP_UNREACH)
	    && (cv4->_payload._icmp4->icmp_type != ICMP_TIMXCEED)))
	return (NULL);

    icmpip4 = &cv4->_payload._icmp4->icmp_ip;
    if (icmpip4->ip_p != IPPROTO_UDP)
	return (NULL);

#if defined(fixSuMiReICMPBug)
    icmpip4->ip_src.s_addr = ICMPSRC;					/* XXX	*/
#endif

    icmpudp4 = (struct udphdr *)((caddr_t)icmpip4 + (icmpip4->ip_hl << 2));

    bzero(&src, sizeof(struct sockaddr_in));
    bzero(&dst, sizeof(struct sockaddr_in));
    src.sin_addr = icmpip4->ip_src;
    src.sin_port = icmpudp4->uh_sport;
    dst.sin_addr = icmpip4->ip_dst;
    dst.sin_port = icmpudp4->uh_dport;
    hv = ((_hash_sockaddr4(&src) + _hash_sockaddr4(&dst)) % NATPT_MAXHASH);
    for (p = _outsideHash[hv]; p; p = CDR(p))
    {
	ats = (struct _tSlot *)CAR(p);

	if (ats->remote.ip_p != IPPROTO_IPV4)				continue;
	if (ats->ip_payload  != IPPROTO_UDP)				continue;

	if (icmpip4->ip_src.s_addr != ats->remote.src.u.in4.s_addr)	continue;
	if (icmpip4->ip_dst.s_addr != ats->remote.dst.u.in4.s_addr)	continue;

	if (icmpudp4->uh_sport != ats->remote.sport)			continue;
	if (icmpudp4->uh_dport != ats->remote.dport)			continue;

	cv4->flags |= NATPT_TRACEROUTE;
	return (ats);
    }

    return (NULL);
}


static struct _pat *
fillupOutgoingV6local(struct _cSlot *acs, struct _cv *cv6, struct _pat *local)
{
    local->ip_p = IPPROTO_IPV6;
    local->src.sa_family = local->dst.sa_family = AF_INET6;
    local->src.u.in6 = cv6->_ip._ip6->ip6_dst;
    local->dst.u.in6 = cv6->_ip._ip6->ip6_src;

    if ((cv6->ip_payload == IPPROTO_TCP)
	|| (cv6->ip_payload == IPPROTO_UDP))
    {
	local->sport = cv6->_payload._tcp6->th_dport;
	local->dport = cv6->_payload._tcp6->th_sport;
    }

    return (local);
}


static struct _pat *
fillupOutgoingV6Remote(struct _cSlot *acs, struct _cv *cv6, struct _pat *remote)
{
    remote->ip_p = IPPROTO_IPV4;
    remote->src.sa_family = remote->dst.sa_family = AF_INET;
    remote->src.u.in4 = acs->remote.in4;
    remote->dst.u.in4.s_addr = cv6->_ip._ip6->ip6_dst.s6_addr32[3];

    if ((cv6->ip_payload == IPPROTO_TCP)
	|| (cv6->ip_payload == IPPROTO_UDP))
    {
	remote->sport = cv6->_payload._tcp6->th_sport;
	remote->dport = cv6->_payload._tcp6->th_dport;

	if (acs->c.flags == NATPT_DYNAMIC)
	{
	    int		firsttime = 0;
	    struct _pat	pata;	/* pata.{s,d}port hold network byte order	*/

	    if (acs->aux->cport == 0)
		acs->aux->cport = acs->sport - 1;

	    bzero(&pata, sizeof(pata));
	    pata.ip_p = IPPROTO_IPV4;
	    pata.src.u.in4 = acs->remote.in4;
	    pata.dst.u.in4.s_addr = cv6->_ip._ip6->ip6_dst.s6_addr32[3];
	    pata.dport = remote->dport;

	    for (;;)
	    {
		while (++acs->aux->cport <= acs->eport)
		{
		    pata.sport = htons(acs->aux->cport);
		    if (_outsideHash[_hash_pat4(&pata)] == NULL)
			goto found;
		}

		if (firsttime == 0)
		    firsttime++,
		    acs->aux->cport = acs->sport - 1;
		else
		    return (NULL);
	    }

	found:;
	    remote->sport = pata.sport;
	}
    }

    return (remote);
}


static struct _tSlot *
registTSlotEntry(struct _tSlot *ats)
{
    int			 s;
    Cell		*p;
    struct timeval	 atv;

    if (tSlotEntryUsed >= tSlotEntryMax)
	return (NULL);

    tSlotEntryUsed++;

    microtime(&atv);
    ats->tstamp = atv.tv_sec;

    p = LST_cons(ats, NULL);

    s = splnet();

    if (tSlotEntry == NULL)
	tSlotEntry = p;
    else
	CDR(p) = tSlotEntry, tSlotEntry = p;

    splx(s);

    return (ats);
}


/*
 *
 */

static void
_expireTSlot(void *ignored_arg)
{
    struct timeval	atv;

    timeout(_expireTSlot, (caddr_t)0, tSlotTimer);
    microtime(&atv);

    _expireTSlotEntry(&atv);
}


static void
_expireTSlotEntry(struct timeval *atv)
{
    struct _cell	*p0, *p1, *q;
    struct _tSlot	*tsl;

    p0 = tSlotEntry;
    q  = NULL;
    while (p0)
    {
	tsl = (struct _tSlot *)CAR(p0);
	p1  = CDR(p0);

	switch (tsl->ip_payload)
	{
	  case IPPROTO_ICMP:
	    if ((atv->tv_sec - tsl->tstamp) >= maxTTLicmp)
		_removeTSlotEntry(p0, q);
	    break;

	  case IPPROTO_UDP:
	    if ((atv->tv_sec - tsl->tstamp) >= maxTTLudp)
		_removeTSlotEntry(p0, q);
	    break;

	  case IPPROTO_TCP:
	    switch (tsl->suit.tcp->_state)
	    {
	      case TCPS_CLOSED:
		if ((atv->tv_sec - tsl->tstamp) >= _natpt_TCPT_2MSL)
		    _removeTSlotEntry(p0, q);
		break;

	      case TCPS_SYN_SENT:
	      case TCPS_SYN_RECEIVED:
		if ((atv->tv_sec - tsl->tstamp) >= _natpt_tcp_maxidle)
		    _removeTSlotEntry(p0, q);
		break;

	      case TCPS_ESTABLISHED:
		if ((atv->tv_sec - tsl->tstamp) >= maxTTLtcp)
		    _removeTSlotEntry(p0, q);
		break;

	      case TCPS_FIN_WAIT_1:
	      case TCPS_FIN_WAIT_2:
		if ((atv->tv_sec - tsl->tstamp) >= _natpt_tcp_maxidle)
		    _removeTSlotEntry(p0, q);
		break;
		
	      case TCPS_TIME_WAIT:
		if ((atv->tv_sec - tsl->tstamp) >= _natpt_TCPT_2MSL)
		    _removeTSlotEntry(p0, q);
		break;

	      default:
		if ((atv->tv_sec - tsl->tstamp) >= maxTTLtcp)
		    _removeTSlotEntry(p0, q);
		break;
	    }
	    break;

	  default:
	    if ((atv->tv_sec - tsl->tstamp) >= maxTTLany)
		_removeTSlotEntry(p0, q);
	    break;
	}

	if (CAR(p0) != CELL_FREE_MARKER)		/* p0 may not removed	*/
	    q  = p0;

	p0 = p1;
    }
}


static void
_removeTSlotEntry(struct _cell *p, struct _cell *q)
{
    int			 s;
    int			 hvin, hvout;
    struct _tSlot	*tsl = (struct _tSlot *)CAR(p);

    if ((tsl->ip_payload == IPPROTO_TCP)
	&& (tsl->suit.tcp != NULL))
    {
	FREE(tsl->suit.tcp, M_NATPT);
    }

    if (tsl->local.ip_p == IPPROTO_IPV4)
	hvin = _hash_pat4(&tsl->local);
    else
	hvin = _hash_pat6(&tsl->local);

    if (tsl->remote.ip_p == IPPROTO_IPV4)
	hvout = _hash_pat4(&tsl->remote);
    else
	hvout = _hash_pat6(&tsl->remote);

    s = splnet();

    _removeHash(&_insideHash,  hvin,  (caddr_t)tsl);
    _removeHash(&_outsideHash, hvout, (caddr_t)tsl);
    
    if (q != NULL)
	CDR(q) = CDR(p);
    else
	tSlotEntry = CDR(p);

    splx(s);

    LST_free(p);
    FREE(tsl, M_NATPT);

    tSlotEntryUsed--;
}


static	int
_removeHash(Cell *(*table)[], int hv, caddr_t node)
{
    register	Cell	*p, *q;

    if ((p = (*table)[hv]) == NULL)
	return (0);

    if (CDR(p) == NULL)
    {
	if (CAR(p) == (Cell *)node)
	{
	    LST_free(p);
	    (*table)[hv] = NULL;
	}
	return (0);
    }

    for (p = (*table)[hv], q = NULL; p; q = p, p = CDR(p))
    {
	if (CAR(p) != (Cell *)node)
	    continue;

	if (q == NULL)
	    (*table)[hv] = CDR(p);
	else
	    CDR(q) = CDR(p);

	LST_free(p);
	return (0);
    }

    return (0);
}


/*
 *
 */

static int
_hash_ip4(struct _cv *cv)
{
    struct ip		*ip;
    struct sockaddr_in	 src, dst;

    bzero(&src, sizeof(struct sockaddr_in));
    bzero(&dst, sizeof(struct sockaddr_in));

    ip = cv->_ip._ip4;
    src.sin_addr = ip->ip_src;
    dst.sin_addr = ip->ip_dst;

    if ((ip->ip_p == IPPROTO_TCP) || (ip->ip_p == IPPROTO_UDP))
    {
	struct	tcphdr	*tcp = cv->_payload._tcp4;

	src.sin_port = tcp->th_sport;
	dst.sin_port = tcp->th_dport;
    }

    return ((_hash_sockaddr4(&src) + _hash_sockaddr4(&dst)) % NATPT_MAXHASH);
}


static int
_hash_ip6(struct _cv *cv)
{
    struct ip6_hdr	*ip6;
    struct sockaddr_in6	 src, dst;

    bzero(&src, sizeof(struct sockaddr_in6));
    bzero(&dst, sizeof(struct sockaddr_in6));

    ip6 = cv->_ip._ip6;
    src.sin6_addr = ip6->ip6_src;
    dst.sin6_addr = ip6->ip6_dst;

    if ((cv->ip_payload == IPPROTO_TCP) || (cv->ip_payload == IPPROTO_UDP))
    {
	struct tcp6hdr	*tcp6 = cv->_payload._tcp6;

	src.sin6_port = tcp6->th_sport;
	dst.sin6_port = tcp6->th_dport;
    }

    return ((_hash_sockaddr6(&src) + _hash_sockaddr6(&dst)) % NATPT_MAXHASH);
}


static int
_hash_pat4(struct _pat *pat4)
{
    struct sockaddr_in	src, dst;

    bzero(&src, sizeof(struct sockaddr_in));
    bzero(&dst, sizeof(struct sockaddr_in));

    src.sin_port = pat4->sport;
    src.sin_addr = pat4->src.u.in4;
    dst.sin_port = pat4->dport;
    dst.sin_addr = pat4->dst.u.in4;

    return ((_hash_sockaddr4(&src) + _hash_sockaddr4(&dst)) % NATPT_MAXHASH);
}


static int
_hash_pat6(struct _pat *pat6)
{
    struct sockaddr_in6	src, dst;

    bzero(&src, sizeof(struct sockaddr_in6));
    bzero(&dst, sizeof(struct sockaddr_in6));

    src.sin6_port = pat6->sport;
    src.sin6_addr = pat6->src.u.in6;
    dst.sin6_port = pat6->dport;
    dst.sin6_addr = pat6->dst.u.in6;

    return ((_hash_sockaddr6(&src) + _hash_sockaddr6(&dst)) % NATPT_MAXHASH);
}


static int
_hash_sockaddr4(struct sockaddr_in *sin4)
{
    int	byte;
    
    byte = sizeof(sin4->sin_port) + sizeof(sin4->sin_addr);
    return (_hash_pjw((char *)&sin4->sin_port, byte));
}


static int
_hash_sockaddr6(struct sockaddr_in6 *sin6)
{
    int	byte;
    
    sin6->sin6_flowinfo = 0;
    byte = sizeof(sin6->sin6_port)
	    + sizeof(sin6->sin6_flowinfo)
	    + sizeof(sin6->sin6_addr);
    return (_hash_pjw((char *)&sin6->sin6_port, byte));
}


/*	CAUTION								*/
/*	This hash routine is byte order sensitive.  Be Careful.		*/

static	int
_hash_pjw(register u_char *s, int len)
{
    register	u_int	c;
    register	u_int	h, g;

    for (c = h = g = 0; c < len; c++, s++)
    {
	h = (h << 4) + (*s);
	if ((g = h & 0xf0000000))
	{
	    h ^= (g >> 24);
	    h ^= g;
	}
    }
    return (h % NATPT_MAXHASH);
}


/*
 *
 */

void
init_hash()
{
    bzero((caddr_t)_insideHash,	 sizeof(_insideHash));
    bzero((caddr_t)_outsideHash, sizeof(_outsideHash));
}


void
init_tslot()
{
    tSlotEntry = NULL;
    tSlotEntryMax = MAXTSLOTENTRY;
    tSlotEntryUsed = 0;

    tSlotTimer = 60 * hz;
    timeout(_expireTSlot, (caddr_t)0, tSlotTimer);

    _natpt_TCPT_2MSL   = 120;				/* [sec]	*/
    _natpt_tcp_maxidle = 600;				/* [sec]	*/

    maxTTLicmp = maxTTLudp = _natpt_TCPT_2MSL;
    maxTTLtcp  = maxTTLany = 86400;			/* [sec]	*/
}
