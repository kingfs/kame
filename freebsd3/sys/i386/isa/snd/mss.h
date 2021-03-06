/*
 * file: mss.h
 *
 * (C) 1997 Luigi Rizzo (luigi@iet.unipi.it)
 *
 * This file contains information and macro definitions for
 * AD1848-compatible devices, used in the MSS/WSS compatible boards.
 *
 */

/*
 *

The codec part of the board is seen as a set of 4 registers mapped
at the base address for the board (default 0x534). Note that some
(early) boards implemented 4 additional registers 4 location before
(usually 0x530) to store configuration information. This is a source
of confusion in that one never knows what address to specify. The
(current) convention is to use the old address (0x530) in the kernel
configuration file and consider MSS registers start four location
ahead.

 *
 */

/*
 * The four visible registers of the MSS :
 *
 */

#define io_Index_Addr(d)        ((d)->io_base + 4)
#define	IA_BUSY			0x80	/* readonly, set when busy */
#define	IA_MCE			0x40	/* the MCE bit. */
	/*
	 * the MCE bit must be set whenever the current mode of the
	 * codec is changed; this in particular is true for the
	 * Data Format (I8, I28) and Interface Config(I9) registers.
	 * Only exception are CEN and PEN which can be changed on the fly.
	 * The DAC output is muted when MCE is set.
	 */
#define	IA_TRD			0x20	/* Transfer request disable */
	/*
	 * When TRD is set, DMA transfers cease when the INT bit in
	 * the MSS status reg is set. Must be cleared for automode
	 * DMA, set otherwise.
	 */
#define	IA_AMASK		0x1f	/* mask for indirect address */

#define io_Indexed_Data(d)      ((d)->io_base+1+4)
	/*
	 * data to be transferred to the indirect register addressed
	 * by index addr. During init and sw. powerdown, cannot be
	 * written to, and is always read as 0x80 (consistent with the
	 * busy flag).
	 */

#define io_Status(d)            ((d)->io_base+2+4)

#define	IS_CUL		0x80	/* capture upper/lower */
#define	IS_CLR		0x40	/* capture left/right */
#define	IS_CRDY		0x20	/* capture ready for programmed i/o */
#define	IS_SER		0x10	/* sample error (overrun/underrun) */
#define	IS_PUL		0x08	/* playback upper/lower */
#define	IS_PLR		0x04	/* playback left/right */
#define	IS_PRDY		0x02	/* playback ready for programmed i/o */
#define	IS_INT		0x01	/* int status (1 = active) */
	/*
	 * IS_INT is clreared by any write to the status register.
	 */
	
#define io_Polled_IO(d)         ((d)->io_base+3+4)
	/*
	 * this register is used in case of polled i/o
	 */

/*
 * The MSS has a set of 16 (or 32 depending on the model) indirect
 * registers accessible through the data port by specifying the
 * appropriate address in the address register.
 *
 * The 16 low registers are uniformly handled in AD1848/CS4248 compatible
 * mode (often called MODE1). For the upper 16 registers there are
 * some differences among different products, mainly Crystal uses them
 * differently from OPTi.
 *
 */

/*
 * volume registers
 */

#define	I6_MUTE		0x80

/*
 * register I9 -- interface configuration.
 */

#define	I9_PEN		0x01	/* playback enable */
#define	I9_CEN		0x02	/* capture enable */

/*
 * values used in bd_flags
 */
#define	BD_F_MCE_BIT	0x0001
#define	BD_F_IRQ_OK	0x0002
#define	BD_F_TMR_RUN	0x0004

/* AD1816 register macros */

#define ad1816_ale(d)  ((d)->io_base+0) /* indirect reg access */
#define ad1816_int(d)  ((d)->io_base+1) /* interupt status     */
#define ad1816_low(d)  ((d)->io_base+2) /* indirect low byte   */
#define ad1816_high(d) ((d)->io_base+3) /* indirect high byte  */
/* unused */
#define ad1816_pioD(d) ((d)->io_base+4) /* PIO debug           */
#define ad1816_pios(d) ((d)->io_base+5) /* PIO status          */
#define ad1816_piod(d) ((d)->io_base+6) /* PIO data            */
/* end of unused */
/* values for playback/capture config:
   bits: 0   enable/disable
         1   pio/dma
         2   stereo/mono
         3   companded/linearPCM
         4-5 format : 00 8bit  linear (uncomp)
                      00 8bit  mulaw  (comp)
                      01 16bit le     (uncomp)
                      01 8bit  alaw   (comp)
                      11 16bit be     (uncomp)
*/
#define ad1816_play(d) ((d)->io_base+8) /* playback config     */
#define ad1816_capt(d) ((d)->io_base+9) /* capture config      */

