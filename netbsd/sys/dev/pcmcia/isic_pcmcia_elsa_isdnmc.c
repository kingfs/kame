/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Martin Husemann <martin@NetBSD.org>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: isic_pcmcia_elsa_isdnmc.c,v 1.6 2003/12/04 13:57:31 keihan Exp $");

#include "opt_isicpcmcia.h"
#ifdef ISICPCMCIA_ELSA_ISDNMC

#include <sys/param.h>
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
#include <sys/ioccom.h>
#else
#include <sys/ioctl.h>
#endif
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/mbuf.h>

#ifdef __FreeBSD__
#include <machine/clock.h>
#include <i386/isa/isa_device.h>
#else
#include <machine/bus.h>
#include <sys/device.h>
#endif

#include <sys/socket.h>
#include <net/if.h>

#if defined(__NetBSD__) && __NetBSD_Version__ >= 104230000
#include <sys/callout.h>
#endif

#ifdef __FreeBSD__
#include <machine/i4b_debug.h>
#include <machine/i4b_ioctl.h>
#else
#include <netisdn/i4b_debug.h>
#include <netisdn/i4b_ioctl.h>

#include <dev/pcmcia/pcmciareg.h>
#include <dev/pcmcia/pcmciavar.h>
#endif

#include <netisdn/i4b_l2.h>
#include <netisdn/i4b_l1l2.h>
#include <dev/ic/isic_l1.h>
#include <dev/ic/isac.h>
#include <dev/ic/hscx.h>

#include <dev/pcmcia/isic_pcmcia.h>

#ifndef __FreeBSD__
/* PCMCIA support routines */
static u_int8_t elsa_isdnmc_read_reg __P((struct isic_softc *sc, int what, bus_size_t offs));
static void elsa_isdnmc_write_reg __P((struct isic_softc *sc, int what, bus_size_t offs, u_int8_t data));
static void elsa_isdnmc_read_fifo __P((struct isic_softc *sc, int what, void *buf, size_t size));
static void elsa_isdnmc_write_fifo __P((struct isic_softc *sc, int what, const void *data, size_t size));
static void elsa_isdnmc_clrirq __P((struct isic_softc *sc));
#endif

/*
 * The ELSA MicroLink ISDN/MC uses one contigous IO region,
 * mapped by the pcmcia code.
 * The chip access is via three ports:
 */
#define	ISAC_DATA	1	/* ISAC dataport at offset 1 */
#define HSCX_DATA	2	/* HSCX dataport at offset 2 */
#define ADDR_LATCH	4	/* address latch at offset 4 */

/* This is very similar to the ELSA QuickStep 1000 (ISA) card */

#ifdef __FreeBSD__
static void
elsa_isdnmc_clrirq(void *base)
{
}
#else
static void
elsa_isdnmc_clrirq(struct isic_softc *sc)
{
	ISAC_WRITE(I_MASK, 0xff);
	HSCX_WRITE(0, H_MASK, 0xff);
	HSCX_WRITE(1, H_MASK, 0xff);
	ISAC_WRITE(I_MASK, ISAC_IMASK);
	HSCX_WRITE(0, H_MASK, HSCX_A_IMASK);
	HSCX_WRITE(1, H_MASK, HSCX_B_IMASK);
}
#endif

/*---------------------------------------------------------------------------*
 *	read fifo routines
 *---------------------------------------------------------------------------*/
#ifdef __FreeBSD__
static void		
elsa_isdnmc_read_fifo(void *buf, const void *base, size_t len)
{
}
#else
static void
elsa_isdnmc_read_fifo(struct isic_softc *sc, int what, void *buf, size_t size)
{
	bus_space_tag_t t = sc->sc_maps[0].t;
	bus_space_handle_t h = sc->sc_maps[0].h;
	switch (what) {
		case ISIC_WHAT_ISAC:
			bus_space_write_1(t, h, ADDR_LATCH, 0);
			bus_space_read_multi_1(t, h, ISAC_DATA, buf, size);
			break;
		case ISIC_WHAT_HSCXA:
			bus_space_write_1(t, h, ADDR_LATCH, 0);
			bus_space_read_multi_1(t, h, HSCX_DATA, buf, size);
			break;
		case ISIC_WHAT_HSCXB:
			bus_space_write_1(t, h, ADDR_LATCH, 0x40);
			bus_space_read_multi_1(t, h, HSCX_DATA, buf, size);
			break;
	}
}
#endif

/*---------------------------------------------------------------------------*
 *	write fifo routines
 *---------------------------------------------------------------------------*/
#ifdef __FreeBSD__
static void
elsa_isdnmc_write_fifo(void *base, const void *buf, size_t len)
{
}
#else
static void
elsa_isdnmc_write_fifo(struct isic_softc *sc, int what, const void *buf, size_t size)
{
	bus_space_tag_t t = sc->sc_maps[0].t;
	bus_space_handle_t h = sc->sc_maps[0].h;
	switch (what) {
		case ISIC_WHAT_ISAC:
			bus_space_write_1(t, h, ADDR_LATCH, 0);
			bus_space_write_multi_1(t, h, ISAC_DATA, (u_int8_t*)buf, size);
			break;
		case ISIC_WHAT_HSCXA:
			bus_space_write_1(t, h, ADDR_LATCH, 0);
			bus_space_write_multi_1(t, h, HSCX_DATA, (u_int8_t*)buf, size);
			break;
		case ISIC_WHAT_HSCXB:
			bus_space_write_1(t, h, ADDR_LATCH, 0x40);
			bus_space_write_multi_1(t, h, HSCX_DATA, (u_int8_t*)buf, size);
			break;
	}
}
#endif

