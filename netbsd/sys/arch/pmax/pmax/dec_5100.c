/*	$NetBSD: dec_5100.c,v 1.4 1999/03/15 11:45:16 nisimura Exp $	*/

/*
 * Copyright (c) 1998 Jonathan Stone.  All rights reserved.
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
 *	This product includes software developed by Jonathan Stone for
 *      the NetBSD Project.
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

/*
 *  Prototypes
 */


/* 
 * Local declarations
 */
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <machine/cpu.h>
#include <machine/intr.h>
#include <machine/reg.h>
#include <machine/psl.h>
#include <machine/locore.h>
#include <machine/autoconf.h>		/* intr_arg_t */
#include <machine/sysconf.h>		/* intr_arg_t */

#include <mips/mips_param.h>		/* hokey spl()s */
#include <mips/mips/mips_mcclock.h>	/* mcclock CPUspeed estimation */

#include <pmax/pmax/clockreg.h>
#include <pmax/pmax/turbochannel.h> 
#include <pmax/pmax/pmaxtype.h> 

#include <pmax/pmax/machdep.h>		/* XXXjrs replace with vectors */

#include <pmax/pmax/kn01.h>		/* common definitions */
#include <pmax/pmax/kn230.h>
#include <pmax/ibus/ibusvar.h>


/*
 * Forward declarations
 */
void		dec_5100_init __P((void));
void		dec_5100_os_init __P((void));
void		dec_5100_bus_reset __P((void));

void		dec_5100_enable_intr 
		   __P ((u_int slotno, int (*handler) __P((intr_arg_t sc)),
			 intr_arg_t sc, int onoff));
int		dec_5100_intr __P((u_int mask, u_int pc, 
			      u_int statusReg, u_int causeReg));

void		dec_5100_cons_init __P((void));
void		dec_5100_device_register __P((struct device *, void *));

void		dec_5100_memintr __P((void));

void	dec_5100_intr_establish __P((void * cookie, int level,
			 int (*handler) __P((intr_arg_t)), intr_arg_t arg));
void	dec_5100_intr_disestablish __P((struct ibus_attach_args *ia));

extern void dec_mips1_wbflush __P((void));


/*
 * Fill in platform struct. 
 */
void
dec_5100_init()
{

	platform.iobus = "ibus";

	platform.os_init = dec_5100_os_init;
	platform.bus_reset = dec_5100_bus_reset;
	platform.cons_init = dec_5100_cons_init;
	platform.device_register = dec_5100_device_register;

	dec_5100_os_init();
	sprintf(cpu_model, "DECsystem 5100 (MIPSMATE)");
}

void
dec_5100_os_init()
{

	/* set correct wbflush routine for this motherboard */
	 mips_set_wbflush(dec_mips1_wbflush);

	/*
	 * Set up interrupt handling and I/O addresses.
	 */
	mips_hardware_intr = dec_5100_intr;
	tc_enable_interrupt = dec_5100_enable_intr; /*XXX*/

	/* NB: note inversion, ether, disk on hard int 1, tty on hard int 0 */
	Mach_splbio = Mach_spl1;	/* just block hard int 1 */
	Mach_splnet = Mach_spl1;	/* just block hard int 1 */
	Mach_spltty = cpu_spl1;		/* block hard int 0 and 1 */
	Mach_splimp = cpu_spl3;		/* block 0,1,2 */
					/* XXX blocks out reset button? */
	Mach_splclock = cpu_spl2;
	Mach_splstatclock = cpu_spl2;

	mcclock_addr = (volatile struct chiptime *)
		MIPS_PHYS_TO_KSEG1(KN01_SYS_CLOCK);
	mc_cpuspeed(mcclock_addr, MIPS_INT_MASK_2);
}


/*
 * Initalize the memory system and I/O buses.
 */
void
dec_5100_bus_reset()
{
	register volatile u_int *icsr_addr =
	   (volatile u_int *)MIPS_PHYS_TO_KSEG1(KN230_SYS_ICSR);

	*icsr_addr |= KN230_CSR_INTR_WMERR ;

	/* nothing else to do */
	dec_mips1_wbflush();
}

void
dec_5100_cons_init()
{
}


void
dec_5100_device_register(dev, aux)
	struct device *dev;
	void *aux;
{
	panic("dec_5100_device_register unimplemented");
}


/*
 * Enable an interrupt from a slot on the KN01 internal bus.
 *
 * The 4.4bsd kn01 interrupt handler hard-codes r3000 CAUSE register
 * bits to particular device interrupt handlers.  We may choose to store
 * function and softc pointers at some future point.
 */