#define	AD1816_BUSY	0x80	/* chip is busy			*/
#define	AD1816_ALEMASK	0x3F	/* mask for indirect adr.	*/
/* unusud */
#define	AD1816_INTRSI	0x01	/* sb intr			*/
#define	AD1816_INTRGI	0x02	/* game intr			*/
#define	AD1816_INTRRI	0x04	/* ring intr			*/
#define	AD1816_INTRDI	0x08	/* dsp intr			*/
#define	AD1816_INTRVI	0x10	/* vol intr			*/
#define	AD1816_INTRTI	0x20 	/* timer intr			*/
/* used again */
#define	AD1816_INTRCI	0x40	/* capture intr			*/
#define	AD1816_INTRPI	0x80	/* playback intr		*/
/* PIO stuff is not supplied here */
/* playback / capture config      */
#define	AD1816_ENABLE	0x01	/* enable pl/cp			*/
#define	AD1816_PIO	0x02	/* use pio			*/
#define	AD1816_STEREO	0x04
#define	AD1816_COMP	0x08	/* data is companded		*/
#define	AD1816_U8	0x00	/* 8 bit linear pcm		*/
#define	AD1816_MULAW	0x08	/* 8 bit mulaw			*/
#define	AD1816_ALAW	0x18	/* 8 bit alaw			*/
#define	AD1816_S16LE	0x10	/* 16 bit linear little endian	*/
#define	AD1816_S16BE	0x30	/* 16 bit linear big endian	*/
#define	AD1816_FORMASK  0x38	/* format mask			*/

/*
 * sound/ad1848_mixer.h
 * 
 * Definitions for the mixer of AD1848 and compatible codecs.
 * 
 * Copyright by Hannu Savolainen 1994
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer. 2.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * The AD1848 codec has generic input lines called Line, Aux1 and Aux2.
 * Soundcard manufacturers have connected actual inputs (CD, synth, line,
 * etc) to these inputs in different order. Therefore it's difficult
 * to assign mixer channels to to these inputs correctly. The following
 * contains two alternative mappings. The first one is for GUS MAX and
 * the second is just a generic one (line1, line2 and line3).
 * (Actually this is not a mapping but rather some kind of interleaving
 * solution).
 */

#define MSS_REC_DEVICES	\
    (SOUND_MASK_LINE | SOUND_MASK_MIC | SOUND_MASK_CD|SOUND_MASK_IMIX)


/*
 * Table of mixer registers. There is a default table for the
 * AD1848/CS423x clones, and one for the OPTI931. As more MSS
 * clones come out, there ought to be more tables.
 *
 * Fields in the table are : polarity, register, offset, bits
 *
 * The channel numbering used by individual soundcards is not fixed.
 * Some cards have assigned different meanings for the AUX1, AUX2
 * and LINE inputs. Some have different features...
 *
 * Following there is a macro ...MIXER_DEVICES which is a bitmap
 * of all non-zero fields in the table.
 * MODE1_MIXER_DEVICES is the basic mixer of the 1848 in mode 1
 * registers I0..I15)
 *
 */

mixer_ent mix_devices[32][2] = {
MIX_NONE(SOUND_MIXER_VOLUME),
MIX_NONE(SOUND_MIXER_BASS),
MIX_NONE(SOUND_MIXER_TREBLE),
MIX_ENT(SOUND_MIXER_SYNTH,	 2, 1, 0, 5,	 3, 1, 0, 5),
MIX_ENT(SOUND_MIXER_PCM,	 6, 1, 0, 6,	 7, 1, 0, 6),
MIX_ENT(SOUND_MIXER_SPEAKER,	26, 1, 0, 4,	 0, 0, 0, 0),
MIX_ENT(SOUND_MIXER_LINE,	18, 1, 0, 5,	19, 1, 0, 5),
MIX_ENT(SOUND_MIXER_MIC,	 0, 0, 5, 1,	 1, 0, 5, 1),
MIX_ENT(SOUND_MIXER_CD,	 	 4, 1, 0, 5,	 5, 1, 0, 5),
MIX_ENT(SOUND_MIXER_IMIX,	13, 1, 2, 6,	 0, 0, 0, 0),
MIX_NONE(SOUND_MIXER_ALTPCM),
MIX_NONE(SOUND_MIXER_RECLEV),
MIX_ENT(SOUND_MIXER_IGAIN,	 0, 0, 0, 4,	 1, 0, 0, 4),
MIX_NONE(SOUND_MIXER_OGAIN),
MIX_NONE(SOUND_MIXER_LINE1),
MIX_NONE(SOUND_MIXER_LINE2),
MIX_NONE(SOUND_MIXER_LINE3),
};

