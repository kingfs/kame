/*      $NetBSD: rndpool.c,v 1.17 2002/11/10 03:29:00 thorpej Exp $        */

/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Michael Graff <explorer@flame.org>.  This code uses ideas and
 * algorithms from the Linux driver written by Ted Ts'o.
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
__KERNEL_RCSID(0, "$NetBSD: rndpool.c,v 1.17 2002/11/10 03:29:00 thorpej Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sha1.h>

#include <sys/rnd.h>

/*
 * The random pool "taps"
 */
#define	TAP1	99
#define	TAP2	59
#define	TAP3	31
#define	TAP4	 9
#define	TAP5	 7

static inline void rndpool_add_one_word(rndpool_t *, u_int32_t);

void
rndpool_init(rndpool_t *rp)
{

	rp->cursor = 0;
	rp->rotate = 1;

	memset(&rp->stats, 0, sizeof(rp->stats));

	rp->stats.curentropy = 0;
	rp->stats.poolsize = RND_POOLWORDS;
	rp->stats.threshold = RND_ENTROPY_THRESHOLD;
	rp->stats.maxentropy = RND_POOLBITS;

	KASSERT(RND_ENTROPY_THRESHOLD*2 <= 20); /* XXX sha knowledge */
}

u_int32_t
rndpool_get_entropy_count(rndpool_t *rp)
{

	return (rp->stats.curentropy);
}

void rndpool_get_stats(rndpool_t *rp, void *rsp, int size)
{

	memcpy(rsp, &rp->stats, size);
}

void
rndpool_increment_entropy_count(rndpool_t *rp, u_int32_t  entropy)
{

	rp->stats.curentropy += entropy;
	rp->stats.added += entropy;
	if (rp->stats.curentropy > RND_POOLBITS) {
		rp->stats.discarded += (rp->stats.curentropy - RND_POOLBITS);
		rp->stats.curentropy = RND_POOLBITS;
	}
}

u_int32_t *
rndpool_get_pool(rndpool_t *rp)
{

	return (rp->pool);
}

u_int32_t
rndpool_get_poolsize(void)
{

	return (RND_POOLWORDS);
}

/*
 * The input function treats the contents of the pool as an array of
 * 32 LFSR's of length RND_POOLWORDS, one per bit-plane.  The LFSR's
 * are clocked once in parallel, using 32-bit xor operations, for each
 * word to be added.
 *
 * Each word to be added is xor'd with the output word of the LFSR
 * array (one tap at a time).
 * 
 * In order to facilitate distribution of entropy between the
 * bit-planes, a 32-bit rotate of this result is performed prior to
 * feedback. The rotation distance is incremented every RND_POOLWORDS
 * clocks, by a value that is relativly prime to the word size to try
 * to spread the bits throughout the pool quickly when the pool is
 * empty.
 *
 * Each LFSR thus takes its feedback from another LFSR, and is
 * effectively re-keyed by both that LFSR and the new data.  Feedback
 * occurs with another XOR into the new LFSR, rather than assignment,
 * to avoid destroying any entropy in the destination.
 *
 * Even with zeros as input, the LFSR output data are never visible;
 * the contents of the pool are never divulged except via a hash of
 * the entire pool, so there is no information for correlation
 * attacks. With rotation-based rekeying, each LFSR runs at most a few
 * cycles before being permuted.  However, beware of initial
 * conditions when no entropy has been added.
 *
 * The output function also stirs the generated hash back into the
 * pool, further permuting the LFSRs and spreading entropy through the
 * pool.  Any unknown bits anywhere in the pool are thus reflected
 * across all the LFSRs after output.
 * 
 * (The final XOR assignment into the pool for feedback is equivalent
 * to an additional LFSR tap of the MSB before shifting, in the case
 * where no rotation is done, once every 32 cycles. This LFSR runs for
 * at most one length.)
 */
static inline void
rndpool_add_one_word(rndpool_t *rp, u_int32_t  val)
{
  	/*
	 * Shifting is implemented using a cursor and taps as offsets,
	 * added mod the size of the pool. For this reason,
	 * RND_POOLWORDS must be a power of two.
	 */
	val ^= rp->pool[(rp->cursor + TAP1) & (RND_POOLWORDS - 1)];
	val ^= rp->pool[(rp->cursor + TAP2) & (RND_POOLWORDS - 1)];
	val ^= rp->pool[(rp->cursor + TAP3) & (RND_POOLWORDS - 1)];
	val ^= rp->pool[(rp->cursor + TAP4) & (RND_POOLWORDS - 1)];
	val ^= rp->pool[(rp->cursor + TAP5) & (RND_POOLWORDS - 1)];
	if (rp->rotate != 0)
		val = ((val << rp->rotate) | (val >> (32 - rp->rotate)));
	rp->pool[rp->cursor++] ^= val;

	/*
	 * If we have looped around the pool, increment the rotate
	 * variable so the next value will get xored in rotated to
	 * a different position.
	 */
	if (rp->cursor == RND_POOLWORDS) {
		rp->cursor = 0;
		rp->rotate = (rp->rotate + 7) & 31;
	}
}

#if 0
/*
 * Stir a 32-bit value (with possibly less entropy than that) into the pool.
 * Update entropy estimate.
 */
void
rndpool_add_uint32(rndpool_t *rp, u_int32_t  val, u_int32_t  entropy)
{
	rndpool_add_one_word(rp, val);

	rp->entropy += entropy;
	rp->stats.added += entropy;
	if (rp->entropy > RND_POOLBITS) {
		rp->stats.discarded += (rp->entropy - RND_POOLBITS);
		rp->entropy = RND_POOLBITS;
	}
}
#endif

/*
 * Add a buffer's worth of data to the pool.
 */
void
rndpool_add_data(rndpool_t *rp, void *p, u_int32_t len, u_int32_t entropy)
{
	u_int32_t val;
	u_int8_t *buf;

	buf = p;

	for (; len > 3; len -= 4) {
		val = *((u_int32_t *)buf);

		rndpool_add_one_word(rp, val);
		buf += 4;
	}

	if (len != 0) {
		val = 0;
		switch (len) {
		case 3:
			val = *buf++;
		case 2:
			val = val << 8 | *buf++;
		case 1:
			val = val << 8 | *buf++;
		}

		rndpool_add_one_word(rp, val);
	}

	rp->stats.curentropy += entropy;
	rp->stats.added += entropy;

	if (rp->stats.curentropy > RND_POOLBITS) {
		rp->stats.discarded += (rp->stats.curentropy - RND_POOLBITS);
		rp->stats.curentropy = RND_POOLBITS;
	}
}

/*
 * Extract some number of bytes from the random pool, decreasing the
 * estimate of randomness as each byte is extracted.
 *
 * Do this by hashing the pool and returning a part of the hash as
 * randomness.  Stir the hash back into the pool.  Note that no
 * secrets going back into the pool are given away here since parts of
 * the hash are xored together before being returned.
 *
 * Honor the request from the caller to only return good data, any data,
 * etc.  Note that we must have at least 64 bits of entropy in the pool
 * before we return anything in the high-quality modes.
 */
u_int32_t
rndpool_extract_data(rndpool_t *rp, void *p, u_int32_t len, u_int32_t mode)
{
	u_int i;
	SHA1_CTX hash;
	u_char digest[20];	/* XXX SHA knowledge */
	u_int32_t remain, deltae, count;
	u_int8_t *buf;
	int good;

	buf = p;
	remain = len;

	if (mode == RND_EXTRACT_ANY)
		good = 1;
	else
		good = (rp->stats.curentropy >= (8 * RND_ENTROPY_THRESHOLD));

	KASSERT(RND_ENTROPY_THRESHOLD*2 <= 20); /* XXX SHA knowledge */

	while (good && (remain != 0)) {
		/*
		 * While bytes are requested, compute the hash of the pool,
		 * and then "fold" the hash in half with XOR, keeping the
		 * exact hash value secret, as it will be stirred back into
		 * the pool.
		 *
		 * XXX this approach needs examination by competant
		 * cryptographers!  It's rather expensive per bit but
		 * also involves every bit of the pool in the
		 * computation of every output bit..
		 */
		SHA1Init(&hash);
		SHA1Update(&hash, (u_int8_t *)rp->pool, RND_POOLWORDS * 4);
		SHA1Final(digest, &hash);

		/*
		 * Stir the hash back into the pool.  This guarantees
		 * that the next hash will generate a different value
		 * if no new values were added to the pool.
		 */
		for (i = 0; i < 5; i++) {
			u_int32_t word;
			memcpy(&word, &digest[i * 4], 4);
			rndpool_add_one_word(rp, word);
		}

		count = min(remain, RND_ENTROPY_THRESHOLD);

		for (i = 0; i < count; i++)
			buf[i] = digest[i] ^ digest[i + RND_ENTROPY_THRESHOLD];

		buf += count;
		deltae = count * 8;
		remain -= count;

		deltae = min(deltae, rp->stats.curentropy);

		rp->stats.removed += deltae;
		rp->stats.curentropy -= deltae;

		if (rp->stats.curentropy == 0)
			rp->stats.generated += (count * 8) - deltae;

		if (mode == RND_EXTRACT_GOOD)
			good = (rp->stats.curentropy >=
			    (8 * RND_ENTROPY_THRESHOLD));
	}

	memset(&hash, 0, sizeof(hash));
	memset(digest, 0, sizeof(digest));

	return (len - remain);
}
