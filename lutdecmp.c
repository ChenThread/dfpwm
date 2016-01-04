/*
DFPWM (Dynamic Filter Pulse Width Modulation) codec - C LUT Approximate Implementation
by Ben "GreaseMonkey" Russell, 2012, 2016
Public Domain

Decompression Component
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <immintrin.h>

// note, len denotes how many compressed bytes there are (uncompressed bytes / 8).

void au_decompress_real(int *fq, int *q, int *s, int *lt, int fs, int ri, int rd, int len, int8_t *outbuf, uint8_t *inbuf)
{
	int i,j;
	uint8_t d;
	for(i = 0; i < len; i++)
	{
		// get bits
		d = *(inbuf++);
		
		for(j = 0; j < 8; j++)
		{
			// set target
			int t = ((d&1) ? 127 : -128);
			d >>= 1;
			
			// adjust charge
			int nq = *q + ((*s * (t-*q) + 0x80)>>8);
			if(nq == *q && nq != t)
				*q += (t == 127 ? 1 : -1);
			int lq = *q;
			*q = nq;
			
			// adjust strength
			int st = (t != *lt ? 0 : 255);
			int sr = (t != *lt ? rd : ri);
			int ns = *s + ((sr*(st-*s) + 0x80)>>8);
			if(ns == *s && ns != st)
				ns += (st == 255 ? 1 : -1);
			*s = ns;
			
			// FILTER: perform antijerk
			int ov = (t != *lt ? (nq+lq)>>1 : nq);
			//int ov = nq;
			
			// FILTER: perform LPF
			*fq += ((fs*(ov-*fq) + 0x80)>>8);
			ov = *fq;
			
			// output sample
			*(outbuf++) = ov;
			
			*lt = t;
		}
	}
}

int8_t decmp_lut[65536];

void au_decompress(int len, int8_t *outbuf, uint8_t *inbuf)
{
	int i;

	i = 0;

	// FIXME: make OpenMP not limp (it works, it just limps)
//pragma omp parallel for
	for(i = 0; i < len; i++)
	{
		int j;
		int oi = i*8;
		uint8_t v2 = inbuf[i-2];
		uint8_t v1 = inbuf[i-1];
		uint16_t v2a = v2;
		uint16_t v1a = v1;
		uint16_t v = (v1a<<8)|v2a;
		uint8_t b = inbuf[i];

		for(j = 0; j < 8; j++, oi++)
		{
			v = (v>>1)|(b<<15);
			b >>= 1;

			// seems to work better with it precalc'd in the LUT
			//const int fs = 100;
			//outbuf[oi] = (decmp_lut[v]*(256-fs) + outbuf[oi-1]*fs + 128)>>8;
			outbuf[oi] = decmp_lut[v];
		}
	}

	inbuf[-2] = inbuf[len-2];
	inbuf[-1] = inbuf[len-1];
	outbuf[-1] = inbuf[len*8-1];
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int8_t rawbuf[1024];
	uint8_t cmpbuf[128+2];
	
	int q = 0;
	int s = 0;
	int lt = -128;
	int ri = 7;
	int rd = 20;
	
	int fq = 0;
	int fs = 100;

	uint16_t v = 0;
	rawbuf[0] = 0;
	cmpbuf[0] = 0;
	cmpbuf[1] = 0;
	
	FILE *infp = fopen("/dev/stdin","rb");
	FILE *outfp = fopen("/dev/stdout","wb");

	{
		int32_t i;
		int j;

		//fprintf(stderr, "LUT gen\n");
		for(i = 0; i < 0x10000; i++)
		{
			v = i;
			v = (v>>8)|(v<<8);
			v = ((v>>4)&0x0F0F)|((v<<4)&0xF0F0);
			v = ((v>>2)&0x3333)|((v<<2)&0xCCCC);
			v = ((v>>1)&0x5555)|((v<<1)&0xAAAA);

			for(j = 0; j < 16; j++)
				cmpbuf[j+2] = (uint8_t)(v>>((j & 1)*8));
			au_decompress_real(&fq, &q, &s, &lt, fs, ri, rd, 16, rawbuf+1, cmpbuf+2);
			decmp_lut[i] = rawbuf[1+16*8-1];
			//fprintf(stderr, "%04X %02X\n", i, 0x80+(int16_t)decmp_lut[i]);
		}
		//fprintf(stderr, "Done\n");
	}

	while(!feof(infp))
	{
		int i;
		for(i = 0; i < 128; i++)
			cmpbuf[i+2] = fgetc(infp);
		//fprintf(stderr, "%i\n", fgetc(infp));
		//au_decompress(&fq, &q, &s, &lt, fs, ri, rd, 128, rawbuf, cmpbuf);
		au_decompress(128, rawbuf, cmpbuf+2);
		
		fwrite(rawbuf, 128*8, 1, outfp);
	}
	
	fclose(outfp);
	fclose(infp);
	
	return 0;
}