#define MODE2_MIXER_DEVICES	\
    (SOUND_MASK_SYNTH | SOUND_MASK_PCM    | SOUND_MASK_SPEAKER | \
     SOUND_MASK_LINE  | SOUND_MASK_MIC    | SOUND_MASK_CD      | \
     SOUND_MASK_IMIX  | SOUND_MASK_IGAIN                         )

#define MODE1_MIXER_DEVICES	\
    (SOUND_MASK_SYNTH | SOUND_MASK_PCM    | SOUND_MASK_MIC     | \
     SOUND_MASK_CD    | SOUND_MASK_IMIX   | SOUND_MASK_IGAIN     )


/*
 * entries for the opti931...
 */

mixer_ent opti931_devices[32][2] = {	/* for the opti931 */
MIX_ENT(SOUND_MIXER_VOLUME,	22, 1, 1, 5,	23, 1, 1, 5),
MIX_NONE(SOUND_MIXER_BASS),
MIX_NONE(SOUND_MIXER_TREBLE),
MIX_ENT(SOUND_MIXER_SYNTH,	 4, 1, 1, 4,	 5, 1, 1, 4),
MIX_ENT(SOUND_MIXER_PCM,	 6, 1, 0, 5,	 7, 1, 0, 5),
MIX_NONE(SOUND_MIXER_SPEAKER),
MIX_ENT(SOUND_MIXER_LINE,	18, 1, 1, 4,	19, 1, 1, 4),
MIX_ENT(SOUND_MIXER_MIC,	 0, 0, 5, 1,	 1, 0, 5, 1),
MIX_ENT(SOUND_MIXER_CD,	 	 2, 1, 1, 4,	 3, 1, 1, 4),
MIX_NONE(SOUND_MIXER_IMIX),
MIX_NONE(SOUND_MIXER_ALTPCM),
MIX_NONE(SOUND_MIXER_RECLEV),
MIX_ENT(SOUND_MIXER_IGAIN,	 0, 0, 0, 4,	 1, 0, 0, 4),
MIX_NONE(SOUND_MIXER_OGAIN),
MIX_ENT(SOUND_MIXER_LINE1, 	16, 1, 1, 4,	17, 1, 1, 4),
MIX_NONE(SOUND_MIXER_LINE2),
MIX_NONE(SOUND_MIXER_LINE3),
};

#define OPTI931_MIXER_DEVICES	\
    (SOUND_MASK_VOLUME | SOUND_MASK_SYNTH | SOUND_MASK_PCM | \
     SOUND_MASK_LINE   | SOUND_MASK_MIC   | SOUND_MASK_CD  | \
     SOUND_MASK_IGAIN  | SOUND_MASK_LINE1                    )

#define AD1816_REC_DEVICES	\
    (SOUND_MASK_LINE | SOUND_MASK_MIC | SOUND_MASK_CD)

#define AD1816_MIXER_DEVICES	\
    (SOUND_MASK_VOLUME | SOUND_MASK_PCM | SOUND_MASK_SYNTH | \
     SOUND_MASK_LINE   | SOUND_MASK_MIC | SOUND_MASK_CD | SOUND_MASK_IGAIN)

static u_short default_mixer_levels[SOUND_MIXER_NRDEVICES] = {
  0x5a5a,			/* Master Volume */
  0x3232,			/* Bass */
  0x3232,			/* Treble */
  0x4b4b,			/* FM */
  0x4040,			/* PCM */
  0x4b4b,			/* PC Speaker */
  0x2020,			/* Ext Line */
  0x4040,			/* Mic */
  0x4b4b,			/* CD */
  0x0000,			/* Recording monitor */
  0x4b4b,			/* SB PCM */
  0x4b4b,			/* Recording level */
  0x2525,			/* Input gain */
  0x0000,			/* Output gain */
  /*  0x4040,			Line1 */
  0x0000,			/* Line1 */
  0x0000,			/* Line2 */
  0x1515			/* Line3 (usually line in)*/
};

