/*	$NetBSD: debug_subr.c,v 1.5 2003/07/15 02:29:27 lukem Exp $	*/

/*-
 * Copyright (c) 2001, 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by UCHIYAMA Yasushi.
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
__KERNEL_RCSID(0, "$NetBSD: debug_subr.c,v 1.5 2003/07/15 02:29:27 lukem Exp $");

#include "debug_hpc.h"

#include <sys/param.h>
#include <sys/systm.h>

#include <machine/debug.h>
#include <machine/bootinfo.h>

#define BANNER_LENGTH		80

static const char onoff[2] = "_x";

void
__dbg_bit_print(u_int32_t a, int len, int start, int end, char *title,
    int flags)
{
	u_int32_t j, j1;
	int i, n;
	char buf[64];

	n = len * NBBY - 1;
	j1 = 1 << n;
	end = end ? end : n;

	if (!(flags & DBG_BIT_PRINT_QUIET))
		printf(" ");
	if (title) {
		printf("[%-16s] ", title);
	}

	for (j = j1, i = n; j > 0; j >>=1, i--) {
		if (i > end || i < start) {
			printf("%c", a & j ? '+' : '-'); /* out of range */
		} else {
			printf("%c", a & j ? '|' : '.');
		}
	}

	if (!(flags & DBG_BIT_PRINT_QUIET)) {
		snprintf(buf, sizeof buf, " [0x%%0%dx %%12d]", len << 1);
		printf(buf, a, a);
	}

	if (flags & DBG_BIT_PRINT_COUNT) {
		for (j = j1, i = n; j > 0; j >>=1, i--) {
			if (!(i > end || i < start) && (a & j)) {
				printf(" %d", i);
			}
		}
	}

	if (!(flags & DBG_BIT_PRINT_QUIET))
		printf("\n");
}

void
dbg_bitmask_print(u_int32_t reg, u_int32_t mask, const char *name)
{

	printf("%s[%c] ", name, onoff[reg & mask ? 1 : 0]);
}

void
dbg_banner_title(const char *name, size_t len)
{
	int n = (BANNER_LENGTH - (len + 2)) >> 1;

	dbg_draw_line(n);
	printf("[%s]", name);
	dbg_draw_line(n);
	printf("\n");
}

void
dbg_banner_line()
{

	dbg_draw_line(BANNER_LENGTH);
	printf("\n");
}

void
dbg_draw_line(int n)
{
	int i;

	for (i = 0; i < n; i++)
		printf("-");
}

#ifdef HPC_DEBUG_LCD
/*
 * LCD test routines for 8bpp, 16bpp LCD.
 */
