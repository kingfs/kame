/*-
 * Copyright (c) 2004 Jung-uk Kim
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/pci/agp_amd64.c,v 1.1 2004/08/16 12:25:48 obrien Exp $");

#include "opt_bus.h"
#ifdef __i386__
#include "opt_agp.h"
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <pci/agppriv.h>
#include <pci/agpreg.h>

#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/pmap.h>
#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

/* XXX */
extern void pci_cfgregwrite(int, int, int, int, uint32_t, int);
extern uint32_t pci_cfgregread(int, int, int, int, int);

MALLOC_DECLARE(M_AGP);

#define	AMD64_MAX_MCTRL		8

struct agp_amd64_softc {
	struct agp_softc	agp;
	uint32_t	initial_aperture; /* aperture size at startup */
	struct agp_gatt	*gatt;
	int		mctrl[AMD64_MAX_MCTRL];
	int		n_mctrl;
};

static const char*
agp_amd64_match(device_t dev)
{
	if (pci_get_class(dev) != PCIC_BRIDGE
	    || pci_get_subclass(dev) != PCIS_BRIDGE_HOST)
		return NULL;

	if (agp_find_caps(dev) == 0)
		return NULL;

	switch (pci_get_devid(dev)) {
	case 0x74541022:
		return ("AMD 8151 AGP graphics tunnel");
	case 0x10221039:
		return ("SiS 755 host to AGP bridge");
	case 0x02041106:
		return ("VIA 8380 host to PCI bridge");
	case 0x31881106:
		return ("VIA 8385 host to PCI bridge");
	};

	return NULL;
}

static int
agp_amd64_probe(device_t dev)
{
	const char *desc;

	if (resource_disabled("agp", device_get_unit(dev)))
		return ENXIO;
	if ((desc = agp_amd64_match(dev))) {
		device_verbose(dev);
		device_set_desc(dev, desc);
		return 0;
	}

	return ENXIO;
}

static int
agp_amd64_attach(device_t dev)
{
	struct agp_amd64_softc *sc = device_get_softc(dev);
	struct agp_gatt *gatt;
	int i, n, error;

	for (i = 0, n = 0; i < PCI_SLOTMAX && n < AMD64_MAX_MCTRL; i++)
		if (pci_cfgregread(0, i, 3, 0, 4) == 0x11031022) {
			sc->mctrl[n] = i;
			n++;
		}

	if (n == 0)
		return ENXIO;

	sc->n_mctrl = n;

	if (bootverbose)
		printf("AMD64: %d Misc. Control unit(s) found.\n", sc->n_mctrl);

	if ((error = agp_generic_attach(dev)))
		return error;

	sc->initial_aperture = AGP_GET_APERTURE(dev);

	for (;;) {
		gatt = agp_alloc_gatt(dev);
		if (gatt)
			break;

		/*
		 * Probably contigmalloc failure. Try reducing the
		 * aperture so that the gatt size reduces.
		 */
		if (AGP_SET_APERTURE(dev, AGP_GET_APERTURE(dev) / 2)) {
			agp_generic_detach(dev);
			return ENOMEM;
		}
	}
	sc->gatt = gatt;

	/* Install the gatt and enable aperture. */
	for (i = 0; i < sc->n_mctrl; i++) {
		pci_cfgregwrite(0, sc->mctrl[i], 3, AGP_AMD64_ATTBASE,
		    (uint32_t)(gatt->ag_physical >> 8) & AGP_AMD64_ATTBASE_MASK,
		    4);
		pci_cfgregwrite(0, sc->mctrl[i], 3, AGP_AMD64_APCTRL,
		    (pci_cfgregread(0, sc->mctrl[i], 3, AGP_AMD64_APCTRL, 4) |
		    AGP_AMD64_APCTRL_GARTEN) &
		    ~(AGP_AMD64_APCTRL_DISGARTCPU | AGP_AMD64_APCTRL_DISGARTIO),
		    4);
	}

	agp_flush_cache();

	return 0;
}

static int
agp_amd64_detach(device_t dev)
{
	struct agp_amd64_softc *sc = device_get_softc(dev);
	int i, error;

	if ((error = agp_generic_detach(dev)))
		return error;

	for (i = 0; i < sc->n_mctrl; i++)
		pci_cfgregwrite(0, sc->mctrl[i], 3, AGP_AMD64_APCTRL,
		    pci_cfgregread(0, sc->mctrl[i], 3, AGP_AMD64_APCTRL, 4) &
		    ~AGP_AMD64_APCTRL_GARTEN, 4);

	AGP_SET_APERTURE(dev, sc->initial_aperture);
	agp_free_gatt(sc->gatt);

	return 0;
}

