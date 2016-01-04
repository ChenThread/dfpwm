/*
DFPWM (Dynamic Filter Pulse Width Modulation) codec - C Implementation
by Ben "GreaseMonkey" Russell, 2012
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

/*

Going to have to do some mathematical crap.
THIS IS GOING TO SUCK as I have to find out how to optimise stuff SOMEHOW.

For simplicity, we're treating these as floating point.

For the filter filter (the 'Dynamic' in DFPWM):
	Let f(x) be the result of going UP   to 1 using ri.
	Let g(x) be the result of going DOWN to 0 using rd.

Base formulae:
	f(x) = ri * (1 - x) + x
	g(x) = rd * (0 - x) + x

Simplify f:
	f(x) = ri - ri*x + x
	f(x) = ri + x*(1 - ri)

Simplify g:
	g(x) = rd * (0 - x) + x
	g(x) = rd * -x + x
	g(x) = -rd * x + x
	g(x) = x*(1 - rd)

Compose f.g
	f(g(x)) = ri + g(x)*(1 - ri)
		= ri + x*(1 - rd)*(1 - ri)

Compose g.f
	g(f(x)) = f(x)*(1 - rd)
		= (ri + x*(1 - ri))*(1 - rd)
		= ri*(1 - rd) + x*(1 - ri)*(1 - rd)
		= ri*(1 - rd) + f(g(x)).

Therefore composition is NOT commutative.
However, one composition can be defined in terms of another.
This is a start.

Let
	K = ri*(1 - rd)
Then
	g.f(x) = K + f.g(x)

Compose g.g
	g(g(x)) = g(x)*(1 - rd)
		= x*(1 - rd)*(1 - rd)
		= x * (1 - rd)^2
Compose f.f
	f(f(x)) = ri + f(x)*(1 - ri)
		= ri + (ri + x*(1 - ri))*(1 - ri)
		= ri + ri*(1 - ri) + x*(1 - ri)*(1 - ri)
		= ri + ri*(1 - ri) + x*(1 - ri)^2

Let
	P = 1 - ri
	Q = 1 - rd

Then
	f.f(x) = ri + ri*P + x*P^2
	g.g(x) = x * Q^2

I'm going to skip some steps and just say
	f^n(x) = ri + ri*P + ... + ri*P^(n-1) + x*P^n
	g^n(x) = x*Q^n

Let
	h(n) = sum(1 <= i < n, ri * P^i)
		= ri * sum(1 <= i < n, P^i)
Then
	f^n(x) = h(n) + x*P^n
	f^m.g^n(x) = h(m) + (x * P^m * Q^n)
	g^n.f^m(x) = (h(m) + x * P^m) * Q^n

The real question of course is, how the hell can I SIMD the thing?
*/

// small LUT - pentium has a cache! (but not a very big one)
uint32_t lut_4b1to8[16] = {
	0x00000000,
	0x000000FF,
	0x0000FF00,
	0x0000FFFF,
	0x00FF0000,
	0x00FF00FF,
	0x00FFFF00,
	0x00FFFFFF,
	0xFF000000,
	0xFF0000FF,
	0xFF00FF00,
	0xFF00FFFF,
	0xFFFF0000,
	0xFFFF00FF,
	0xFFFFFF00,
	0xFFFFFFFF,
};

//define OPTIMP

#ifdef OPTIMP
// TODO: get this accurate
void au_decompress(int *fq, int *q, int *s, int *lt, int fs, int ri, int rd, int len, int8_t *outbuf, uint8_t *inbuf)
{
	int i,j;
	uint8_t d;
	
	int8_t tbuf[8];
	uint8_t dtbuf[8];
	
	int rx = ri^rd;
	
	for(i = 0; i < len; i++)
	{
		// load bits
		int d = *(inbuf++);
		
		// use last target to determine turn (for antijerk + strength)
		int d2 = ((d<<1)|(*lt))&255;
		
		// build halves
		uint32_t vl = lut_4b1to8[d&15];
		uint32_t vh = lut_4b1to8[d>>4];
		*(uint32_t *)(&tbuf[0])  = vl^0x80808080;
		*(uint32_t *)(&tbuf[4])  = vh^0x80808080;
		*(uint32_t *)(&dtbuf[0]) = ~(vl^lut_4b1to8[d2&15]);
		*(uint32_t *)(&dtbuf[4]) = ~(vh^lut_4b1to8[d2>>4]);
		
		// loop through
		for(j = 0; j < 8; j++)
		{
			int t = tbuf[j];
			int st = dtbuf[j];
			int sr = ri^(rx & st);
			
			// adjust charge
			int nq = *q + ((*s * (t-*q) + 0x80)>>8);
			if(nq == *q && nq != t)
				*q += (t == 127 ? 1 : -1);
				//*q += (int)(int8_t)(~(((int8_t)t)<<1));
			int lq = *q;
			*q = nq;
			
			// adjust strength
			int ns = *s + ((sr*(st-*s) + 0x80)>>8);
			if(ns == *s && ns != st)
				ns += (st == 255 ? 1 : -1);
				//ns += (int)(int8_t)((~((int8_t)st))|(int8_t)1);
			*s = ns;
			
			// FILTER: perform antijerk
			int ov = (t != *lt ? (nq+lq)>>1 : nq);
			//int ov = (st ? (nq+lq)>>1 : nq);
			
			// FILTER: perform LPF
			// NOTE: fs ignored and treated as 128
			*fq += ((ov-*fq+2)>>2);
			ov = *fq;
			
			// output sample
			*(outbuf++) = ov;
			
			*lt = t;
		}
		
		// TODO!
	}
	
	_mm_empty();
}
#else
void au_decompress(int *fq, int *q, int *s, int *lt, int fs, int ri, int rd, int len, int8_t *outbuf, uint8_t *inbuf)
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
			
			// FILTER: perform LPF
			*fq += ((fs*(ov-*fq) + 0x80)>>8);
			ov = *fq;
			
			// output sample
			*(outbuf++) = ov;
			
			*lt = t;
		}
	}
}
#endif

int main(int argc, char *argv[])
{
	int8_t rawbuf[1024];
	uint8_t cmpbuf[128];
	
	int q = 0;
	int s = 0;
	int lt = -128;
	int ri = 7;
	int rd = 20;
	
	int fq = 0;
	int fs = 100;
	
	FILE *infp = fopen("/dev/stdin","rb");
	FILE *outfp = fopen("/dev/stdout","wb");
	
	while(!feof(infp))
	{
		int i;
		for(i = 0; i < 128; i++)
			cmpbuf[i] = fgetc(infp);
		//fprintf(stderr, "%i\n", fgetc(infp));
		au_decompress(&fq, &q, &s, &lt, fs, ri, rd, 128, rawbuf, cmpbuf);
		
		fwrite(rawbuf, 128*8, 1, outfp);
	}
	
	fclose(outfp);
	fclose(infp);
	
	return 0;
}