void
dec_5100_enable_intr(slotno, handler, sc, on)
	register unsigned int slotno;
	int (*handler) __P((void* softc));
	void *sc;
	int on;
{
	/*
	 */
	if (on) {
		tc_slot_info[slotno].intr = handler;
		tc_slot_info[slotno].sc = sc;
	} else {
		tc_slot_info[slotno].intr = 0;
		tc_slot_info[slotno].sc = 0;
	}
}

void
dec_5100_intr_establish(cookie, level, handler, arg)
	void * cookie;
	int level;
	int (*handler) __P((intr_arg_t));
	intr_arg_t arg;
{
	register int slotno = (int) cookie;

	tc_slot_info[slotno].intr = handler;
	tc_slot_info[slotno].sc = arg;
}

void
dec_5100_intr_disestablish(struct ibus_attach_args *ia)
{
	printf("dec_5100_intr_distestablish: not implemented\n");
}




/*
 * Handle mipsmate (DECstation 5100) interrupts.
 */
int
dec_5100_intr(mask, pc, statusReg, causeReg)
	unsigned mask;
	unsigned pc;
	unsigned statusReg;
	unsigned causeReg;
{
	register volatile struct chiptime *c = 
	    (volatile struct chiptime *)MIPS_PHYS_TO_KSEG1(KN01_SYS_CLOCK);
	struct clockframe cf;
	int temp;
	u_int icsr;

	if (mask & MIPS_INT_MASK_4) {
#ifdef DDB
		Debugger();
#else
		prom_haltbutton();
#endif
	}

	icsr = *((volatile u_int *)MIPS_PHYS_TO_KSEG1(KN230_SYS_ICSR));

	/* handle clock interrupts ASAP */
	if (mask & MIPS_INT_MASK_2) {
		temp = c->regc;	/* XXX clear interrupt bits */
		cf.pc = pc;
		cf.sr = statusReg;
		hardclock(&cf);
		intrcnt[HARDCLOCK]++;

		/* keep clock interrupts enabled when we return */
		causeReg &= ~MIPS_INT_MASK_2;
	}

	/* If clock interrupts were enabled, re-enable them ASAP. */
	splx(MIPS_SR_INT_ENA_CUR | (statusReg & MIPS_INT_MASK_2));

#define CALLINTR(slot, icnt) \
	if (tc_slot_info[slot].intr) {					\
		(*tc_slot_info[slot].intr) (tc_slot_info[slot].sc);	\
		intrcnt[(icnt)]++;					\
	}

	if (mask & MIPS_INT_MASK_0) {
		if (icsr & KN230_CSR_INTR_DZ0) {
		    CALLINTR(1, SERIAL0_INTR);
		}		    
		if (icsr & KN230_CSR_INTR_OPT0)
		    CALLINTR(5, SERIAL0_INTR);
		if (icsr & KN230_CSR_INTR_OPT1)
		    CALLINTR(6, SERIAL0_INTR);
	}

	if (mask & MIPS_INT_MASK_1) {
		if (icsr & KN230_CSR_INTR_LANCE) {
			CALLINTR(2, LANCE_INTR);
		}
		if (icsr & KN230_CSR_INTR_SII) {
			CALLINTR(3, SCSI_INTR);
		}
	}
#undef CALLINTR

	if (mask & MIPS_INT_MASK_3) {
		dec_5100_memintr();
		intrcnt[ERROR_INTR]++;
	}

	return ((statusReg & ~causeReg & MIPS_HARD_INT_MASK) |
		MIPS_SR_INT_ENA_CUR);
}


/*
 * Handle  write-to-nonexistent-address memory errors  on MIPS_INT_MASK_3.
 * These are reported asynchronously, due to hardware write buffering.
 * we can't easily figure out process context, so just panic.
 *
 * XXX drain writebuffer on contextswitch to avoid panic?
 */
void
dec_5100_memintr()
{
	register volatile u_int icsr;
	register volatile u_int *icsr_addr = 
		(volatile u_int *)MIPS_PHYS_TO_KSEG1(KN230_SYS_ICSR);
	extern int cold;

	/* read icsr and clear error  */
	icsr = *icsr_addr;
	*icsr_addr = icsr | KN230_CSR_INTR_WMERR;
	dec_mips1_wbflush();
	
#ifdef DIAGNOSTIC
		printf("\nMemory interrupt\n");
#endif

	/* ignore errors during probes */
	if (cold)
		return;

	if (icsr & KN230_CSR_INTR_WMERR) {
		panic("write to non-existant memory");
	}
	else {
		panic("stray memory error interrupt");
	}
}