/*---------------------------------------------------------------------------*
 *	write register routines
 *---------------------------------------------------------------------------*/
#ifdef __FreeBSD__
static void
elsa_isdnmc_write_reg(u_char *base, u_int offset, u_int v)
{
}
#else
static void
elsa_isdnmc_write_reg(struct isic_softc *sc, int what, bus_size_t offs, u_int8_t data)
{
	bus_space_tag_t t = sc->sc_maps[0].t;
	bus_space_handle_t h = sc->sc_maps[0].h;
	switch (what) {
		case ISIC_WHAT_ISAC:
			bus_space_write_1(t, h, ADDR_LATCH, offs);
			bus_space_write_1(t, h, ISAC_DATA, data);
			break;
		case ISIC_WHAT_HSCXA:
			bus_space_write_1(t, h, ADDR_LATCH, offs);
			bus_space_write_1(t, h, HSCX_DATA, data);
			break;
		case ISIC_WHAT_HSCXB:
			bus_space_write_1(t, h, ADDR_LATCH, 0x40+offs);
			bus_space_write_1(t, h, HSCX_DATA, data);
			break;
	}
}
#endif

/*---------------------------------------------------------------------------*
 *	read register routines
 *---------------------------------------------------------------------------*/
#ifdef __FreeBSD__
static u_char
elsa_isdnmc_read_reg(u_char *base, u_int offset)
{
	return 0;
}
#else
static u_int8_t
elsa_isdnmc_read_reg(struct isic_softc *sc, int what, bus_size_t offs)
{
	bus_space_tag_t t = sc->sc_maps[0].t;
	bus_space_handle_t h = sc->sc_maps[0].h;
	switch (what) {
		case ISIC_WHAT_ISAC:
			bus_space_write_1(t, h, ADDR_LATCH, offs);
			return bus_space_read_1(t, h, ISAC_DATA);
		case ISIC_WHAT_HSCXA:
			bus_space_write_1(t, h, ADDR_LATCH, offs);
			return bus_space_read_1(t, h, HSCX_DATA);
		case ISIC_WHAT_HSCXB:
			bus_space_write_1(t, h, ADDR_LATCH, 0x40+offs);
			return bus_space_read_1(t, h, HSCX_DATA);
	}
	return 0;
}
#endif

#ifdef __FreeBSD__
#else

/*
 * XXX - one time only! Some of this has to go into an enable
 * function, with apropriate counterpart in disable, so a card
 * could be removed an inserted again.
 */
int
isic_attach_elsaisdnmc(struct pcmcia_isic_softc *psc, struct pcmcia_config_entry *cfe, struct pcmcia_attach_args *pa)
{
	struct isic_softc *sc = &psc->sc_isic;
	bus_space_tag_t t;
	bus_space_handle_t h;

	/* Validate config info */
	if (cfe->num_memspace != 0)
		printf(": unexpected number of memory spaces %d should be 0\n",
			cfe->num_memspace);
	if (cfe->num_iospace != 1)
		printf(": unexpected number of memory spaces %d should be 1\n",
			cfe->num_iospace);

	/* Allocate pcmcia space */
	if (pcmcia_io_alloc(pa->pf, 0, cfe->iospace[0].length,
			    cfe->iospace[0].length, &psc->sc_pcioh))
		printf(": can't allocate i/o space\n");

	/* map them */
	if (pcmcia_io_map(pa->pf, ((cfe->flags & PCMCIA_CFE_IO16) ?
	    PCMCIA_WIDTH_IO16 : PCMCIA_WIDTH_IO8), 0,
	    cfe->iospace[0].length, &psc->sc_pcioh, &psc->sc_io_window)) {
		printf(": can't map i/o space\n");
		return 0;
	}

	/* Setup bus space maps */
	sc->sc_num_mappings = 1;
	MALLOC_MAPS(sc);

	/* Copy our handles/tags to the MI maps */
	sc->sc_maps[0].t = psc->sc_pcioh.iot;
	sc->sc_maps[0].h = psc->sc_pcioh.ioh;
	sc->sc_maps[0].offset = 0;
	sc->sc_maps[0].size = 0;	/* not our mapping */

	t = sc->sc_maps[0].t;
	h = sc->sc_maps[0].h;

	sc->clearirq = elsa_isdnmc_clrirq;

	sc->readreg = elsa_isdnmc_read_reg;
	sc->writereg = elsa_isdnmc_write_reg;

	sc->readfifo = elsa_isdnmc_read_fifo;
	sc->writefifo = elsa_isdnmc_write_fifo;

	/* setup IOM bus type */
	
	sc->sc_bustyp = BUS_TYPE_IOM2;

	sc->sc_ipac = 0;
	sc->sc_bfifolen = HSCX_FIFO_LEN;

	return 1;
}
#endif

#endif /* ISICPCMCIA_ELSA_ISDNMC */
