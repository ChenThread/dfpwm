/*
DFPWM2 (Dynamic Filter Pulse Width Modulation) codec - C Implementation
by Ben "GreaseMonkey" Russell, 2012, 2016
Public Domain

Decompression Component
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#ifndef CONST_PREC
#define CONST_PREC 16
#endif
#ifndef CONST_POSTFILT
#define CONST_POSTFILT 140
#endif
#ifndef CONST_POSTFILT_STEREO
#define CONST_POSTFILT_STEREO 20
#endif

#include "prectable.h"

// note, len denotes how many compressed bytes there are (uncompressed bytes / 16).

void au_decompress(int *fq, int *q, int *s, int *lt, int *lq, int len, int16_t *outbuf, uint8_t *inbuf)
{
	int i,j;
	uint8_t d;
	for(i = 0; i < len; i++)
	{
		// get bits
		d = *(inbuf++);

		for(j = 0; j < 8; j++)
		{
#if STEREO
			int ci = (j&1);
#else
			int ci = 0;
#endif

			// set target
			//int t = (((d&1) != 0) == (lt[ci] >= 0) ? 32767 : -32768);
			int t = ((d&1) != 0 ? 32767 : -32768);
			d >>= 1;

			// adjust charge
			int rs = prec_table[s[ci]];
			int nq = q[ci] + ((rs * (t-q[ci]) + (1<<(CONST_PREC-1)))>>CONST_PREC);
			if(nq == q[ci] && nq != t)
				q[ci] += (t == 32767 ? 1 : -1);
			lq[ci] = q[ci];
			q[ci] = nq;

			// adjust strength
			int ns = s[ci];
			ns += (t == lt[ci]
				? (ns >= PREC_TABLE_MID_UP ? 1 : 
					ns >= PREC_TABLE_MID_UP_2 ? 2 : 3)
				: (ns >= PREC_TABLE_MID_DOWN ? -2 : -1)
			)*(ci == 0 ? 1 : 2);
			if(ns < PREC_TABLE_BOTTOM) { ns = PREC_TABLE_BOTTOM; }
			if(ns > PREC_TABLE_LEN-1) { ns = PREC_TABLE_LEN-1; }
#if 0
#if CONST_PREC > 16
			if(ns < 1+(1<<(CONST_PREC-16))) ns = 1+(1<<(CONST_PREC-16));
#endif
#endif
			s[ci] = ns;

#if STEREO
			if(ci != 1) {
				lt[ci] = t;
				continue;
			}
#endif

			// FILTER: perform LPF
			//fq[0] += ((CONST_POSTFILT*(q[0]-fq[0]) + 0x80)>>8);
			fq[0] = q[0];
#if STEREO
			fq[1] = q[1];
#else
			//fq[1] += ((CONST_POSTFILT_STEREO*(q[1]-fq[1]) + 0x80)>>8);
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
			//fprintf(stderr, "%d\n", ov0-ov1);

			// output sample
			//*(outbuf++) = ov;
			//*(outbuf++) = ov;
			*(outbuf++) = ov0;
			*(outbuf++) = ov1;

			lt[ci] = t;
		}
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int16_t rawbuf[1024*2];
	uint8_t cmpbuf[32768];

	int q[2] = {0, 0};
	int s[2] = {0, 0};
	int lt[2] = {-32768, -32768};

	int fq[2] = {0, 0};
	int lq[2] = {0, 0};

	FILE *infp = fopen("/dev/stdin","rb");
	FILE *outfp = fopen("/dev/stdout","wb");

	while(!feof(infp))
	{
		int i;
		for(i = 0; i < 128; i++)
			cmpbuf[i] = fgetc(infp);
		//fprintf(stderr, "%i\n", fgetc(infp));
		au_decompress(&fq[0], &q[0], &s[0], &lt[0], &lq[0], 128, rawbuf, cmpbuf);

		fwrite(rawbuf, 128*16*(STEREO ? 1 : 2), 1, outfp);
	}

	fclose(outfp);
	fclose(infp);

	return 0;
}

