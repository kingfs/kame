/*	$KAME: mip6_var.h,v 1.22 2002/01/21 11:37:51 keiichi Exp $	*/

/*
 * Copyright (C) 2001 WIDE Project.  All rights reserved.
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
 */

/*
 * Copyright (c) 1999, 2000 and 2001 Ericsson Radio Systems AB
 * All rights reserved.
 *
 * Authors: Conny Larsson <Conny.Larsson@era.ericsson.se>
 *          Mattias Pettersson <Mattias.Pettersson@era.ericsson.se>
 *
 */

#ifndef _MIP6_VAR_H_
#define _MIP6_VAR_H_

#ifdef MIP6_DRAFT13
#define MIP6_SEQNO_T u_int16_t
#else
#define MIP6_SEQNO_T u_int8_t
#endif /* MIP6_DRAFT13 */

#define GET_NETVAL_S(p, v)	bcopy((p), &(v), sizeof(v)), v = ntohs(v)
#define GET_NETVAL_L(p, v)	bcopy((p), &(v), sizeof(v)), v = ntohl(v)
#define SET_NETVAL_S(p, v)	do {					\
					u_int16_t s = htons(v);		\
					bcopy(&s, (p), sizeof(s));	\
				} while (0)
#define SET_NETVAL_L(p, v)	do {					\
					u_int32_t s = htonl(v);		\
					bcopy(&s, (p), sizeof(s));	\
				} while (0)

struct mip6_prefix {
	LIST_ENTRY(mip6_prefix) mpfx_entry;
	struct in6_addr         mpfx_prefix;
	u_int8_t                mpfx_prefixlen;
	u_int32_t               mpfx_vltime;
	time_t                  mpfx_vlexpire;
	u_int32_t               mpfx_pltime;
	time_t                  mpfx_plexpire;
	struct in6_addr         mpfx_haddr;
};
LIST_HEAD(mip6_prefix_list, mip6_prefix);

#define MIP6_PREFIX_TIMEOUT_INTERVAL 5

struct mip6_ha {
	LIST_ENTRY(mip6_ha) mha_entry;
	struct in6_addr     mha_lladdr;    /* XXX link-local addr */
	struct in6_addr     mha_gaddr;     /* XXX global addr */
	u_int8_t            mha_flags;     /* RA flags */
	int16_t             mha_pref;      /* preference */
	u_int16_t           mha_lifetime;  /* HA lifetime */
	time_t              mha_expire;    /* expiration time of this HA. */
};
LIST_HEAD(mip6_ha_list, mip6_ha);

#define MIP6_HA_TIMEOUT_INTERVAL 5

struct mip6_subnet_prefix {
	TAILQ_ENTRY(mip6_subnet_prefix) mspfx_entry;
	struct mip6_prefix              *mspfx_mpfx;
};

struct mip6_subnet_ha {
	TAILQ_ENTRY(mip6_subnet_ha) msha_entry;
	struct mip6_ha              *msha_mha;
};

/* subnet infomation */
struct mip6_subnet {
	LIST_ENTRY(mip6_subnet)                         ms_entry;
	TAILQ_HEAD(mip6_subnet_prefix_list, mip6_subnet_prefix) ms_mspfx_list;
	TAILQ_HEAD(mip6_subnet_ha_list, mip6_subnet_ha) ms_msha_list;
	int ms_refcnt;
};
LIST_HEAD(mip6_subnet_list, mip6_subnet);

#define MIP6_SUBNET_TIMEOUT_INTERVAL 10

struct mip6_bu {
	LIST_ENTRY(mip6_bu) mbu_entry;
	struct in6_addr     mbu_paddr;      /* peer addr of this BU */
	struct in6_addr     mbu_haddr;      /* home address */
	struct in6_addr     mbu_coa;        /* COA */
	u_int32_t           mbu_lifetime;   /* BU lifetime */
	time_t              mbu_expire;     /* expiration time of this BU. */
	u_int32_t           mbu_refresh;    /* refresh frequency */
	time_t              mbu_refexpire;  /* expiration time of refresh. */
	u_int32_t           mbu_acktimeout; /* current ack timo value */
	time_t              mbu_ackexpire;  /* expiration time of ack. */
	MIP6_SEQNO_T        mbu_seqno;      /* sequence number */
	u_int8_t            mbu_flags;      /* BU flags */
	u_int8_t            mbu_state;
	u_int8_t            mbu_reg_state;  /* registration status */
	struct hif_softc    *mbu_hif;       /* back pointer to hif */
	const struct encaptab *mbu_encap;
};
#define MIP6_BU_REG_STATE_NOTREG       0x01
#define MIP6_BU_REG_STATE_REGWAITACK   0x02
#define MIP6_BU_REG_STATE_REG          0x03
#define MIP6_BU_REG_STATE_DEREGWAITACK 0x04

