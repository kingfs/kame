/*	$NetBSD: ioat66.c,v 1.7 2003/01/06 13:05:14 wiz Exp $	*/

/*
 * Copyright (c) 1996 Christopher G. Demetriou.  All rights reserved.
 * Copyright (c) 1995 Charles M. Hannum.  All rights reserved.
 *
 * This code is derived from public-domain software written by
 * Roland McGrath, and information provided by David Muir Sharnoff.
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
 *	This product includes software developed by Charles M. Hannum.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: ioat66.c,v 1.7 2003/01/06 13:05:14 wiz Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/termios.h>

#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/ic/comreg.h>
#include <dev/ic/comvar.h>

#include <dev/isa/isavar.h>
#include <dev/isa/com_multi.h>

#define	NSLAVES	6

struct ioat66_softc {
	struct device sc_dev;
	void *sc_ih;

	bus_space_tag_t sc_iot;
	int sc_iobase;

	int sc_alive;			/* mask of slave units attached */
	void *sc_slaves[NSLAVES];	/* com device unit numbers */
	bus_space_handle_t sc_slaveioh[NSLAVES];
	bus_space_handle_t sc_intmasq;
};

int ioatbases[NSLAVES]={0x220,0x228,0x240,0x248,0x260,0x268};
#define IOAT66SHARED 0x208

int ioat66probe __P((struct device *, struct cfdata *, void *));
void ioat66attach __P((struct device *, struct device *, void *));
int ioat66intr __P((void *));
int ioat66print __P((void *, const char *));

CFATTACH_DECL(ioat, sizeof(struct ioat_softc),
    ioat66probe, ioat66attach, NULL, NULL);

int
ioat66probe(parent, self, aux)
	struct device *parent;
	struct cfdata *self;
	void *aux;
{
	struct isa_attach_args *ia = aux;
	int iobase = ia->ia_iobase;
	bus_space_tag_t iot = ia->ia_iot;
	bus_space_handle_t ioh;
	int i, rv = 1;

	/*
	 * Do the normal com probe for the first UART and assume
	 * its presence, and the ability to map the other UARTS,
	 * means there is a multiport board there.
	 * XXX Needs more robustness.
	 */

	/* Disallow wildcarded i/o address. */
	if (ia->ia_iobase == ISACF_PORT_DEFAULT)
		return (0);

	/* if the first port is in use as console, then it. */
	if (com_is_console(iot, iobase, 0))
		goto checkmappings;

	if (bus_space_map(iot, iobase, COM_NPORTS, 0, &ioh)) {
		rv = 0;
		goto out;
	}
	rv = comprobe1(iot, ioh);
	bus_space_unmap(iot, ioh, COM_NPORTS);
	if (rv == 0)
		goto out;

checkmappings:
	for (i = 1; i < NSLAVES; i++) {
		iobase = ioatbases[i];

		if (com_is_console(iot, iobase, 0))
			continue;

		if (bus_space_map(iot, iobase, COM_NPORTS, 0, &ioh)) {
			rv = 0;
			goto out;
		}
		bus_space_unmap(iot, ioh, COM_NPORTS);
	}

out:
	if (rv)
		ia->ia_iosize = NSLAVES * COM_NPORTS;
	return (rv);
}

int
ioat66print(aux, pnp)
	void *aux;
	const char *pnp;
{
	struct commulti_attach_args *ca = aux;

	if (pnp)
		aprint_normal("com at %s", pnp);
	aprint_normal(" slave %d", ca->ca_slave);
	return (UNCONF);
}

void
ioat66attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct ioat66_softc *sc = (void *)self;
	struct isa_attach_args *ia = aux;
	struct commulti_attach_args ca;
	bus_space_tag_t iot = ia->ia_iot;
	int i, iobase;

	printf("\n");

	sc->sc_iot = ia->ia_iot;
	sc->sc_iobase = ia->ia_iobase;

	for (i = 0; i < NSLAVES; i++) {
		iobase = ioatbases[i];
		if (!com_is_console(iot, iobase, &sc->sc_slaveioh[i]) &&
		    bus_space_map(iot, iobase, COM_NPORTS, 0,
			&sc->sc_slaveioh[i])) {
			printf("%s: can't map i/o space for slave %d\n",
			     sc->sc_dev.dv_xname, i);
			return;
		}
	}

	if(bus_space_map(iot, IOAT66SHARED, 1, 0, &sc->sc_intmasq)) {
	  printf("%s: can't map shared interrupt mask\n", 
	 	 sc->sc_dev.dv_xname);
	  return;
	}

	for (i = 0; i < NSLAVES; i++) {

		ca.ca_slave = i;
		ca.ca_iot = sc->sc_iot;
		ca.ca_ioh = sc->sc_slaveioh[i];
		ca.ca_iobase = ioatbases[i];
		ca.ca_noien = 0;

		sc->sc_slaves[i] = config_found(self, &ca, ioat66print);
		if (sc->sc_slaves[i] != NULL)
			sc->sc_alive |= 1 << i;
	}

	sc->sc_ih = isa_intr_establish(ia->ia_ic, ia->ia_irq, IST_EDGE,
	    IPL_SERIAL, ioat66intr, sc);
}

int
ioat66intr(arg)
	void *arg;
{
	struct ioat66_softc *sc = arg;
	bus_space_tag_t iot = sc->sc_iot;
	int alive = sc->sc_alive;
	int bits;

	bits = bus_space_read_1(iot, sc->sc_intmasq, 0) & alive;
	if (bits == 0)
		return (0);

	for (;;) {
#define	TRY(n) \
		if (bits & (1 << (n))) \
			comintr(sc->sc_slaves[n]);
		TRY(0);
		TRY(1);
		TRY(2);
		TRY(3);
		TRY(4);
		TRY(5);
#undef TRY
		bits = bus_space_read_1(iot, sc->sc_intmasq, 0) & alive;
		if (bits == 0)
			return (1);
 	}
}