static uint32_t agp_amd64_table[] = {
	0x02000000,	/*   32 MB */
	0x04000000,	/*   64 MB */
	0x08000000,	/*  128 MB */
	0x10000000,	/*  256 MB */
	0x20000000,	/*  512 MB */
	0x40000000,	/* 1024 MB */
	0x80000000,	/* 2048 MB */
};

#define AGP_AMD64_TABLE_SIZE \
	(sizeof(agp_amd64_table) / sizeof(agp_amd64_table[0]))

static uint32_t
agp_amd64_get_aperture(device_t dev)
{
	struct agp_amd64_softc *sc = device_get_softc(dev);
	uint32_t i;

	i = (pci_cfgregread(0, sc->mctrl[0], 3, AGP_AMD64_APCTRL, 4) &
		AGP_AMD64_APCTRL_SIZE_MASK) >> 1;

	if (i >= AGP_AMD64_TABLE_SIZE)
		return 0;

	return (agp_amd64_table[i]);
}

static int
agp_amd64_set_aperture(device_t dev, uint32_t aperture)
{
	struct agp_amd64_softc *sc = device_get_softc(dev);
	uint32_t i;
	int j;

	for (i = 0; i < AGP_AMD64_TABLE_SIZE; i++)
		if (agp_amd64_table[i] == aperture)
			break;
	if (i == AGP_AMD64_TABLE_SIZE)
		return EINVAL;

	for (j = 0; j < sc->n_mctrl; j++)
		pci_cfgregwrite(0, sc->mctrl[j], 3, AGP_AMD64_APCTRL,
		    (pci_cfgregread(0, sc->mctrl[j], 3, AGP_AMD64_APCTRL, 4) &
		    ~(AGP_AMD64_APCTRL_SIZE_MASK)) | (i << 1), 4);

	return 0;
}

static int
agp_amd64_bind_page(device_t dev, int offset, vm_offset_t physical)
{
	struct agp_amd64_softc *sc = device_get_softc(dev);

	if (offset < 0 || offset >= (sc->gatt->ag_entries << AGP_PAGE_SHIFT))
		return EINVAL;

	sc->gatt->ag_virtual[offset >> AGP_PAGE_SHIFT] = physical;
	return 0;
}

static int
agp_amd64_unbind_page(device_t dev, int offset)
{
	struct agp_amd64_softc *sc = device_get_softc(dev);

	if (offset < 0 || offset >= (sc->gatt->ag_entries << AGP_PAGE_SHIFT))
		return EINVAL;

	sc->gatt->ag_virtual[offset >> AGP_PAGE_SHIFT] = 0;
	return 0;
}

static void
agp_amd64_flush_tlb(device_t dev)
{
	struct agp_amd64_softc *sc = device_get_softc(dev);
	int i;

	for (i = 0; i < sc->n_mctrl; i++)
		pci_cfgregwrite(0, sc->mctrl[i], 3, AGP_AMD64_CACHECTRL,
		    pci_cfgregread(0, sc->mctrl[i], 3, AGP_AMD64_CACHECTRL, 4) |
		    AGP_AMD64_CACHECTRL_INVGART, 4);
}

static device_method_t agp_amd64_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		agp_amd64_probe),
	DEVMETHOD(device_attach,	agp_amd64_attach),
	DEVMETHOD(device_detach,	agp_amd64_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD(device_suspend,	bus_generic_suspend),
	DEVMETHOD(device_resume,	bus_generic_resume),

	/* AGP interface */
	DEVMETHOD(agp_get_aperture,	agp_amd64_get_aperture),
	DEVMETHOD(agp_set_aperture,	agp_amd64_set_aperture),
	DEVMETHOD(agp_bind_page,	agp_amd64_bind_page),
	DEVMETHOD(agp_unbind_page,	agp_amd64_unbind_page),
	DEVMETHOD(agp_flush_tlb,	agp_amd64_flush_tlb),
	DEVMETHOD(agp_enable,		agp_generic_enable),
	DEVMETHOD(agp_alloc_memory,	agp_generic_alloc_memory),
	DEVMETHOD(agp_free_memory,	agp_generic_free_memory),
	DEVMETHOD(agp_bind_memory,	agp_generic_bind_memory),
	DEVMETHOD(agp_unbind_memory,	agp_generic_unbind_memory),

	{ 0, 0 }
};

static driver_t agp_amd64_driver = {
	"agp",
	agp_amd64_methods,
	sizeof(struct agp_amd64_softc),
};

static devclass_t agp_devclass;

DRIVER_MODULE(agp_amd64, pci, agp_amd64_driver, agp_devclass, 0, 0);
MODULE_DEPEND(agp_amd64, agp, 1, 1, 1);
MODULE_DEPEND(agp_amd64, pci, 1, 1, 1);