#define MIP6_BU_STATE_WAITSENT    0x01
#define MIP6_BU_STATE_WAITACK     0x02
#define MIP6_BU_STATE_BUNOTSUPP   0x04
#define MIP6_BU_STATE_MIP6NOTSUPP 0x80

#define MIP6_BU_TIMEOUT_INTERVAL 1

struct mip6_bc {
	LIST_ENTRY(mip6_bc)   mbc_entry;
	struct in6_addr       mbc_phaddr;    /* peer home address */
	struct in6_addr       mbc_pcoa;      /* peer coa */
	struct in6_addr       mbc_addr;      /* my addr (needed?) */
	u_int8_t              mbc_status;    /* BA statue */
	u_int8_t              mbc_flags;     /* recved BU flags */
#ifdef MIP6_DRAFT13
	u_int8_t              mbc_prefixlen; /* recved BU prefixlen */
#endif /* MIP6_DRAFT13 */
	MIP6_SEQNO_T          mbc_seqno;     /* recved BU seqno */
	u_int32_t             mbc_lifetime;  /* recved BU lifetime */
	time_t                mbc_expire;    /* expiration time of this BC. */
	u_int8_t              mbc_state;     /* BC state */
	struct ifnet          *mbc_ifp;      /* ifp that the BC belongs to. */
	                                     /* valid only when BUF_HOME. */
	const struct encaptab *mbc_encap;    /* encapsulation from MN */
	void		      *mbc_dad;	     /* dad handler */
};

#define MIP6_BC_STATE_BA_WAITSENT 0x01
#define MIP6_BC_STATE_BR_WAITSENT 0x02
#define MIP6_BC_STATE_DAD_WAIT	  0x04

#define MIP6_BA_STATUS_ACCEPTED              0
#define MIP6_BA_STATUS_UNSPECIFIED           128
#define MIP6_BA_STATUS_PROHIBIT              130
#define MIP6_BA_STATUS_RESOURCES             131
#define MIP6_BA_STATUS_NOT_SUPPORTED         132
#define MIP6_BA_STATUS_NOT_HOME_SUBNET       133
#ifdef MIP6_DRAFT13
#define MIP6_BA_STATUS_INCORRECT_IFID_LENGTH 136
#endif
#define MIP6_BA_STATUS_NOT_HOME_AGENT        137
#define MIP6_BA_STATUS_DAD_FAILED            138
#define MIP6_BA_STATUS_NO_SA                 139
#define MIP6_BA_STATUS_SEQNO_TOO_SMALL       141

LIST_HEAD(mip6_bc_list, mip6_bc);

#define MIP6_BC_TIMEOUT_INTERVAL 1

#ifdef MIP6_DRAFT13
/* Macro for modulo 2^^16 comparison */
#define MIP6_LEQ(a,b)   ((int16_t)((a)-(b)) <= 0)
#else
/* Macro for modulo 2^^8 comparison */
#define MIP6_LEQ(a,b)   ((int8_t)((a)-(b)) <= 0)
#endif /* MIP6_DRAFT13 */

struct mip6_config {
	u_int8_t mcfg_type;
	u_int8_t mcfg_use_ipsec;
	u_int8_t mcfg_use_authdata;
	u_int8_t mcfg_debug;
};
#define MIP6_CONFIG_TYPE_MN 1
#define MIP6_CONFIG_TYPE_HA 2

#define MIP6_IS_MN (mip6_config.mcfg_type == MIP6_CONFIG_TYPE_MN)
#define MIP6_IS_HA (mip6_config.mcfg_type == MIP6_CONFIG_TYPE_HA)

/* packet options used by the mip6 packet output processing routine */
struct mip6_pktopts {
	struct ip6_rthdr *mip6po_rthdr;
	struct ip6_dest *mip6po_haddr;
	struct ip6_dest *mip6po_dest2;
};

/* Buffer for storing a consequtive sequence of sub-options */
struct mip6_buffer {
	int      off;  /* Offset in buffer */
	u_int8_t *buf; /* Must be at least IPV6_MMTU */
};
#define MIP6_BUFFER_SIZE 1500 /* XXX 1500 ? */

/* Definition of length for different destination options */
#define IP6OPT_BULEN   8 /* Length of BU option */
#define IP6OPT_BALEN  11 /* Length of BA option */
#define IP6OPT_BRLEN   0 /* Length of BR option */
#define IP6OPT_HALEN  16 /* Length of HA option */
#define IP6OPT_UIDLEN  2 /* Length of Unique Identifier sub-option */
#define IP6OPT_COALEN 16 /* Length of Alternate COA sub-option */


struct mip6_unuse_hoa {
	LIST_ENTRY (mip6_unuse_hoa) unuse_entry;
	struct in6_addr unuse_addr;
	u_int16_t unuse_port;
};
LIST_HEAD(mip6_unuse_hoa_list, mip6_unuse_hoa);

#endif /* !_MIP6_VAR_H_ */
