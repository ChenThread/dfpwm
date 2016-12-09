#include "prectable.h"

// note, len denotes how many compressed bytes there are (uncompressed bytes / 16).
#ifndef DFPWM_DECOMPRESS
void au_compress(int *q, int *s, int *lt, int len, uint8_t *outbuf, int16_t *inbuf)
#else /* DFPWM_DECOMPRESS */
void au_decompress(int *fq, int *q, int *s, int *lt, int *lq, int len, int16_t *outbuf, uint8_t *inbuf)
#endif /* DFPWM_DECOMPRESS */
{
	int i,j;
	uint8_t d = 0;
	for(i = 0; i < len; i++)
	{
		//
		// DECOMP: get bits
		//
#ifdef DFPWM_DECOMPRESS
		d = *(inbuf++);
#endif /* DFPWM_DECOMPRESS */

		for(j = 0; j < 8; j++)
		{
			//
			// COMP: get sample
			//
#ifndef DFPWM_DECOMPRESS
#if STEREO
			int v = *(inbuf++);
#else
			int v0 = *(inbuf++);
			int v1 = *(inbuf++);
			int v = (v0+v1+1)>>1;
#endif
#endif /* ! DFPWM_DECOMPRESS */

			//
			// Select channel
			//
			int ci = (STEREO ? (j&1) : 0);

			//
			// Set bit (DECOMP) / target (BOTH)
			//
#ifndef DFPWM_DECOMPRESS
			int t = (v < q[ci] || v == -32768 ? -32768 : 32767);
			d >>= 1;
			if(t > 0) { d |= 0x80; }
#else /* DFPWM_DECOMPRESS */
			int t = ((d&1) != 0 ? 32767 : -32768);
			d >>= 1;
#endif /* DFPWM_DECOMPRESS */

			//
			// Adjust charge
			//
			int rs = prec_table[s[ci]];
			int nq = q[ci] + ((rs * (t-q[ci]) + (1<<(CONST_PREC-1)))>>CONST_PREC);
			if(nq == q[ci] && nq != t)
				q[ci] += (t == 32767 ? 1 : -1);
#ifdef DFPWM_DECOMPRESS
			lq[ci] = q[ci];
#endif /* DFPWM_DECOMPRESS */
			q[ci] = nq;

			//
			// Adjust strength
			//
			int ns = s[ci];
			ns += (t == lt[ci]
				? (ns >= PREC_TABLE_MID_UP ? 1 : 
					ns >= PREC_TABLE_MID_UP_2 ? 2 : 3)
				: (ns >= PREC_TABLE_MID_DOWN ? -2 : -1)
			);
			if(ns < PREC_TABLE_BOTTOM) { ns = PREC_TABLE_BOTTOM; }
			if(ns > PREC_TABLE_LEN-1) { ns = PREC_TABLE_LEN-1; }
			s[ci] = ns;

#ifndef DFPWM_DECOMPRESS
			lt[ci] = t;
#else /* DFPWM_DECOMPRESS */
#if STEREO
			if(ci != 1) {
				lt[ci] = t;
				continue;
			}
#endif
#endif /* DFPWM_DECOMPRESS */

			//
			// DECOMP: Apply filters
			//
#ifdef DFPWM_DECOMPRESS
			// FILTER: perform LPF
			//fq[0] += ((CONST_POSTFILT*(q[0]-fq[0]) + 0x80)>>8);
			//fq[1] += ((CONST_POSTFILT_STEREO*(q[1]-fq[1]) + 0x80)>>8);
			fq[0] = q[0];
#if STEREO
			fq[1] = q[1];
#endif

			// interpolation of stereo
			//fq[1] += ((q[1]-fq[1])*((j+1)&7))/7;

			int mv0 = fq[0];
			int mv1 = fq[1];

			// FILTER: perform antijerk
			// TODO: stereo-friendly version
			//mv0 = (t != lt[ci] ? (mv0+lq[0])>>1 : mv0);

#if STEREO
			int ov0 = mv0;
			int ov1 = mv1;
#else
			int ov0 = mv0;
			int ov1 = mv0;
#endif
			if(ov0 < -32768) { ov0 = -32768; }
			if(ov0 >  32767) { ov0 =  32767; }
			if(ov1 < -32768) { ov1 = -32768; }
			if(ov1 >  32767) { ov1 =  32767; }

			// output sample
			*(outbuf++) = ov0;
			*(outbuf++) = ov1;

			lt[ci] = t;
#endif /* DFPWM_DECOMPRESS */
		}

		//
		// COMP: output bits
		//
#ifndef DFPWM_DECOMPRESS
		*(outbuf++) = d;
#endif /* ! DFPWM_DECOMPRESS */

	}
}
