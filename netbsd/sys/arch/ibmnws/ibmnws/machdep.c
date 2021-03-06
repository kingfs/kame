/*	$NetBSD: machdep.c,v 1.2 2003/10/20 00:12:10 matt Exp $	*/

/*
 * Copyright (C) 1995, 1996 Wolfgang Solfrank.
 * Copyright (C) 1995, 1996 TooLs GmbH.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by TooLs GmbH.
 * 4. The name of TooLs GmbH may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TOOLS GMBH ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TOOLS GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "opt_compat_netbsd.h"
#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/exec.h>
#include <sys/extent.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/mount.h>
#include <sys/msgbuf.h>
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/sa.h>
#include <sys/syscallargs.h>
#include <sys/syslog.h>
#include <sys/systm.h>
#include <sys/user.h>

#include <uvm/uvm_extern.h>

#include <sys/sysctl.h>

#include <net/netisr.h>

#include <machine/autoconf.h>
#include <machine/bus.h>
#include <machine/intr.h>
#include <machine/pmap.h>
#include <machine/powerpc.h>
#include <machine/trap.h>

#include <powerpc/oea/bat.h>

#include <dev/cons.h>

#include "com.h"
#if (NCOM > 0)
#include <sys/termios.h>
#include <dev/ic/comreg.h>
#include <dev/ic/comvar.h>
void comsoft(void);
#endif

#ifdef DDB
#include <machine/db_machdep.h>
#include <ddb/db_extern.h>
#endif

void initppc(u_long, u_long, u_int, void *);
void dumpsys(void);
void strayintr(int);
int lcsplx(int);
void ibmnws_bus_space_init(void);

vaddr_t prep_intr_reg;			/* PReP interrupt vector register */

#define	OFMEMREGIONS	32
struct mem_region physmemr[OFMEMREGIONS], availmemr[OFMEMREGIONS];

paddr_t avail_end;			/* XXX temporary */

#ifdef DDB
extern void *endsym, *startsym;
#endif

void
initppc(startkernel, endkernel, args, btinfo)
	u_long startkernel, endkernel;
	u_int args;
	void *btinfo;
{

	/*
	 * Set memory region
	 */
	{
		u_long memsize;

#if 0
		/* Get the memory size from the PCI host bridge */

		pci_read_config_32(0, 0x90, &ea);
		if(ea & 0xff00)
		    memsize = (((ea >> 8) & 0xff) + 1) << 20;
		else
		    memsize = ((ea & 0xff) + 1) << 20;
#else
		memsize = 64 * 1024 * 1024;         /* 64MB hardcoded for now */
#endif

		physmemr[0].start = 0;
		physmemr[0].size = memsize & ~PGOFSET;
		availmemr[0].start = (endkernel + PGOFSET) & ~PGOFSET;
		availmemr[0].size = memsize - availmemr[0].start;
	}
	avail_end = physmemr[0].start + physmemr[0].size;    /* XXX temporary */

	/*
	 * Set CPU clock
	 */
	{
		extern u_long ticks_per_sec, ns_per_tick;

		ticks_per_sec = 16666666;		/* hardcoded */
		ns_per_tick = 1000000000 / ticks_per_sec;
	}

	/* Initialize the CPU type */
	/* ident_platform(); */

	/*
	 * boothowto
	 */
	boothowto = 0;		/* XXX - should make this an option */

	/*
	 * Initialize bus_space.
	 */
	ibmnws_bus_space_init();

/* JG  - OK to here */
	/*
	 * Now setup fixed bat registers
	 */
	oea_batinit(
	    PREP_BUS_SPACE_MEM, BAT_BL_256M,
	    PREP_BUS_SPACE_IO,  BAT_BL_256M,
	    0);

	/*
	 * i386 port says, that this shouldn't be here,
	 * but I really think the console should be initialized
	 * as early as possible.
	 */
	consinit();

/* JG - Fails here */

	oea_init(NULL);

	/*
	 * external interrupt handler install
	 */

	/* init_intr_ivr(); */
	init_intr();

        /*
	 * Set the page size.
	 */
	uvm_setpagesize();

	/*
	 * Initialize pmap module.
	 */
	pmap_bootstrap(startkernel, endkernel);

#ifdef DDB
	ddb_init((int)((u_long)endsym - (u_long)startsym), startsym, endsym);

	if (boothowto & RB_KDB)
		Debugger();
#endif
}

void
mem_regions(mem, avail)
	struct mem_region **mem, **avail;
{

	*mem = physmemr;
	*avail = availmemr;
}

/*
 * Machine dependent startup code.
 */
void
cpu_startup()
{
	/*
	 * Mapping PReP interrput vector register.
	 */
	prep_intr_reg = (vaddr_t) mapiodev(PREP_INTR_REG, PAGE_SIZE);
	if (!prep_intr_reg)
		panic("startup: no room for interrupt register");

	/*
	 * Do common startup.
	 */
	oea_startup("IBM NetworkStation 1000 (8362-XXX)");

	/*
	 * Now allow hardware interrupts.
	 */
	{
		int msr;

		splraise(-1);
		__asm __volatile ("mfmsr %0; ori %0,%0,%1; mtmsr %0"
			      : "=r"(msr) : "K"(PSL_EE));
	}

	/*
	 * Now safe for bus space allocation to use malloc.
	 */
	bus_space_mallocok();
}

