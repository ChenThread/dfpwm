/*
DFPWM2 (Dynamic Filter Pulse Width Modulation) codec - C Implementation
by Ben "GreaseMonkey" Russell, 2012, 2016
Public Domain

Compression Component
*/

#ifndef CONST_PREC
#define CONST_PREC 16
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "prectable.h"

// note, len denotes how many compressed bytes there are (uncompressed bytes / 16).
void au_compress(int *q, int *s, int *lt, int len, uint8_t *outbuf, int16_t *inbuf)
{
	int i,j;
	uint8_t d = 0;
	for(i = 0; i < len; i++)
	{
		for(j = 0; j < 8; j++)
		{
#if STEREO
			int ci = (j&1);
#else
			int ci = 0;
#endif

			// get sample
#if STEREO
			int v = *(inbuf++);
#else
			int v0 = *(inbuf++);
			int v1 = *(inbuf++);
			int v = (v0+v1+1)>>1;
#endif

			// set bit / target
			int t = (v < q[ci] || v == -32768 ? -32768 : 32767);
			d >>= 1;
			//if((t > 0) == (lt[ci] >= 0))
			if(t > 0) {
				d |= 0x80;
			}

			// adjust charge
			int rs = prec_table[s[ci]];
			int nq = q[ci] + ((rs * (t-q[ci]) + (1<<(CONST_PREC-1)))>>CONST_PREC);
			if(nq == q[ci] && nq != t)
				q[ci] += (t == 32767 ? 1 : -1);
			//int lq = q[ci];
			q[ci] = nq;

			// adjust strength
			int ns = s[ci];
			ns += (t == lt[ci]
				? (ns >= PREC_TABLE_MID_UP ? 1 : 
					ns >= PREC_TABLE_MID_UP_2 ? 2 : 3)
				: (ns >= PREC_TABLE_MID_DOWN ? -2 : -1)
			)*(ci == 0 ? 1 : 2);
			//ns += (t == lt[ci] ? 2 : -1)*(ci == 0 ? 1 : 2);
			if(ns < PREC_TABLE_BOTTOM) { ns = PREC_TABLE_BOTTOM; }
			if(ns > PREC_TABLE_LEN-1) { ns = PREC_TABLE_LEN-1; }
#if 0
#if CONST_PREC > 16
			if(ns < 1+(1<<(CONST_PREC-16))) ns = 1+(1<<(CONST_PREC-16));
#endif
#endif
			//fprintf(stderr, "%d %d\n", nq, ns);
			s[ci] = ns;

			lt[ci] = t;

			//fprintf(stderr, "%4i %4i %4i %4i\n", v, q[ci], s[ci], t);
			//usleep(10000);
		}

		// output bits
		*(outbuf++) = d;
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int16_t rawbuf[1024*2];
	uint8_t cmpbuf[128];

	int q[2] = {0, 0};
	int s[2] = {0, 0};
	int lt[2] = {-32768, -32768};

	FILE *infp = fopen("/dev/stdin","rb");
	FILE *outfp = fopen("/dev/stdout","wb");

	while(!feof(infp))
	{
		int i;
		for(i = 0; i < 1024*(STEREO ? 1 : 2); i++) {
			int b0 = fgetc(infp);
			int b1 = fgetc(infp);
			rawbuf[i] = b0|(b1<<8);
		}

		au_compress(&q[0], &s[0], &lt[0], 128, cmpbuf, rawbuf);

		fwrite(cmpbuf, 128, 1, outfp);
	}

	fclose(outfp);
	fclose(infp);

	return 0;
}

