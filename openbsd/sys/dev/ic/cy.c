/*	$OpenBSD: cy.c,v 1.8 1997/08/04 09:57:19 dgregor Exp $	*/

/*
 * cy.c
 *
 * Driver for Cyclades Cyclom-8/16/32 multiport serial cards
 * (currently not tested with Cyclom-32 cards)
 *
 * Timo Rossi, 1996
 *
 * Supports both ISA and PCI Cyclom cards
 *
 * Uses CD1400 automatic CTS flow control, and
 * if CY_HW_RTS is defined, uses CD1400 automatic input flow control.
 * This requires a special cable that exchanges the RTS and DTR lines.
 *
 * Lots of debug output can be enabled by defining CY_DEBUG
 * Some debugging counters (number of receive/transmit interrupts etc.)
 * can be enabled by defining CY_DEBUG1
 *
 * This version uses the bus_space/io_??() stuff
 *
 */

#undef CY_DEBUG
#undef CY_DEBUG1

/* NCY is the number of Cyclom cards in the machine */
#include "cy.h"
#if NCY > 0

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <sys/fcntl.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/systm.h>

#include <machine/bus.h>
#include <machine/intr.h>

#if NCY_ISA > 0	
#include <dev/isa/isavar.h>
#include <dev/isa/isareg.h>
#endif /* NCY_ISA > 0 */
#if NCY_PCI > 0
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#endif /* NCY_PCI > 0 */

#include <dev/ic/cd1400reg.h>
#include <dev/ic/cyreg.h>


/* Macros to clear/set/test flags. */
#define	SET(t, f)	(t) |= (f)
#define	CLR(t, f)	(t) &= ~(f)
#define	ISSET(t, f)	((t) & (f))


void	cyattach __P((struct device *, struct device *, void *));
int	cy_probe_common __P((int, bus_space_tag_t, bus_space_handle_t, int));
int	cyintr __P((void *));
int	cyparam __P((struct tty *, struct termios *));
void	cystart __P((struct tty *));
void	cy_poll __P((void *));
int	cy_modem_control __P((struct cy_port *, int, int));
void	cy_enable_transmitter __P((struct cy_port *));
void	cd1400_channel_cmd __P((struct cy_port *, int));
int	cy_speed __P((speed_t, int *, int *));

struct cfdriver cy_cd = {
  NULL, "cy", DV_TTY
};

static int cy_nr_cd1400s[NCY];
static int cy_bus_types[NCY];
static bus_space_handle_t cy_card_memh[NCY];
static int cy_open = 0;
static int cy_events = 0;


/*
 * Common probe routine
 */
int
cy_probe_common(card, memt, memh, bustype)
     int card, bustype;
     bus_space_tag_t memt;
     bus_space_handle_t memh;
{
  int cy_chip, chip_offs;
  u_char firmware_ver;

  /* Cyclom card hardware reset */
  bus_space_write_1(memt, memh, CY16_RESET<<bustype, 0);
  DELAY(500); /* wait for reset to complete */
  bus_space_write_1(memt, memh, CY_CLEAR_INTR<<bustype, 0);

#ifdef CY_DEBUG
  printf("cy: card reset done\n");
#endif

  cy_nr_cd1400s[card] = 0;

  for(cy_chip = 0, chip_offs = 0;
      cy_chip < CY_MAX_CD1400s;
      cy_chip++, chip_offs += (CY_CD1400_MEMSPACING<<bustype)) {
    int i;

    /* the last 4 cd1400s are 'interleaved'
       with the first 4 on 32-port boards */
    if(cy_chip == 4)
      chip_offs -= (CY32_ADDR_FIX<<bustype);

#ifdef CY_DEBUG
    printf("cy%d probe chip %d offset 0x%lx ... ",
	   card, cy_chip, chip_offs);
#endif

    /* wait until the chip is ready for command */
    DELAY(1000);
    if(bus_space_read_1(memt, memh, chip_offs +
		      ((CD1400_CCR<<1) << bustype)) != 0) {
#ifdef CY_DEBUG
      printf("not ready for command\n");
#endif
      break;
    }

    /* clear the firmware version reg. */
    bus_space_write_1(memt, memh, chip_offs +
		    ((CD1400_GFRCR<<1) << bustype), 0);

    /*
     * On Cyclom-16 references to non-existent chip 4
     * actually access chip 0 (address line 9 not decoded).
     * Here we check if the clearing of chip 4 GFRCR actually
     * cleared chip 0 GFRCR. In that case we have a 16 port card.
     */
    if(cy_chip == 4 &&
       bus_space_read_1(memt, memh, chip_offs +
		      ((CD1400_GFRCR<<1) << bustype)) ==0)
      break;

    /* reset the chip */
    bus_space_write_1(memt, memh, chip_offs +
		    ((CD1400_CCR<<1) << bustype),
		    CD1400_CCR_CMDRESET | CD1400_CCR_FULLRESET);

    /* wait for the chip to initialize itself */
    for(i = 0; i < 200; i++) {
      DELAY(50);
      firmware_ver =
	bus_space_read_1(memt, memh, chip_offs +
		       ((CD1400_GFRCR<<1) << bustype));
      if((firmware_ver & 0xf0) == 0x40) /* found a CD1400 */
	break;
    }
#ifdef CY_DEBUG
    printf("firmware version 0x%x\n", firmware_ver);
#endif      

    if((firmware_ver & 0xf0) != 0x40)
      break;

    /* firmware version OK, CD1400 found */
    cy_nr_cd1400s[card]++;
  }

  if(cy_nr_cd1400s[card] == 0) {
#ifdef CY_DEBUG
    printf("no CD1400s found\n");
#endif
    return 0;
  }

#ifdef CY_DEBUG
  printf("found %d CD1400s\n", cy_nr_cd1400s[card]);
#endif

  cy_card_memh[card] = memh;
  cy_bus_types[card] = bustype;

  return 1;
}

/*
 * Attach (ISA/PCI)
 */
