/*	$NetBSD: joy.c,v 1.6 2003/06/29 22:30:12 fvdl Exp $	*/

/*-
 * Copyright (c) 1995 Jean-Marc Zucconi
 * All rights reserved.
 *
 * Ported to NetBSD by Matthieu Herrb <matthieu@laas.fr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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
 *
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: joy.c,v 1.6 2003/06/29 22:30:12 fvdl Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/conf.h>
#include <sys/event.h>

#include <machine/bus.h>

#include <machine/cpu.h>
#include <machine/pio.h>
#include <machine/joystick.h>

#include <dev/isa/isavar.h>
#include <dev/isa/isareg.h>

#include <dev/ic/joyvar.h>

/*
 * The game port can manage 4 buttons and 4 variable resistors (usually 2
 * joysticks, each with 2 buttons and 2 pots.) via the port at address 0x201.
 * Getting the state of the buttons is done by reading the game port;
 * buttons 1-4 correspond to bits 4-7 and resistors 1-4 (X1, Y1, X2, Y2)
 * to bits 0-3.  If button 1 (resp 2, 3, 4) is pressed, the bit 4 (resp 5,
 * 6, 7) is set to 0 to get the value of a resistor, write the value 0xff
 * at port and wait until the corresponding bit returns to 0.
 */

/*
 * The formulae below only work if u is ``not too large''.  See also
 * the discussion in microtime.s
 */
#define USEC2TICKS(u) 	(((u) * 19549) >> 14)
#define TICKS2USEC(u) 	(((u) * 3433) >> 12)


#define JOYPART(d) (minor(d) & 1)
#define JOYUNIT(d) minor(d) >> 1 & 3

#ifndef JOY_TIMEOUT
#define JOY_TIMEOUT   2000	/* 2 milliseconds */
#endif

extern struct cfdriver joy_cd;

dev_type_open(joyopen);
dev_type_close(joyclose);
dev_type_read(joyread);
dev_type_ioctl(joyioctl);

const struct cdevsw joy_cdevsw = {
	joyopen, joyclose, joyread, nowrite, joyioctl,
	nostop, notty, nopoll, nommap, nokqfilter,
};

void
joyattach(sc)
	struct joy_softc *sc;
{

	sc->timeout[0] = sc->timeout[1] = 0;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0xff);
	DELAY(10000);		/* 10 ms delay */
	printf("%s: joystick %sconnected\n", sc->sc_dev.dv_xname,
	    (bus_space_read_1(sc->sc_iot, sc->sc_ioh, 0) & 0x0f) == 0x0f ?
	    "not " : "");

	sc->sc_timer_freq = joy_timer_freq();
}

int
joyopen(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	int unit = JOYUNIT(dev);
	int i = JOYPART(dev);
	struct joy_softc *sc;

	if (unit >= joy_cd.cd_ndevs)
		return (ENXIO);
	sc = joy_cd.cd_devs[unit];
	if (sc == 0)
		return (ENXIO);

	if (sc->timeout[i])
		return (EBUSY);

	sc->x_off[i] = sc->y_off[i] = 0;
	sc->timeout[i] = JOY_TIMEOUT;
	return (0);
}

int
joyclose(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	int unit = JOYUNIT(dev);
	int i = JOYPART(dev);
	struct joy_softc *sc = joy_cd.cd_devs[unit];

	sc->timeout[i] = 0;
	return (0);
}

int
joyread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	int unit = JOYUNIT(dev);
	struct joy_softc *sc = joy_cd.cd_devs[unit];
	bus_space_tag_t iot = sc->sc_iot;
	bus_space_handle_t ioh = sc->sc_ioh;
	struct joystick c;
	int i, t0, t1, s;
	int state = 0, x = 0, y = 0;

	s = splhigh();	/* XXX */
	bus_space_write_1(iot, ioh, 0, 0xff);
	t0 = joy_get_tick();
	t1 = t0;
	i = USEC2TICKS(sc->timeout[JOYPART(dev)]);
	while (t0 - t1 < i) {
		state = bus_space_read_1(iot, ioh, 0);
		if (JOYPART(dev) == 1)
			state >>= 2;
		t1 = joy_get_tick();
		if (t1 > t0)
			t1 -= sc->sc_timer_freq / hz;
		if (!x && !(state & 0x01))
			x = t1;
		if (!y && !(state & 0x02))
			y = t1;
		if (x && y)
			break;
	}
	splx(s);	/* XXX */

	c.x = x ? sc->x_off[JOYPART(dev)] + TICKS2USEC(t0 - x) : 0x80000000;
	c.y = y ? sc->y_off[JOYPART(dev)] + TICKS2USEC(t0 - y) : 0x80000000;
	state >>= 4;
	c.b1 = ~state & 1;
	c.b2 = ~(state >> 1) & 1;
	return (uiomove(&c, sizeof(struct joystick), uio));
}

int
joyioctl(dev, cmd, data, flag, p)
	dev_t dev;
	u_long cmd;
	caddr_t data;
	int flag;
	struct proc *p;
{
	int unit = JOYUNIT(dev);
	struct joy_softc *sc = joy_cd.cd_devs[unit];
	int i = JOYPART(dev);
	int x;

	switch (cmd) {
	case JOY_SETTIMEOUT:
		x = *(int *) data;
		if (x < 1 || x > 10000)	/* 10ms maximum! */
			return (EINVAL);
		sc->timeout[i] = x;
		break;
	case JOY_GETTIMEOUT:
		*(int *) data = sc->timeout[i];
		break;
	case JOY_SET_X_OFFSET:
		sc->x_off[i] = *(int *) data;
		break;
	case JOY_SET_Y_OFFSET:
		sc->y_off[i] = *(int *) data;
		break;
	case JOY_GET_X_OFFSET:
		*(int *) data = sc->x_off[i];
		break;
	case JOY_GET_Y_OFFSET:
		*(int *) data = sc->y_off[i];
		break;
	default:
		return (ENXIO);
	}
	return 0;
}
