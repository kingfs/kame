/* $OpenBSD: wdcevent.h,v 1.1 2002/03/16 17:12:09 csapuntz Exp $ */

/*
 * Copyright (c) 2001 Constantine Sapuntzakis
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef WDCEVENT_H
#define WDCEVENT_H

void wdc_log(struct channel_softc *chp, int type, 
    unsigned int size, char  val[]);

static __inline void WDC_LOG_STATUS(struct channel_softc *chp,
    u_int8_t status) {
	if (chp->ch_prev_log_status == status)
		return;
	
	chp->ch_prev_log_status = status;
	wdc_log(chp, 1, 1, &status);
}

static __inline void WDC_LOG_ERROR(struct channel_softc *chp,
    u_int8_t error) {
	wdc_log(chp, 2, 1, &error);
}

static __inline void WDC_LOG_ATAPI_CMD(struct channel_softc *chp, int drive,
    int flags, int len, void *cmd) {
	u_int8_t record[20];

	record[0] = (flags >> 8);
	record[1] = flags & 0xff;
	bcopy(cmd, &record[2], len);

	wdc_log(chp, 3, len + 2, record);
}

static __inline void WDC_LOG_ATAPI_DONE(struct channel_softc *chp, int drive,
    int flags, u_int8_t error) {
	char record[3] = {flags >> 8, flags & 0xff, error};
	wdc_log(chp, 4, 3, record);
}

static __inline void WDC_LOG_ATA_CMDSHORT(struct channel_softc *chp, u_int8_t cmd) {
	wdc_log(chp, 5, 1, &cmd);
}

static __inline void WDC_LOG_ATA_CMDLONG(struct channel_softc *chp, 
    u_int8_t head, u_int8_t precomp, u_int8_t cylinhi, u_int8_t cylinlo,
    u_int8_t sector, u_int8_t count, u_int8_t command) {
	char record[8] = { head, precomp, cylinhi, cylinlo,
			   sector, count, command };

	wdc_log(chp, 6, 7, record);
}

static __inline void WDC_LOG_SET_DRIVE(struct channel_softc *chp,
    u_int8_t drive) {
	wdc_log(chp, drive ? 7 : 8, 0, NULL);
}

static __inline void WDC_LOG_REG(struct channel_softc *chp,
    enum wdc_regs reg, u_int16_t val) {
	char record[3];

	record[0] = reg;
	record[1] = (val >> 8);
	record[2] = val & 0xff;

	wdc_log(chp, 9, 3, record);
}

#endif