void
cyattach(parent, self, aux)
     struct device *parent, *self;
     void *aux;
{
  struct cy_softc *sc = (void *)self;
  int card, port, cy_chip, num_chips, cdu, chip_offs;

  card = sc->sc_dev.dv_unit;
  num_chips = cy_nr_cd1400s[card];
  if(num_chips == 0)
    return;

  sc->sc_bustype = cy_bus_types[card];
  sc->sc_memh = cy_card_memh[card];
  switch(sc->sc_bustype) {
#if NCY_ISA > 0
    case CY_BUSTYPE_ISA:
      sc->sc_memt = ((struct isa_attach_args *)(aux))->ia_memt;
      break;
#endif
#if NCY_PCI > 0
    case CY_BUSTYPE_PCI:
      sc->sc_memt = ((struct pci_attach_args *)aux)->pa_memt;
      break;
#endif
  }

  bzero(sc->sc_ports, sizeof(sc->sc_ports));
  sc->sc_nports = num_chips * CD1400_NO_OF_CHANNELS;

  port = 0;
  for(cy_chip = 0, chip_offs = 0;
      cy_chip < num_chips; cy_chip++,
      chip_offs += (CY_CD1400_MEMSPACING<<sc->sc_bustype)) {

    if(cy_chip == 4)
      chip_offs -= (CY32_ADDR_FIX<<sc->sc_bustype);

#ifdef CY_DEBUG
    printf("attach CD1400 #%d offset 0x%x\n", cy_chip, chip_offs);
#endif
    sc->sc_cd1400_offs[cy_chip] = chip_offs;

    /* configure port 0 as serial port (should already be after reset) */
    cd_write_reg_sc(sc, cy_chip, CD1400_GCR, 0);

    /* set up a receive timeout period (1ms) */
    cd_write_reg_sc(sc, cy_chip, CD1400_PPR,
		    (CY_CLOCK / CD1400_PPR_PRESCALER / 1000) + 1);

    for(cdu = 0; cdu < CD1400_NO_OF_CHANNELS; cdu++) {
      sc->sc_ports[port].cy_port_num = port;
      sc->sc_ports[port].cy_memt = sc->sc_memt;
      sc->sc_ports[port].cy_memh = sc->sc_memh;
      sc->sc_ports[port].cy_chip_offs = chip_offs;
      sc->sc_ports[port].cy_bustype = sc->sc_bustype;

      /* should we initialize anything else here? */
      port++;
    } /* for(each port on one CD1400...) */

  } /* for(each CD1400 on a card... ) */

  printf(" (%d ports)\n", port);

  /* ensure an edge for the next interrupt */
  bus_space_write_1(sc->sc_memt, sc->sc_memh,
		  CY_CLEAR_INTR<<sc->sc_bustype, 0);

  switch(sc->sc_bustype) {
#if NCY_ISA > 0
    case CY_BUSTYPE_ISA:
      {
	struct isa_attach_args *ia = aux;

	sc->sc_ih =
	  isa_intr_establish(ia->ia_ic, ia->ia_irq,
	    IST_EDGE, IPL_TTY, cyintr, sc, sc->sc_dev.dv_xname);
      }
      break;
#endif /* NCY_ISA > 0 */
#if NCY_PCI > 0
    case CY_BUSTYPE_PCI:
      {
	pci_intr_handle_t intrhandle;
	struct pci_attach_args *pa = aux;

	if(pci_intr_map(pa->pa_pc, pa->pa_intrtag, pa->pa_intrpin,
			pa->pa_intrline, &intrhandle) != 0)
	  panic("cy: couldn't map PCI interrupt");

	sc->sc_ih = pci_intr_establish(pa->pa_pc, intrhandle,
	  IPL_TTY, cyintr, sc, sc->sc_dev.dv_xname);
      }
      break;
#endif /* NCY_PCI > 0 */
  }

  if(sc->sc_ih == NULL)
    panic("cy: couldn't establish interrupt");
}

#undef CY_DEBUG /*!!*/

/*
 * open routine. returns zero if successfull, else error code
 */
int cyopen __P((dev_t, int, int, struct proc *));
int cyclose __P((dev_t, int, int, struct proc *));
int cyread __P((dev_t, struct uio *, int));
int cywrite __P((dev_t, struct uio *, int));
struct tty *cytty __P((dev_t));
int cyioctl __P((dev_t, u_long, caddr_t, int, struct proc *));
int cystop __P((struct tty *, int flag));

int
cyopen(dev, flag, mode, p)
     dev_t dev;
     int flag, mode;
     struct proc *p;
{
    int card = CY_CARD(dev);
    int port = CY_PORT(dev);
    struct cy_softc *sc;
    struct cy_port *cy;
    struct tty *tp;
    int s, error;

#ifdef CY_DEBUG
    printf("cy%d open port %d flag 0x%x mode 0x%x\n",
	   card, port, flag, mode);
#endif

    if(card >= cy_cd.cd_ndevs ||
       (sc = cy_cd.cd_devs[card]) == NULL)
      return ENXIO;

    cy = &sc->sc_ports[port];

    s = spltty();
    if(cy->cy_tty == NULL) {
	if((cy->cy_tty = ttymalloc()) == NULL) {
	    splx(s);
	    printf("cy%d port %d open: can't allocate tty\n", card, port);
	    return ENOMEM;
	}
    }
    splx(s);

    tp = cy->cy_tty;
    tty_attach(tp);
    tp = cy->cy_tty;
    tp->t_oproc = cystart;
    tp->t_param = cyparam;
    tp->t_dev = dev;