static const u_int8_t img[1024] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xd5,0xff,0xc3,0xff,0xd5,0xff,0xc3,
	0xff,0xd5,0xff,0xc3,0xff,0xd5,0xff,0xc3,
	0xff,0xd5,0xff,0xc3,0xff,0xd5,0xc3,0xd5,
	0xff,0xc3,0xff,0xd5,0xff,0xc3,0xff,0xd5,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0x82,0x82,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc3,
	0xff,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xb9,0x82,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb9,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xd5,0x82,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xd5,
	0xff,0xb9,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xc3,0x82,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc3,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xd5,0x82,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xd5,
	0xff,0xd5,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xb9,0xd5,0xc3,0xd5,
	0xff,0xff,0xff,0xff,0xff,0xff,0x3d,0x82,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb9,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xd5,0x82,0x82,0x82,0x82,0x82,
	0x82,0x82,0xd5,0xff,0xff,0xff,0xd5,0x82,
	0xff,0xff,0xd5,0xc3,0xff,0xff,0xff,0xc3,
	0xff,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0x82,0x82,0xff,0xff,0xff,0xff,
	0xd5,0x3e,0x82,0x82,0x82,0xc3,0x82,0x3d,
	0xff,0xc3,0x82,0x3d,0xff,0xff,0xff,0xd5,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xd5,0x82,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x82,0x82,0x82,0x3d,
	0x82,0x82,0xd5,0xff,0xff,0xff,0xff,0xb9,
	0xff,0xb9,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x82,0x82,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xd5,0x82,0x82,0x82,
	0x82,0x82,0xb9,0xff,0xff,0xff,0xff,0xc3,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x82,0xd5,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0x82,0x82,0x82,
	0xd5,0xff,0x82,0xd5,0xff,0xff,0xff,0xd5,
	0xff,0xd5,0xff,0xff,0xff,0xff,0xff,0xff,
	0xd5,0x82,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0x82,0x82,0x3e,
	0xff,0xff,0xc3,0x82,0xff,0xff,0xff,0xb9,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x82,0x3e,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xd5,0x82,0xc3,
	0xff,0xff,0xff,0x82,0xf8,0xff,0xff,0xd5,
	0xff,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,
	0x82,0xd5,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x3e,0x82,0x82,0x82,
	0x80,0x3d,0xff,0x3d,0x82,0xff,0xff,0xc3,
	0xff,0xff,0xff,0xff,0xd5,0xc3,0xd5,0xb9,
	0x82,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x82,0x82,0xd5,0xff,
	0x82,0xd5,0xff,0xd5,0x82,0xff,0xff,0xb9,
	0xff,0xb9,0xff,0x82,0x82,0x82,0x82,0x82,
	0x82,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0x82,0x82,0x82,0x82,0x3d,
	0x82,0xb9,0xff,0xc3,0x82,0xff,0xff,0xd5,
	0xff,0xff,0xd5,0x82,0xd5,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0x82,0x82,0xd5,0x3e,0x82,
	0x82,0xc3,0xff,0xb9,0x82,0xff,0xff,0xc3,
	0xff,0xc3,0x3e,0x82,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xd5,0x82,0xff,0xff,0xb9,
	0xff,0xff,0x82,0x3d,0xff,0xff,0x3e,0xd5,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xc3,0x82,0xff,0xff,0xd5,
	0xff,0xd5,0x82,0xd5,0xff,0xc3,0x82,0xff,
	0xc3,0xd5,0xb9,0xd5,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xc3,0xd5,
	0xb9,0xff,0xff,0xb9,0x82,0xff,0xff,0xc3,
	0xff,0xb9,0x82,0xb9,0xff,0x82,0x3d,0xff,
	0x82,0x82,0x82,0x82,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xd5,0x82,0x82,
	0x82,0x82,0xff,0xd5,0x82,0xff,0xff,0xd5,
	0xff,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
	0x82,0x82,0x82,0x82,0x82,0xc3,0xb9,0xd5,
	0xc3,0xd5,0xff,0xff,0xc3,0x82,0x82,0x82,
	0x82,0x82,0xb9,0xc3,0x82,0xff,0xff,0xb9,
	0xff,0xd5,0xc3,0xd5,0xb9,0x82,0x3d,0xd5,
	0x82,0x82,0x3e,0x82,0x82,0x82,0x82,0x82,
	0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x3d,
	0x82,0x82,0x82,0x82,0x3d,0xff,0xff,0xc3,
	0xff,0xff,0xff,0xff,0xff,0xd5,0x82,0xc3,
	0x82,0xd5,0xff,0x82,0xd5,0xff,0xff,0xff,
	0xff,0xd5,0xb9,0xd5,0x82,0x82,0x82,0xd5,
	0x82,0x82,0x82,0xff,0xd5,0xff,0xff,0xd5,
	0xff,0xb9,0xff,0xff,0xff,0xff,0x82,0x82,
	0x82,0xc3,0x82,0x82,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0x82,0x82,0x82,
	0x82,0x82,0x82,0xff,0xff,0xff,0xff,0xb9,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3d,
	0x82,0x82,0x3d,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xd5,0x82,0x82,
	0x82,0x82,0xd5,0xff,0xff,0xff,0xff,0xc3,
	0xff,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc3,
	0xd5,0xff,0xff,0xff,0xff,0xff,0xff,0xd5,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xb9,
	0xff,0xd5,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc3,
	0xff,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
};

