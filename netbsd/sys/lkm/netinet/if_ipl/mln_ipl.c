/*	$NetBSD: mln_ipl.c,v 1.33.2.1 2004/08/30 19:41:10 tron Exp $	*/

/*
 *  Copyright (c) 2004 The NetBSD Foundation, Inc.
 *  All rights reserved.
 *
 *  This code is derived from software contributed to the NetBSD Foundation
 *  by Peter Postma.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *         This product includes software developed by the NetBSD
 *         Foundation, Inc. and its contributors.
 *  4. Neither the name of The NetBSD Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 *  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: mln_ipl.c,v 1.33.2.1 2004/08/30 19:41:10 tron Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lkm.h>

#include <net/if.h>
#include <net/if_types.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

#include "ipl.h"
#include "ip_compat.h"
#include "ip_fil.h"

int		if_ipl_lkmentry(struct lkm_table *, int, int);
static int	if_ipl_lkmload(struct lkm_table *, int);
static int	if_ipl_lkmunload(struct lkm_table *, int);

extern const struct cdevsw ipl_cdevsw;

MOD_DEV(IPL_VERSION, "ipl", NULL, -1, &ipl_cdevsw, -1);

int
if_ipl_lkmentry(struct lkm_table *lkmtp, int cmd, int ver)
{
	LKM_DISPATCH(lkmtp, cmd, ver, if_ipl_lkmload, if_ipl_lkmunload,
	    lkm_nofunc);
}

static int
if_ipl_lkmload(struct lkm_table *lkmtp, int cmd)
{
	const char *defpass;
	int error;

	if (lkmexists(lkmtp))
		return (EEXIST);

	error = iplattach();
	if (error == 0) {
		if (FR_ISPASS(fr_pass))
			defpass = "pass";
		else if (FR_ISBLOCK(fr_pass))
			defpass = "block";
		else
			defpass = "no-match -> block";

		printf("%s initialized.  Default = %s all, Logging = %s%s\n",
		    ipfilter_version, defpass,
#ifdef IPFILTER_LOG
		    "enabled",
#else
		    "disabled",
#endif
#ifdef IPFILTER_COMPILED
		    " (COMPILED)"
#else
		    ""
#endif
		);

		fr_running = 1;
	}

	return (error);
}

static int
if_ipl_lkmunload(struct lkm_table *lkmtp, int cmd)
{
	int error = 0;

	if (fr_running > 0) {
		error = ipldetach();
		if (error == 0)
			fr_running = -1;
	}
	if (error == 0)
		printf("%s unloaded\n", ipfilter_version);

	return (error);
}
