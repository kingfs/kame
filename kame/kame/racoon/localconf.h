/*	$KAME: localconf.h,v 1.17 2000/09/13 04:50:27 itojun Exp $	*/

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
 */
/* YIPS @(#)$Id: localconf.h,v 1.17 2000/09/13 04:50:27 itojun Exp $ */

/* local configuration */

/* must include algstrength.h first. */

#define LC_DEFAULT_CF	SYSCONFDIR "/racoon.conf"

#define LC_PATHTYPE_INCLUDE	0
#define LC_PATHTYPE_PSK		1
#define LC_PATHTYPE_CERT	2
#define LC_PATHTYPE_MAX		3

#define LC_IDENTTYPE_FQDN		0
#define LC_IDENTTYPE_USERFQDN		1
#define LC_IDENTTYPE_KEYID		2
#define LC_IDENTTYPE_ADDRESS		3
#define LC_IDENTTYPE_CERTNAME		4
#define LC_IDENTTYPE_CERTALTNAME	5
#define LC_IDENTTYPE_MAX		6

#define LC_DEFAULT_PAD_MAXSIZE		20
#define LC_DEFAULT_PAD_RANDOM		TRUE
#define LC_DEFAULT_PAD_RANDOMLEN	FALSE
#define LC_DEFAULT_PAD_STRICT		FALSE
#define LC_DEFAULT_PAD_EXCLTAIL		TRUE
#define LC_DEFAULT_RETRY_COUNTER	5
#define LC_DEFAULT_RETRY_INTERVAL	20
#define LC_DEFAULT_COUNT_PERSEND	1
#define LC_DEFAULT_RETRY_CHECKPH1	15
#define LC_DEFAULT_WAIT_PH2COMPLETE	10

#define LC_DEFAULT_SECRETSIZE	16	/* 128 bits */

struct localconf {
	char *racoon_conf;

	u_int16_t port_isakmp;		/* port for isakmp as default */
	u_int16_t port_admin;		/* port for admin */
	int default_af;			/* default address family */

	int sock_admin;
	int sock_pfkey;
	int rtsock;			/* routing socket */

	int autograbaddr;
	struct myaddrs *myaddrs;

	vchar_t *vendorid;		/* XXX Peer's vendorid should be
					 * holded to be multiple */
	char *pathinfo[LC_PATHTYPE_MAX];
	vchar_t *ident[LC_IDENTTYPE_MAX]; /* base of Identifier payload. */

	int pad_random;
	int pad_randomlen;
	int pad_maxsize;
	int pad_strict;
	int pad_excltail;

	int retry_counter;		/* times to retry. */
	int retry_interval;		/* interval each retry. */
	int count_persend;		/* the number of packets each retry. */
				/* above 3 values are copied into a handler. */

	int retry_checkph1;
	int wait_ph2complete;

	int secret_size;
	int strict_address;		/* strictly check addresses. */

	struct algorithm_strength **algstrength;
		/*
		 * There is a different both of the number and the kind of
		 * algorithms between oakley's and ipsec_doi's.
		 */
};

extern struct localconf *lcconf;

extern void initlcconf __P((void));
extern void flushlcconf __P((void));
extern vchar_t *getpskbyname __P((vchar_t *));
extern vchar_t *getpskbyaddr __P((struct sockaddr *));
extern void getpathname __P((char *, int, int, const char *));
extern int doi2idtype __P((int));
extern int idtype2doi __P((int));
extern int sittype2doi __P((int));
extern int doitype2doi __P((int));