    if(!ISSET(tp->t_state, TS_ISOPEN)) {
	SET(tp->t_state, TS_WOPEN);
	ttychars(tp);
	tp->t_iflag = TTYDEF_IFLAG;
	tp->t_oflag = TTYDEF_OFLAG;
	tp->t_cflag = TTYDEF_CFLAG;
	if(ISSET(cy->cy_openflags, TIOCFLAG_CLOCAL))
	  SET(tp->t_cflag, CLOCAL);
	if(ISSET(cy->cy_openflags, TIOCFLAG_CRTSCTS))
	  SET(tp->t_cflag, CRTSCTS);
	if(ISSET(cy->cy_openflags, TIOCFLAG_MDMBUF))
	  SET(tp->t_cflag, MDMBUF);
	tp->t_lflag = TTYDEF_LFLAG;
	tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;

	s = spltty();

	/*
	 * Allocate input ring buffer if we don't already have one
	 */
	if(cy->cy_ibuf == NULL) {
	    cy->cy_ibuf = malloc(IBUF_SIZE, M_DEVBUF, M_NOWAIT);
	    if(cy->cy_ibuf == NULL) {
		printf("cy%d: (port %d) can't allocate input buffer\n",
		       card, port);
		splx(s);
		return ENOMEM;
	    }
	    cy->cy_ibuf_end = cy->cy_ibuf + IBUF_SIZE;
	}

	/* mark the ring buffer as empty */
	cy->cy_ibuf_rd_ptr = cy->cy_ibuf_wr_ptr = cy->cy_ibuf;

	/* select CD1400 channel */
	cd_write_reg(cy, CD1400_CAR, port & CD1400_CAR_CHAN);
	/* reset the channel */
	cd1400_channel_cmd(cy, CD1400_CCR_CMDRESET);
	/* encode unit (port) number in LIVR */
	/* there is just enough space for 5 bits (32 ports) */
	cd_write_reg(cy, CD1400_LIVR, port << 3);

	cy->cy_channel_control = 0;

	/* hmm... need spltty() here? */
	if(cy_open == 0)
	  {
	    cy_open = 1;
	    timeout(cy_poll, NULL, 1);
	  }

	/* this sets parameters and raises DTR */
	cyparam(tp, &tp->t_termios);

	ttsetwater(tp);

	/* raise RTS too */
	cy_modem_control(cy, TIOCM_RTS, DMBIS);

	cy->cy_carrier_stat =
	  cd_read_reg(cy, CD1400_MSVR2);

	/* enable receiver and modem change interrupts */
	cd_write_reg(cy, CD1400_SRER, CD1400_SRER_MDMCH | CD1400_SRER_RXDATA);

	if(CY_DIALOUT(dev) ||
	   ISSET(cy->cy_openflags, TIOCFLAG_SOFTCAR) ||
	   ISSET(tp->t_cflag, MDMBUF) ||
	   ISSET(cy->cy_carrier_stat, CD1400_MSVR2_CD))
	  SET(tp->t_state, TS_CARR_ON);
	else
	  CLR(tp->t_state, TS_CARR_ON);
    } else if(ISSET(tp->t_state, TS_XCLUDE) && p->p_ucred->cr_uid != 0) {
	return EBUSY;
    } else {
	s = spltty();
    }

    /* wait for carrier if necessary */
    if(!ISSET(flag, O_NONBLOCK)) {
	while(!ISSET(tp->t_cflag, CLOCAL) &&
	    !ISSET(tp->t_state, TS_CARR_ON)) {
	    SET(tp->t_state, TS_WOPEN);
	    error = ttysleep(tp, &tp->t_rawq, TTIPRI | PCATCH, "cydcd", 0);
	    if(error != 0) {
		splx(s);
		CLR(tp->t_state, TS_WOPEN);
		return error;
	    }
	}
    }

    splx(s);

    return (*linesw[tp->t_line].l_open)(dev, tp);
}

/*
 * close routine. returns zero if successfull, else error code
 */
