/*
DFPWM1a (Dynamic Filter Pulse Width Modulation) codec - C Implementation
by Ben "GreaseMonkey" Russell, 2012, 2016
Public Domain

Compression Component
*/

#ifndef CONST_PREC
#define CONST_PREC 10
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <immintrin.h>

// note, len denotes how many compressed bytes there are (uncompressed bytes / 8).
void au_compress(int *q, int *s, int *lt, int len, uint8_t *outbuf, int8_t *inbuf)
{
	int i,j;
	uint8_t d = 0;
	for(i = 0; i < len; i++)
	{
		for(j = 0; j < 8; j++)
		{
			// get sample
			int v = *(inbuf++);

			// set bit / target
			int t = (v < *q || v == -128 ? -128 : 127);
			d >>= 1;
			if(t > 0)
				d |= 0x80;

			// adjust charge
			int nq = *q + ((*s * (t-*q) + (1<<(CONST_PREC-1)))>>CONST_PREC);
			if(nq == *q && nq != t)
				nq += (t == 127 ? 1 : -1);
			*q = nq;

			// adjust strength
			int st = (t != *lt ? 0 : (1<<CONST_PREC)-1);
			int ns = *s;
			if(ns != st)
				ns += (st != 0 ? 1 : -1);
#if CONST_PREC > 8
			if(ns < 1+(1<<(CONST_PREC-8))) ns = 1+(1<<(CONST_PREC-8));
#endif
			*s = ns;

			*lt = t;

			//fprintf(stderr, "%4i %4i %4i %4i\n", v, *q, *s, t);
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

	int8_t rawbuf[1024];
	uint8_t cmpbuf[128];

	int q = 0;
	int s = 0;
	int lt = -128;

	FILE *infp = fopen("/dev/stdin","rb");
	FILE *outfp = fopen("/dev/stdout","wb");

	while(!feof(infp))
	{
		int i;
		for(i = 0; i < 1024; i++)
			rawbuf[i] = fgetc(infp);

		au_compress(&q, &s, &lt, 128, cmpbuf, rawbuf);

		fwrite(cmpbuf, 128, 1, outfp);
	}

	fclose(outfp);
	fclose(infp);

	return 0;
}

