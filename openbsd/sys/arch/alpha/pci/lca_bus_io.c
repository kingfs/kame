/*	$OpenBSD: lca_bus_io.c,v 1.2 1997/11/10 15:53:12 niklas Exp $	*/
/*	$NetBSD: lca_bus_io.c,v 1.1 1996/11/25 03:42:14 cgd Exp $	*/

/*
 * Copyright (c) 1996 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/syslog.h>
#include <sys/device.h>
#include <vm/vm.h>

#include <machine/bus.h>

#include <alpha/pci/lcareg.h>
#include <alpha/pci/lcavar.h>

#define	CHIP		lca

#define	CHIP_EX_MALLOC_SAFE(v)	(((struct lca_config *)(v))->lc_mallocsafe)
#define	CHIP_IO_EXTENT(v)	(((struct lca_config *)(v))->lc_io_ex)

/* IO region 1 */
#define	CHIP_IO_W1_BUS_START(v)	0x00000000UL
#define	CHIP_IO_W1_BUS_END(v)	0x00ffffffUL
#define	CHIP_IO_W1_SYS_START(v)	LCA_PCI_SIO
#define	CHIP_IO_W1_SYS_END(v)	(LCA_PCI_SIO + ((0x00ffffffUL + 1) << 5) - 1)

#include "pcs_bus_io_common.c"