int
cyclose(dev, flag, mode, p)
     dev_t dev;
     int flag, mode;
     struct proc *p;
{
    int card = CY_CARD(dev);
    int port = CY_PORT(dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    struct tty *tp = cy->cy_tty;
    int s;

#ifdef CY_DEBUG
    printf("cy%d close port %d, flag 0x%x, mode 0x%x\n",
	 card, port, flag, mode);
#endif

    (*linesw[tp->t_line].l_close)(tp, flag);
    s = spltty();

    if(ISSET(tp->t_cflag, HUPCL) &&
       !ISSET(cy->cy_openflags, TIOCFLAG_SOFTCAR)) {
	/* drop DTR and RTS
	   (should we wait for output buffer to become empty first?) */
	cy_modem_control(cy, 0, DMSET);
    }

/*
 * XXX should we disable modem change and
 * receive interrupts here or somewhere ?
 */
    CLR(tp->t_state, TS_BUSY | TS_FLUSH);

    splx(s);
    ttyclose(tp);

    return 0;
}

/*
 * Read routine
 */
int
cyread(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
    int card = CY_CARD(dev);
    int port = CY_PORT(dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    struct tty *tp = cy->cy_tty;

#ifdef CY_DEBUG
    printf("cy%d read port %d uio 0x%x flag 0x%x\n",
	   card, port, uio, flag);
#endif

    return ((*linesw[tp->t_line].l_read)(tp, uio, flag));
}

/*
 * Write routine
 */
int
cywrite(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
    int card = CY_CARD(dev);
    int port = CY_PORT(dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    struct tty *tp = cy->cy_tty;

#ifdef CY_DEBUG
    printf("cy%d write port %d uio 0x%x flag 0x%x\n",
	   card, port, uio, flag);
#endif

    return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}

/*
 * return tty pointer
 */
struct tty *
cytty(dev)
     dev_t dev;
{
    int card = CY_CARD(dev);
    int port = CY_PORT(dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    struct tty *tp = cy->cy_tty;

#ifdef CY_DEBUG
    printf("cy%d tty port %d tp 0x%x\n",
	   card, port, tp);
#endif

    return tp;
}

/*
 * ioctl routine
 */
int
cyioctl(dev, cmd, data, flag, p)
     dev_t dev;
     u_long cmd;
     caddr_t data;
     int flag;
     struct proc *p;
{
    int card = CY_CARD(dev);
    int port = CY_PORT(dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    struct tty *tp = cy->cy_tty;
    int error;

#ifdef CY_DEBUG
    printf("cy%d port %d ioctl cmd 0x%x data 0x%x flag 0x%x\n",
	   card, port, cmd, data, flag);
#endif

    error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag, p);
    if(error >= 0)
      return error;

    error = ttioctl(tp, cmd, data, flag, p);
    if(error >= 0)
      return error;

/* XXX should not allow dropping DTR when dialin? */

    switch(cmd) {
      case TIOCSBRK: /* start break */
        SET(cy->cy_flags, CYF_START_BREAK);
	cy_enable_transmitter(cy);
        break;

      case TIOCCBRK: /* stop break */
	SET(cy->cy_flags, CYF_END_BREAK);
	cy_enable_transmitter(cy);
	break;

      case TIOCSDTR: /* DTR on */
	cy_modem_control(cy, TIOCM_DTR, DMBIS);
	break;

      case TIOCCDTR: /* DTR off */
	cy_modem_control(cy, TIOCM_DTR, DMBIC);
	break;

      case TIOCMSET: /* set new modem control line values */
	cy_modem_control(cy, *((int *)data), DMSET);
	break;

      case TIOCMBIS: /* turn modem control bits on */
	cy_modem_control(cy, *((int *)data), DMBIS);
	break;

      case TIOCMBIC: /* turn modem control bits off */
	cy_modem_control(cy, *((int *)data), DMBIC);
	break;

      case TIOCMGET: /* get modem control/status line state */
	*((int *)data) = cy_modem_control(cy, 0, DMGET);
	break;

      case TIOCGFLAGS:
	*((int *)data) = cy->cy_openflags |
	  (CY_DIALOUT(dev) ? TIOCFLAG_SOFTCAR : 0);
	break;

      case TIOCSFLAGS:
	error = suser(p->p_ucred, &p->p_acflag);
	if(error != 0)
	  return EPERM;

	cy->cy_openflags = *((int *)data) &
	  (TIOCFLAG_SOFTCAR | TIOCFLAG_CLOCAL |
	   TIOCFLAG_CRTSCTS | TIOCFLAG_MDMBUF);
	break;

      default:
	return ENOTTY;
    }

    return 0;
}

/*
 * start output
 */
void
cystart(tp)
     struct tty *tp;
{
    int card = CY_CARD(tp->t_dev);
    int port = CY_PORT(tp->t_dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    int s;

#ifdef CY_DEBUG
    printf("cy%d port %d start, tty 0x%x\n", card, port, tp);
#endif


    s = spltty();

#ifdef CY_DEBUG1
    cy->cy_start_count++;
#endif

  if(!ISSET(tp->t_state, TS_TTSTOP | TS_TIMEOUT | TS_BUSY)) {
      if(tp->t_outq.c_cc <= tp->t_lowat) {
	  if(ISSET(tp->t_state, TS_ASLEEP)) {
	      CLR(tp->t_state, TS_ASLEEP);
	      wakeup(&tp->t_outq);
	  }

	  selwakeup(&tp->t_wsel);

	  if(tp->t_outq.c_cc == 0)
	    goto out;
      }

      SET(tp->t_state, TS_BUSY);
      cy_enable_transmitter(cy);
  }
out:

    splx(s);
}

/*
 * stop output
 */
int
cystop(tp, flag)
     struct tty *tp;
     int flag;
{
    int card = CY_CARD(tp->t_dev);
    int port = CY_PORT(tp->t_dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    int s;

#ifdef CY_DEBUG
    printf("cy%d port %d stop tty 0x%x flag 0x%x\n",
	   card, port, tp, flag);
#endif

    s = spltty();

    if(ISSET(tp->t_state, TS_BUSY)) {
	if(!ISSET(tp->t_state, TS_TTSTOP))
	  SET(tp->t_state, TS_FLUSH);

	/*
	 * the transmit interrupt routine will disable transmit when it
	 * notices that CYF_STOP has been set.
	 */
	SET(cy->cy_flags, CYF_STOP);
    }
    splx(s);
    return(0);
}

/*
 * parameter setting routine.
 * returns 0 if successfull, else returns error code
 */
int
cyparam(tp, t)
     struct tty *tp;
     struct termios *t;
{
    int card = CY_CARD(tp->t_dev);
    int port = CY_PORT(tp->t_dev);
    struct cy_softc *sc = cy_cd.cd_devs[card];
    struct cy_port *cy = &sc->sc_ports[port];
    int ibpr, obpr, i_clk_opt, o_clk_opt;
    int s, opt;

#ifdef CY_DEBUG
    printf("cy%d port %d param tty 0x%x termios 0x%x\n",
	   card, port, tp, t);
    printf("ispeed %d ospeed %d\n", t->c_ispeed, t->c_ospeed);
#endif

    if(t->c_ospeed != 0 && cy_speed(t->c_ospeed, &o_clk_opt, &obpr) < 0)
      return EINVAL;

    if(t->c_ispeed != 0 && cy_speed(t->c_ispeed, &i_clk_opt, &ibpr) < 0)
      return EINVAL;

    s = spltty();

    /* hang up the line is ospeed is zero, else turn DTR on */
    cy_modem_control(cy, TIOCM_DTR, (t->c_ospeed == 0 ? DMBIC : DMBIS));

    /* channel was selected by the above call to cy_modem_control() */
    /* cd_write_reg(cy, CD1400_CAR, port & CD1400_CAR_CHAN); */

    /* set transmit speed */
    if(t->c_ospeed != 0) {
	cd_write_reg(cy, CD1400_TCOR, o_clk_opt);
	cd_write_reg(cy, CD1400_TBPR, obpr);
    }
    /* set receive speed */
    if(t->c_ispeed != 0) {
	cd_write_reg(cy, CD1400_RCOR, i_clk_opt);
	cd_write_reg(cy, CD1400_RBPR, ibpr);
    }

    opt = CD1400_CCR_CMDCHANCTL | CD1400_CCR_XMTEN
      | (ISSET(t->c_cflag, CREAD) ? CD1400_CCR_RCVEN : CD1400_CCR_RCVDIS);

    if(opt != cy->cy_channel_control) {
	cy->cy_channel_control = opt;
	cd1400_channel_cmd(cy, opt);
    }

    /* compute COR1 contents */
    opt = 0;
    if(ISSET(t->c_cflag, PARENB)) {
	if(ISSET(t->c_cflag, PARODD))
	  opt |= CD1400_COR1_PARODD;
	opt |= CD1400_COR1_PARNORMAL;
    }

    if(!ISSET(t->c_iflag, INPCK))
      opt |= CD1400_COR1_NOINPCK; /* no parity checking */

    if(ISSET(t->c_cflag, CSTOPB))
      opt |= CD1400_COR1_STOP2;

    switch(t->c_cflag & CSIZE) {
      case CS5:
        opt |= CD1400_COR1_CS5;
        break;

      case CS6:
        opt |= CD1400_COR1_CS6;
	break;

      case CS7:
        opt |= CD1400_COR1_CS7;
	break;

      default:
        opt |= CD1400_COR1_CS8;
	break;
    }

    cd_write_reg(cy, CD1400_COR1, opt);

#ifdef CY_DEBUG
    printf("cor1 = 0x%x...", opt);
#endif

/*
 * use the CD1400 automatic CTS flow control if CRTSCTS is set
 *
 * CD1400_COR2_ETC is used because breaks are generated with
 * embedded transmit commands
 */
    cd_write_reg(cy, CD1400_COR2,
		 CD1400_COR2_ETC |
		 (ISSET(t->c_cflag, CRTSCTS) ? CD1400_COR2_CCTS_OFLOW : 0));

    cd_write_reg(cy, CD1400_COR3, RX_FIFO_THRESHOLD);

    cd1400_channel_cmd(cy,
		       CD1400_CCR_CMDCORCHG |
		       CD1400_CCR_COR1 | CD1400_CCR_COR2 | CD1400_CCR_COR3);

    cd_write_reg(cy, CD1400_COR4, CD1400_COR4_PFO_EXCEPTION);
    cd_write_reg(cy, CD1400_COR5, 0);

  /*
   * set modem change option registers to generate interrupts
   * on carrier detect changes.
   *
   * if hardware RTS handshaking is used (CY_HW_RTS, DTR and RTS lines
   * exchanged), also set the handshaking threshold.
   */
#ifdef CY_HW_RTS
    cd_write_reg(cy, CD1400_MCOR1, CD1400_MCOR1_CDzd |
		 (ISSET(t->c_cflag, CRTSCTS) ? RX_DTR_THRESHOLD : 0));
#else
    cd_write_reg(cy, CD1400_MCOR1, CD1400_MCOR1_CDzd);
#endif /* CY_HW_RTS */

    cd_write_reg(cy, CD1400_MCOR2, CD1400_MCOR2_CDod);

    /*
     * set receive timeout to approx. 2ms
     * could use more complex logic here...
     * (but is it actually needed or even useful?)
     */
    cd_write_reg(cy, CD1400_RTPR, 2);

    /*
     * should do anything else here?
     * XXX check MDMBUF handshaking like in com.c?
     */

    splx(s);
    return 0;
}

/*
 * set/get modem line status
 *
 * bits can be: TIOCM_DTR, TIOCM_RTS, TIOCM_CTS, TIOCM_CD, TIOCM_RI, TIOCM_DSR
 *
 * RTS and DTR are exchanged if CY_HW_RTS is set
 *
 */
int
cy_modem_control(cy, bits, howto)
     struct cy_port *cy;
     int bits;
     int howto;
{
    int s, msvr;

    s = spltty();

    /* select channel */
    cd_write_reg(cy, CD1400_CAR, cy->cy_port_num & CD1400_CAR_CHAN);

/* does not manipulate RTS if it is used for flow control */
    switch(howto) {
      case DMGET:
        splx(s);
	bits = 0;
	if(cy->cy_channel_control & CD1400_CCR_RCVEN)
	  bits |= TIOCM_LE;
	msvr = cd_read_reg(cy, CD1400_MSVR2);
#ifdef CY_HW_RTS
	if(cd_read_reg(cy, CD1400_MSVR1) & CD1400_MSVR1_RTS)
	  bits |= TIOCM_DTR;
	if(msvr & CD1400_MSVR2_DTR)
	  bits |= TIOCM_RTS;
#else
	if(cd_read_reg(cy, CD1400_MSVR1) & CD1400_MSVR1_RTS)
	  bits |= TIOCM_RTS;
	if(msvr & CD1400_MSVR2_DTR)
	  bits |= TIOCM_DTR;
#endif /* CY_HW_RTS */
	if(msvr & CD1400_MSVR2_CTS)
	  bits |= TIOCM_CTS;
	if(msvr & CD1400_MSVR2_CD)
	  bits |= TIOCM_CD;
	if(msvr & CD1400_MSVR2_DSR) /* not connected on some Cyclom cards? */
	  bits |= TIOCM_DSR;
	if(msvr & CD1400_MSVR2_RI) /* not connected on Cyclom-8Y cards? */
	  bits |= TIOCM_RI;
	splx(s);
	return bits;

      case DMSET: /* replace old values with new ones */
#ifdef CY_HW_RTS
	if(!ISSET(cy->cy_tty->t_cflag, CRTSCTS))
	  cd_write_reg(cy, CD1400_MSVR2,
		       ((bits & TIOCM_RTS) ? CD1400_MSVR2_DTR : 0));
	cd_write_reg(cy, CD1400_MSVR1,
		     ((bits & TIOCM_DTR) ? CD1400_MSVR1_RTS : 0));
#else
	if(!ISSET(cy->cy_tty->t_cflag, CRTSCTS))
	  cd_write_reg(cy, CD1400_MSVR1,
		  ((bits & TIOCM_RTS) ? CD1400_MSVR1_RTS : 0));
	cd_write_reg(cy, CD1400_MSVR2,
		((bits & TIOCM_DTR) ? CD1400_MSVR2_DTR : 0));
#endif /* CY_HW_RTS */
        break;

      case DMBIS: /* set bits */
#ifdef CY_HW_RTS
	if(!ISSET(cy->cy_tty->t_cflag, CRTSCTS) &&
	   (bits & TIOCM_RTS) != 0)
	  cd_write_reg(cy, CD1400_MSVR2, CD1400_MSVR2_DTR);
	if(bits & TIOCM_DTR)
	  cd_write_reg(cy, CD1400_MSVR1, CD1400_MSVR1_RTS);
#else
	if(!ISSET(cy->cy_tty->t_cflag, CRTSCTS) &&
	   (bits & TIOCM_RTS) != 0)
	  cd_write_reg(cy, CD1400_MSVR1, CD1400_MSVR1_RTS);
	if(bits & TIOCM_DTR)
	  cd_write_reg(cy, CD1400_MSVR2, CD1400_MSVR2_DTR);
#endif /* CY_HW_RTS */
	break;

      case DMBIC: /* clear bits */
#ifdef CY_HW_RTS
	if(!ISSET(cy->cy_tty->t_cflag, CRTSCTS) &&
	   (bits & TIOCM_RTS))
	  cd_write_reg(cy, CD1400_MSVR2, 0);
	if(bits & TIOCM_DTR)
	  cd_write_reg(cy, CD1400_MSVR1, 0);
#else
	if(!ISSET(cy->cy_tty->t_cflag, CRTSCTS) &&
	   (bits & TIOCM_RTS))
	  cd_write_reg(cy, CD1400_MSVR1, 0);
	if(bits & TIOCM_DTR)
	  cd_write_reg(cy, CD1400_MSVR2, 0);
#endif /* CY_HW_RTS */
	break;
    }
    splx(s);
    return 0;
}

/*
 * Upper-level handler loop (called from timer interrupt?)
 * This routine is common for multiple cards
 */
void
cy_poll(arg)
     void *arg;
{
    int card, port;
    struct cy_softc *sc;
    struct cy_port *cy;
    struct tty *tp;
    static int counter = 0;
#ifdef CY_DEBUG1
    int did_something;
#endif

    int s;

    s = splhigh();

    if(cy_events == 0 && ++counter < 200) {
        splx(s);
	goto out;
    }

    cy_events = 0;
    splx(s);

    for(card = 0; card < cy_cd.cd_ndevs; card++) {
	sc = cy_cd.cd_devs[card];
	if(sc == NULL)
	  continue;

#ifdef CY_DEBUG1
	sc->sc_poll_count1++;
	did_something = 0;
#endif

	for(port = 0; port < sc->sc_nports; port++) {
	    cy = &sc->sc_ports[port];
	    if((tp = cy->cy_tty) == NULL || cy->cy_ibuf == NULL ||
	       !ISSET(tp->t_state, TS_ISOPEN | TS_WOPEN))
	    continue;

	    /*
	     * handle received data
	     */
	    while(cy->cy_ibuf_rd_ptr != cy->cy_ibuf_wr_ptr) {
	        u_char line_stat;
		int chr;

		line_stat = cy->cy_ibuf_rd_ptr[0];
		chr = cy->cy_ibuf_rd_ptr[1];

		if(line_stat & (CD1400_RDSR_BREAK|CD1400_RDSR_FE))
		  chr |= TTY_FE;
		if(line_stat & CD1400_RDSR_PE)
		  chr |= TTY_PE;

		/*
		 * on an overrun error the data is treated as good
		 * just as it should be.
		 */

#ifdef CY_DEBUG
		printf("cy%d port %d ttyinput 0x%x\n",
		       card, port, chr);
#endif

		(*linesw[tp->t_line].l_rint)(chr, tp);

                s = splhigh(); /* really necessary? */
		if((cy->cy_ibuf_rd_ptr += 2) == cy->cy_ibuf_end)
		  cy->cy_ibuf_rd_ptr = cy->cy_ibuf;
		splx(s);

#ifdef CY_DEBUG1
		did_something = 1;
#endif
	    }

#ifndef CY_HW_RTS
	    /* If we don't have any received data in ibuf and
	     * CRTSCTS is on and RTS is turned off, it is time
	     * to turn RTS back on
	     */
	    if(ISSET(tp->t_cflag, CRTSCTS)) {
	      /* we can't use cy_modem_control() here as it doesn't
		 change RTS if RTSCTS is on */
	      cd_write_reg(cy, CD1400_CAR, port & CD1400_CAR_CHAN);
	      
	      if((cd_read_reg(cy, CD1400_MSVR1) & CD1400_MSVR1_RTS) == 0) {
		cd_write_reg(cy, CD1400_MSVR1, CD1400_MSVR1_RTS);
#ifdef CY_DEBUG1
		did_something = 1;
#endif
	      }
	    }
#endif /* CY_HW_RTS */

	    /*
	     * handle carrier changes
	     */
	    s = splhigh();
	    if(ISSET(cy->cy_flags, CYF_CARRIER_CHANGED)) {
		int carrier;

		CLR(cy->cy_flags, CYF_CARRIER_CHANGED);
		splx(s);

		carrier = ((cy->cy_carrier_stat & CD1400_MSVR2_CD) != 0);

#ifdef CY_DEBUG
		printf("cy_poll: carrier change "
		       "(card %d, port %d, carrier %d)\n",
		       card, port, carrier);
#endif
		if(CY_DIALIN(tp->t_dev) &&
		   !(*linesw[tp->t_line].l_modem)(tp, carrier))
		  cy_modem_control(cy, TIOCM_DTR, DMBIC);

#ifdef CY_DEBUG1
		did_something = 1;
#endif
	    } else {
		splx(s);
	    }

	    s = splhigh();
	    if(ISSET(cy->cy_flags, CYF_START)) {
		CLR(cy->cy_flags, CYF_START);
              splx(s);

		(*linesw[tp->t_line].l_start)(tp);

#ifdef CY_DEBUG1
		did_something = 1;
#endif
	    } else {
                splx(s);
	    }

	    /* could move this to even upper level... */
	    if(cy->cy_fifo_overruns) {
		cy->cy_fifo_overruns = 0;
		/* doesn't report overrun count,
		   but shouldn't really matter */
		log(LOG_WARNING, "cy%d port %d fifo overrun\n",
		    card, port);
	    }
	    if(cy->cy_ibuf_overruns) {
		cy->cy_ibuf_overruns = 0;
		log(LOG_WARNING, "cy%d port %d ibuf overrun\n",
		    card, port);
	    }
	} /* for(port...) */
#ifdef CY_DEBUG1
	if(did_something && counter >= 200)
	  sc->sc_poll_count2++;
#endif
    } /* for(card...) */

    counter = 0;

out:
    timeout(cy_poll, NULL, 1);
}

/*
 * hardware interrupt routine
 */
int
cyintr(arg)
     void *arg;
{
    struct cy_softc *sc = arg;
    struct cy_port *cy;
    int card = sc->sc_dev.dv_unit;
    int cy_chip, stat;
    int int_serviced = 0;

/*
 * Check interrupt status of each CD1400 chip on this card
 * (multiple cards cannot share the same interrupt)
 */
    for(cy_chip = 0; cy_chip < cy_nr_cd1400s[card]; cy_chip++) {

	stat = cd_read_reg_sc(sc, cy_chip, CD1400_SVRR);
	if(stat == 0)
	  continue;

	if(ISSET(stat, CD1400_SVRR_RXRDY)) {
	    u_char save_car, save_rir, serv_type;
	    u_char line_stat, recv_data, n_chars;
	    u_char *buf_p;

	    save_rir = cd_read_reg_sc(sc, cy_chip, CD1400_RIR);
	    save_car = cd_read_reg_sc(sc, cy_chip, CD1400_CAR);
	    /* enter rx service */
	    cd_write_reg_sc(sc, cy_chip, CD1400_CAR, save_rir);

	    serv_type = cd_read_reg_sc(sc, cy_chip, CD1400_RIVR);
	    cy = &sc->sc_ports[serv_type >> 3];

#ifdef CY_DEBUG1
	    cy->cy_rx_int_count++;
#endif

	    if(cy->cy_tty == NULL ||
	       !ISSET(cy->cy_tty->t_state, TS_ISOPEN))
	      goto end_rx_serv;

	    buf_p = cy->cy_ibuf_wr_ptr;

	    if(ISSET(serv_type, CD1400_RIVR_EXCEPTION)) {
		line_stat = cd_read_reg(cy, CD1400_RDSR);
		recv_data = cd_read_reg(cy, CD1400_RDSR);

#ifdef CY_DEBUG
		printf("cy%d port %d recv exception, "
		       "line_stat 0x%x, char 0x%x\n",
		       card, cy->cy_port_num, line_stat, recv_data);
#endif
		if(ISSET(line_stat, CD1400_RDSR_OE))
		  cy->cy_fifo_overruns++;

		*buf_p++ = line_stat;
		*buf_p++ = recv_data;
		if(buf_p == cy->cy_ibuf_end)
		  buf_p = cy->cy_ibuf;

		if(buf_p == cy->cy_ibuf_rd_ptr) {
		    if(buf_p == cy->cy_ibuf)
		      buf_p = cy->cy_ibuf_end;
		    buf_p -= 2;
		    cy->cy_ibuf_overruns++;
		}
		cy_events = 1;
	    } else { /* no exception, received data OK */
		n_chars = cd_read_reg(cy, CD1400_RDCR);
#ifdef CY_DEBUG
		printf("cy%d port %d receive ok %d chars\n",
		       card, cy->cy_port_num, n_chars);
#endif
		while(n_chars--) {
		    *buf_p++ = 0; /* status: OK */
		    *buf_p++ =
		      cd_read_reg(cy, CD1400_RDSR); /* data byte */
		    if(buf_p == cy->cy_ibuf_end)
		      buf_p = cy->cy_ibuf;
		    if(buf_p == cy->cy_ibuf_rd_ptr) {
			if(buf_p == cy->cy_ibuf)
			  buf_p = cy->cy_ibuf_end;
			buf_p -= 2;
			cy->cy_ibuf_overruns++;
			break;
		    }
		}
		cy_events = 1;
	    }

	    cy->cy_ibuf_wr_ptr = buf_p;

#ifndef CY_HW_RTS
	    /* RTS handshaking for incoming data */
	    if(ISSET(cy->cy_tty->t_cflag, CRTSCTS)) {
		int bf;

		bf = buf_p - cy->cy_ibuf_rd_ptr;
		if(bf < 0)
		  bf += IBUF_SIZE;

		if(bf > (IBUF_SIZE/2))  /* turn RTS off */
		  cd_write_reg(cy, CD1400_MSVR1, 0);
	    }
#endif /* CY_HW_RTS */

end_rx_serv:
	    /* terminate service context */
	    cd_write_reg(cy, CD1400_RIR, save_rir & 0x3f);
	    cd_write_reg(cy, CD1400_CAR, save_car);
	    int_serviced = 1;
	} /* if(rx_service...) */

	if(ISSET(stat, CD1400_SVRR_MDMCH)) {
	    u_char save_car, save_mir, serv_type, modem_stat;

	    save_mir = cd_read_reg_sc(sc, cy_chip, CD1400_MIR);
	    save_car = cd_read_reg_sc(sc, cy_chip, CD1400_CAR);
	    /* enter modem service */
	    cd_write_reg_sc(sc, cy_chip, CD1400_CAR, save_mir);

	    serv_type = cd_read_reg_sc(sc, cy_chip, CD1400_MIVR);
	    cy = &sc->sc_ports[serv_type >> 3];

#ifdef CY_DEBUG1
	    cy->cy_modem_int_count++;
#endif

	    modem_stat = cd_read_reg(cy, CD1400_MSVR2);

#ifdef CY_DEBUG
	    printf("cy%d port %d modem line change, new stat 0x%x\n",
		   card, cy->cy_port_num, modem_stat);
#endif
	    if(ISSET((cy->cy_carrier_stat ^ modem_stat), CD1400_MSVR2_CD)) {
		SET(cy->cy_flags, CYF_CARRIER_CHANGED);
		cy_events = 1;
	    }

	    cy->cy_carrier_stat = modem_stat;

	  /* terminate service context */
	    cd_write_reg(cy, CD1400_MIR, save_mir & 0x3f);
	    cd_write_reg(cy, CD1400_CAR, save_car);
	    int_serviced = 1;
	} /* if(modem_service...) */

	if(ISSET(stat, CD1400_SVRR_TXRDY)) {
	    u_char save_car, save_tir, serv_type, count, ch;
	    struct tty *tp;

	    save_tir = cd_read_reg_sc(sc, cy_chip, CD1400_TIR);
	    save_car = cd_read_reg_sc(sc, cy_chip, CD1400_CAR);
	    /* enter tx service */
	    cd_write_reg_sc(sc, cy_chip, CD1400_CAR, save_tir);

	    serv_type = cd_read_reg_sc(sc, cy_chip, CD1400_TIVR);
	    cy = &sc->sc_ports[serv_type >> 3];

#ifdef CY_DEBUG1
	    cy->cy_tx_int_count++;
#endif
#ifdef CY_DEBUG
	    printf("cy%d port %d tx service\n", card, cy->cy_port_num);
#endif

	    /* stop transmitting if no tty or CYF_STOP set */
	    tp = cy->cy_tty;
	    if(tp == NULL || ISSET(cy->cy_flags, CYF_STOP))
	      goto txdone;

	    count = 0;
	    if(ISSET(cy->cy_flags, CYF_SEND_NUL)) {
		cd_write_reg(cy, CD1400_TDR, 0);
		cd_write_reg(cy, CD1400_TDR, 0);
		count += 2;
		CLR(cy->cy_flags, CYF_SEND_NUL);
	    }

	    if(tp->t_outq.c_cc > 0) {
		SET(tp->t_state, TS_BUSY);
		while(tp->t_outq.c_cc > 0 && count < CD1400_TX_FIFO_SIZE) {
		    ch = getc(&tp->t_outq);
		    /* remember to double NUL characters because
		       embedded transmit commands are enabled */
		    if(ch == 0) {
			if(count >= CD1400_TX_FIFO_SIZE-2) {
			    SET(cy->cy_flags, CYF_SEND_NUL);
			    break;
			}

			cd_write_reg(cy, CD1400_TDR, ch);
			count++;
		    }

		    cd_write_reg(cy, CD1400_TDR, ch);
		    count++;
		}
	    } else {
		/* no data to send -- check if we should start/stop a break */
		/* XXX does this cause too much delay before breaks? */
		if(ISSET(cy->cy_flags, CYF_START_BREAK)) {
		    cd_write_reg(cy, CD1400_TDR, 0);
		    cd_write_reg(cy, CD1400_TDR, 0x81);
		    CLR(cy->cy_flags, CYF_START_BREAK);
		}
		if(ISSET(cy->cy_flags, CYF_END_BREAK)) {
		    cd_write_reg(cy, CD1400_TDR, 0);
		    cd_write_reg(cy, CD1400_TDR, 0x83);
		    CLR(cy->cy_flags, CYF_END_BREAK);
		}
	    }

	    if(tp->t_outq.c_cc == 0) {
txdone:
		/*
		 * No data to send or requested to stop.
		 * Disable transmit interrupt
		 */
		cd_write_reg(cy, CD1400_SRER,
			     cd_read_reg(cy, CD1400_SRER)
			     & ~CD1400_SRER_TXRDY);
		CLR(cy->cy_flags, CYF_STOP);
		CLR(tp->t_state, TS_BUSY);
	    }

	    if(tp->t_outq.c_cc <= tp->t_lowat) {
		SET(cy->cy_flags, CYF_START);
		cy_events = 1;
	    }

	    /* terminate service context */
	    cd_write_reg(cy, CD1400_TIR, save_tir & 0x3f);
	    cd_write_reg(cy, CD1400_CAR, save_car);
	    int_serviced = 1;
	} /* if(tx_service...) */
    } /* for(...all CD1400s on a card) */

    /* ensure an edge for next interrupt */
    bus_space_write_1(sc->sc_memt, sc->sc_memh,
		    CY_CLEAR_INTR<<sc->sc_bustype, 0);
    return int_serviced;
}

/*
 * subroutine to enable CD1400 transmitter
 */
void
cy_enable_transmitter(cy)
     struct cy_port *cy;
{
    int s;
    s = splhigh();
    cd_write_reg(cy, CD1400_CAR, cy->cy_port_num & CD1400_CAR_CHAN);
    cd_write_reg(cy, CD1400_SRER, cd_read_reg(cy, CD1400_SRER)
		 | CD1400_SRER_TXRDY);
    splx(s);
}

/*
 * Execute a CD1400 channel command
 */
void
cd1400_channel_cmd(cy, cmd)
     struct cy_port *cy;
     int cmd;
{
    u_int waitcnt = 5 * 8 * 1024; /* approx 5 ms */

#ifdef CY_DEBUG
    printf("c1400_channel_cmd cy 0x%x command 0x%x\n", cy, cmd);
#endif

    /* wait until cd1400 is ready to process a new command */
    while(cd_read_reg(cy, CD1400_CCR) != 0 && waitcnt-- > 0)
      ;

    if(waitcnt == 0)
      log(LOG_ERR, "cy: channel command timeout\n");

    cd_write_reg(cy, CD1400_CCR, cmd);
}

/*
 * Compute clock option register and baud rate register values
 * for a given speed. Return 0 on success, -1 on failure.
 *
 * The error between requested and actual speed seems
 * to be well within allowed limits (less than 3%)
 * with every speed value between 50 and 150000 bps.
 */
int
cy_speed(speed_t speed, int *cor, int *bpr)
{
    int c, co, br;

    if(speed < 50 || speed > 150000)
      return -1;

    for(c = 0, co = 8; co <= 2048; co <<= 2, c++) {
	br = (CY_CLOCK + (co * speed) / 2) / (co * speed);
	if(br < 0x100) {
	    *bpr = br;
	    *cor = c;
	    return 0;
	}
    }

    return -1;
}

#endif /* NCY > 0 */