static const u_int16_t rgb565_clut[256] = {
	0x0000, 0x8000, 0x0400, 0x8400, 0x0080, 0x8080, 0x0480, 0x8480,
	0xc6e0, 0xa6f0, 0x29ea, 0x29ff, 0x2ae0, 0x2af5, 0x2aea, 0x2aff,
	0x2be0, 0x2bf5, 0x2bea, 0x2bff, 0x2ce0, 0x2cf5, 0x2cea, 0x2cff,
	0x2de0, 0x2df5, 0x2dea, 0x2dff, 0x2ee0, 0x2ef5, 0x2eea, 0x2eff,
	0x2fe0, 0x2ff5, 0x2fea, 0x2fff, 0x5000, 0x5055, 0x50aa, 0x50ff,
	0x50e0, 0x50f5, 0x50ea, 0x50ff, 0x51e0, 0x51f5, 0x51ea, 0x51ff,
	0x52e0, 0x52f5, 0x52ea, 0x52ff, 0x53e0, 0x53f5, 0x53ea, 0x53ff,
	0x54e0, 0x54f5, 0x54ea, 0x54ff, 0x55e0, 0x55f5, 0x55ea, 0x55ff,
	0x56e0, 0x56f5, 0x56ea, 0x56ff, 0x57e0, 0x57f5, 0x57ea, 0x57ff,
	0x7800, 0x7855, 0x78aa, 0x78ff, 0x78e0, 0x78f5, 0x78ea, 0x78ff,
	0x79e0, 0x79f5, 0x79ea, 0x79ff, 0x7ae0, 0x7af5, 0x7aea, 0x7aff,
	0x7be0, 0x7bf5, 0x7bea, 0x7bff, 0x7ce0, 0x7cf5, 0x7cea, 0x7cff,
	0x7de0, 0x7df5, 0x7dea, 0x7dff, 0x7ee0, 0x7ef5, 0x7eea, 0x7eff,
	0x7fe0, 0x7ff5, 0x7fea, 0x7fff, 0xa800, 0xa855, 0xa8aa, 0xa8ff,
	0xa8e0, 0xa8f5, 0xa8ea, 0xa8ff, 0xa9e0, 0xa9f5, 0xa9ea, 0xa9ff,
	0xaae0, 0xaaf5, 0xaaea, 0xaaff, 0xabe0, 0xabf5, 0xabea, 0xabff,
	0xace0, 0xacf5, 0xacea, 0xacff, 0xade0, 0xadf5, 0xadea, 0xadff,
	0xaee0, 0xaef5, 0xaeea, 0xaeff, 0xafe0, 0xaff5, 0xafea, 0xafff,
	0xd000, 0xd055, 0xd0aa, 0xd0ff, 0xd0e0, 0xd0f5, 0xd0ea, 0xd0ff,
	0xd1e0, 0xd1f5, 0xd1ea, 0xd1ff, 0xd2e0, 0xd2f5, 0xd2ea, 0xd2ff,
	0xd3e0, 0xd3f5, 0xd3ea, 0xd3ff, 0xd4e0, 0xd4f5, 0xd4ea, 0xd4ff,
	0xd5e0, 0xd5f5, 0xd5ea, 0xd5ff, 0xd6e0, 0xd6f5, 0xd6ea, 0xd6ff,
	0xd7e0, 0xd7f5, 0xd7ea, 0xd7ff, 0xf855, 0xf8aa, 0xf8e0, 0xf8f5,
	0xf8ea, 0xf8ff, 0xf9e0, 0xf9f5, 0xf9ea, 0xf9ff, 0xfae0, 0xfaf5,
	0xfaea, 0xfaff, 0xfbe0, 0xfbf5, 0xfbea, 0xfbff, 0xfce0, 0xfcf5,
	0xfcea, 0xfcff, 0xfde0, 0xfdf5, 0xfdea, 0xfdff, 0xfee0, 0xfef5,
	0xfeea, 0xfeff, 0xfff5, 0xffea, 0xceff, 0xfeff, 0x37ff, 0x67ff,
	0x9fff, 0xcfff, 0x03e0, 0x03f5, 0x03ea, 0x03ff, 0x04e0, 0x04f5,
	0x04ea, 0x04ff, 0x05e0, 0x05f5, 0x05ea, 0x05ff, 0x06e0, 0x06f5,
	0x06ea, 0x06ff, 0x07f5, 0x07ea, 0x2800, 0x2855, 0x28aa, 0x28ff,
	0x28e0, 0x28f5, 0x28ea, 0x28ff, 0x29e0, 0x29f5, 0xfff0, 0xa5a4,
	0x8480, 0xf800, 0x07e0, 0xffe0, 0x00ff, 0xf8ff, 0x07ff, 0xffff
};

static u_int16_t __color_tab16[] = {
	RGB565_BLACK,
	RGB565_RED,
	RGB565_GREEN,
	RGB565_YELLOW,
	RGB565_BLUE,
	RGB565_MAGENTA,
	RGB565_CYAN,
	RGB565_WHITE
};

static int bootinfo_fb_depth(void);
static void __dbg_lcd_test8(vaddr_t, int, int);
static void __dbg_lcd_test16(vaddr_t, int, int);

void
dbg_lcd_test()
{
	vaddr_t fbaddr;
	int w, h;

	if (bootinfo == NULL || bootinfo->fb_addr == NULL)
		return;

	fbaddr = (vaddr_t)bootinfo->fb_addr;
	w = bootinfo->fb_width;
	h = bootinfo->fb_height;

	switch (bootinfo_fb_depth()) {
	default:
		break;
	case 8:
		__dbg_lcd_test8(fbaddr, w, h);
		break;
	case 16:
		__dbg_lcd_test16(fbaddr, w, h);
		break;
	}
}

/*
 * 8bpp LCD test routine.
 *  HP620LX, HPW50PAD are little-endian display.
 */
