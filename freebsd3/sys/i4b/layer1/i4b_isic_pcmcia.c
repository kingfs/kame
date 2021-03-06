/*
 *   Copyright (c) 1998 Matthias Apitz. All rights reserved.
 *
 *   Copyright (c) 1998 Hellmuth Michaelis. All rights reserved. 
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of the author nor the names of any co-contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *   4. Altered versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software and/or documentation.
 *   
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *   SUCH DAMAGE.
 *
 *---------------------------------------------------------------------------
 *
 *	i4b_isic_pcmcia.c - i4b FreeBSD PCMCIA support
 *	----------------------------------------------
 *
 *	$Id: i4b_isic_pcmcia.c,v 1.3 1999/01/19 00:21:45 peter Exp $
 *
 *      last edit-date: [Mon Dec 14 17:30:09 1998]
 *
 *---------------------------------------------------------------------------*/

#ifdef __FreeBSD__
 
#include "isic.h"
#include "opt_i4b.h"
#include "card.h"

#if (NISIC > 0) && (NCARD > 0)
 
#include "apm.h"
#include <sys/param.h>
#include <sys/select.h>
#include <i386/isa/isa_device.h>

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
#include <sys/ioccom.h>
#else
#include <sys/ioctl.h>
#endif

#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <net/if.h>
#include <machine/clock.h>
#include <i386/isa/isa_device.h>

#include <machine/i4b_debug.h>
#include <machine/i4b_ioctl.h>
#include <machine/i4b_trace.h>

#include <pccard/cardinfo.h>
#include <pccard/slot.h>
#include <pccard/driver.h>

#include <i4b/layer1/i4b_l1.h>
#include <i4b/layer1/i4b_isac.h>
#include <i4b/layer1/i4b_hscx.h>

#include <i4b/include/i4b_l1l2.h>
#include <i4b/include/i4b_mbuf.h>
#include <i4b/include/i4b_global.h>
 
extern int isicattach(struct isa_device *dev);
extern void isicintr(int unit);

/*  
 * PC-Card (PCMCIA) specific code.
 */
static int  isic_pccard_init    __P((struct pccard_devinfo *));
static void isic_unload         __P((struct pccard_devinfo *));
static int  isic_card_intr      __P((struct pccard_devinfo *));
    
PCCARD_MODULE(isic, isic_pccard_init, isic_unload, isic_card_intr, 0,net_imask);

/*
 * Initialize the device - called from Slot manager.
 */

static int opened = 0;			/* our cards status */

static int isic_pccard_init(devi)
struct pccard_devinfo *devi;
{   
    	struct isa_device *is = &devi->isahd;

	if ((1 << is->id_unit) & opened)
		return(EBUSY);

	opened |= 1 << is->id_unit;
	printf("isic%d: PCMCIA init, irqmask = 0x%x (%d), iobase = 0x%x\n",
                is->id_unit, is->id_irq, devi->slt->irq, is->id_iobase);

#if 0
	/* XXX: problems resolving isic_probe_avma1_pcmcia() /phk */
	/* 
	 * look if there is really an AVM PCMCIA Fritz!Card and
	 * setup the card specific stuff
	 */
	isic_probe_avma1_pcmcia(is);
#endif

	/* ap:
	 * XXX what's to do with the return value?
	 */

	/*
	 * try to attach the PCMCIA card as a normal A1 card
	 */
	isicattach(is);
	return(0);
}

static void isic_unload(devi)
struct pccard_devinfo *devi;
{   
    	struct isa_device *is = &devi->isahd;
	printf("isic%d: unloaded\n", is->id_unit);
	opened &= ~(1 << is->id_unit);
}

/*
 * card_intr - Shared interrupt called from
 * front end of PC-Card handler.
 */
static int isic_card_intr(devi)
struct pccard_devinfo *devi;
{   
	isicintr(devi->isahd.id_unit);
	return(1);
}   

#endif /* (NISIC > 0) && (NCARD > 0) */
#endif /* __FreeBSD__ */