/*
 * Soft tty interrupts.
 */
void
softserial(void)
{
#if (NCOM > 0)
	comsoft();
#endif
}

/*
 * Stray interrupts.
 */
void
strayintr(int irq)
{
	log(LOG_ERR, "stray interrupt %d\n", irq);
}

/*
 * Halt or reboot the machine after syncing/dumping according to howto.
 */
void
cpu_reboot(howto, what)
	int howto;
	char *what;
{
	static int syncing;

	if (cold) {
		howto |= RB_HALT;
		goto halt_sys;
	}

	boothowto = howto;
	if ((howto & RB_NOSYNC) == 0 && syncing == 0) {
		syncing = 1;
		vfs_shutdown();		/* sync */
		resettodr();		/* set wall clock */
	}

	/* Disable intr */
	splhigh();

	/* Do dump if requested */
	if ((howto & (RB_DUMP | RB_HALT)) == RB_DUMP)
		oea_dumpsys();

halt_sys:
	doshutdownhooks();

	if (howto & RB_HALT) {
                printf("\n");
                printf("The operating system has halted.\n");
                printf("Please press any key to reboot.\n\n");
                cnpollc(1);	/* for proper keyboard command handling */
                cngetc();
                cnpollc(0);
	}

	printf("rebooting...\n\n");


        {
	    int msr;
	    u_char reg;

	    __asm__ volatile("mfmsr %0" : "=r"(msr));
	    msr |= PSL_IP;
	    __asm__ volatile("mtmsr %0" :: "r"(msr));

	    reg = *(volatile u_char *)(PREP_BUS_SPACE_IO + 0x92);
	    reg &= ~1UL;
	    *(volatile u_char *)(PREP_BUS_SPACE_IO + 0x92) = reg;

	    __asm__ volatile("sync; eieio\n");

	    reg = *(volatile u_char *)(PREP_BUS_SPACE_IO + 0x92);
	    reg |= 1;
	    *(volatile u_char *)(PREP_BUS_SPACE_IO + 0x92) = reg;

	    __asm__ volatile("sync; eieio\n");
	}

	for (;;)
		continue;
	/* NOTREACHED */
}

/*
 * lcsplx() is called from locore; it is an open-coded version of
 * splx() differing in that it returns the previous priority level.
 */
int
lcsplx(ipl)
	int ipl;
{
	int oldcpl;

	__asm__ volatile("sync; eieio\n");	/* reorder protect */
	oldcpl = cpl;
	cpl = ipl;
	if (ipending & ~ipl)
		do_pending_int();
	__asm__ volatile("sync; eieio\n");	/* reorder protect */

	return (oldcpl);
}

struct powerpc_bus_space io_bus_space_tag = {
	_BUS_SPACE_LITTLE_ENDIAN|_BUS_SPACE_IO_TYPE,
	0x80000000, 0x00000000, 0x3f800000,
};
struct powerpc_bus_space isa_io_bus_space_tag = {
	_BUS_SPACE_LITTLE_ENDIAN|_BUS_SPACE_IO_TYPE,
	0x80000000, 0x00000000, 0x00010000,
};
struct powerpc_bus_space mem_bus_space_tag = {
	_BUS_SPACE_LITTLE_ENDIAN|_BUS_SPACE_MEM_TYPE,
	0xC0000000, 0x00000000, 0x3f000000,
};
struct powerpc_bus_space isa_mem_bus_space_tag = {
	_BUS_SPACE_LITTLE_ENDIAN|_BUS_SPACE_MEM_TYPE,
	0xC0000000, 0x00000000, 0x01000000,
};

static char ex_storage[2][EXTENT_FIXED_STORAGE_SIZE(8)]
    __attribute__((aligned(8)));

void
ibmnws_bus_space_init(void)
{
	int error;

	error = bus_space_init(&io_bus_space_tag, "ioport",
	    ex_storage[0], sizeof(ex_storage[0]));
	if (error)
		panic("prep_bus_space_init: can't init io tag");

	error = extent_alloc_region(io_bus_space_tag.pbs_extent,
	    0x10000, 0x7F0000, EX_NOWAIT);
	if (error)
		panic("prep_bus_space_init: can't block out reserved I/O"
		    " space 0x10000-0x7fffff: error=%d", error);
	error = bus_space_init(&mem_bus_space_tag, "iomem",
	    ex_storage[1], sizeof(ex_storage[1]));
	if (error)
		panic("prep_bus_space_init: can't init mem tag");

	isa_io_bus_space_tag.pbs_extent = io_bus_space_tag.pbs_extent;
	error = bus_space_init(&isa_io_bus_space_tag, "isa-ioport", NULL, 0);
	if (error)
		panic("bus_space_init: can't init isa io tag");

	isa_mem_bus_space_tag.pbs_extent = mem_bus_space_tag.pbs_extent;
	error = bus_space_init(&isa_mem_bus_space_tag, "isa-iomem", NULL, 0);
	if (error)
		panic("bus_space_init: can't init isa mem tag");
}