void
__dbg_lcd_test8(vaddr_t fbaddr, int w, int h)
{
	u_int8_t *fb, *fb_start = (u_int8_t *)fbaddr;
	int i, x, y, y_start, y_end, len, skip;
	u_int16_t *fb16;
	u_int32_t *fb32;
	
	/* 
	 * Clear screen (BLACK)
	 */
	for (fb = fb_start, i = 0; i < w * h; i++)
		*fb++ = 0;

	/*
	 * Color bar (I assume palette index is ANSI)
	 *	BLACK|RED|GREEN|YELLOW|BLUE|MAGENTA|CYAN|WHITE
	 */
	len = w / 8;
	y_start = 0;
	y_end = h / 2;
	skip = bootinfo->fb_line_bytes - w;
	for (fb = fb_start, y = y_start; y < y_end; y++, fb += skip) {
		for (i = 0; i < 8; i++) {
			for (x = 0; x < len; x++)
				*fb++ = i;
		}
	}

	/*
	 * Byte order test. (display 32x32 bitmap)
	 */
	/* 
	 * 8-bit access (reference) 
	 */
	y_start = y_end;
	y_end = y_start + 32;
	i = 0;
	fb += w / 2 - 16; /* center */
	skip = bootinfo->fb_line_bytes - 32;
	for (y = y_start; y < y_end; y++, fb += skip)
		for (x = 0; x < 32; x++)
			*fb++ = img[i++];

	/* 
	 * 16-bit access
	 */
	y_start = y_end;
	y_end = y_start + 32;
	i = 0;
	fb16 = (u_int16_t *)fb;
	skip /= 2;
	for (y = y_start; y < y_end; y++, fb16 += skip)
		for (x = 0; x < 16; x++, i += 2)
			*fb16++ =
			    img[i + 1] << 8 |
			    img[i + 0];

	/* 
	 * 32-bit access
	 */
	y_start = y_end;
	y_end = y_start + 32;
	i = 0;
	fb32 = (u_int32_t *)(fb + 32 * w);
	skip /= 2;
	for (y = y_start; y < y_end; y++, fb32 += skip)
		for (x = 0; x < 8; x++, i += 4)
			*fb32++ = 
			    img[i + 3] << 24 | 
			    img[i + 2] << 16 |
			    img[i + 1] <<  8 |
			    img[i + 0];
}

/*
 * 16bpp LCD test routine.
 *  Jornada6[89]0, HPW650PA are big-endian display.
 */
void
__dbg_lcd_test16(vaddr_t fbaddr, int w, int h)
{
	u_int16_t *fb, *fb_start = (u_int16_t *)fbaddr;
	u_int16_t pat;
	int i, x, y, y_start, y_end, len, skip;
	u_int32_t *fb32;

	/* 
	 * Clear screen (BLACK)
	 */
	pat = __color_tab16[0];
	for (fb = fb_start, i = 0; i < w * h; i++)
		*fb++ = pat;

	/*
	 * Color bar
	 *	BLACK|RED|GREEN|YELLOW|BLUE|MAGENTA|CYAN|WHITE
	 */
	len = w / 8;
	y_start = 0;
	y_end = h / 2;
	for (fb = fb_start, y = y_start; y < y_end; y++) {
		for (i = 0; i < 8; i++) {
			pat = __color_tab16[i];
			for (x = 0; x < len; x++)
				*fb++ = pat;
		}
	}
	y_start = y_end;

	/*
	 * Byte order test. (display 32x32 bitmap)
	 */
	/* 
	 * 16-bit access (reference) 
	 */
	y_start = y_end;
	y_end = y_start + 32;
	i = 0;
	fb += w / 2 - 16; /* center */
	skip = w - 32;
	for (y = y_start; y < y_end; y++, fb += skip)
		for (x = 0; x < 32; x++)
			*fb++ = rgb565_clut[img[i++]];

	/* 
	 * 32-bit access (test)
	 */
	y_start = y_end;
	y_end = y_start + 32;
	i = 0;
	fb32 = (u_int32_t *)fb;
	skip /= 2;
	for (y = y_start; y < y_end; y++, fb32 += skip)
		for (x = 0; x < 16; x++, i += 2)
			*fb32++ = (rgb565_clut[img[i + 1]] << 16) |
			    rgb565_clut[img[i]];
}

int
bootinfo_fb_depth()
{

	switch (bootinfo->fb_type) {
	case BIFB_D2_M2L_3:
		/* FALLTHROUGH */
	case BIFB_D2_M2L_0:
		return (2);
	case BIFB_D4_M2L_F:
		/* FALLTHROUGH */
	case BIFB_D4_M2L_0:
		return (4);
	case BIFB_D8_FF:
		/* FALLTHROUGH */
	case BIFB_D8_00:
		return (8);
	case BIFB_D16_FFFF:
		/* FALLTHROUGH */
	case BIFB_D16_0000:
		return (16);
	}

	return (0);
}

#endif /* HPC_DEBUG_LCD */
