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

#define DFPWM_DECOMPRESS
#include "dfpwmodel.h"
#undef DFPWM_DECOMPRESS

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int16_t rawbuf[1024*2];
	uint8_t cmpbuf[32768];

	int q[2] = {0, 0};
	int s[2] = {0, 0};
	int lt[2] = {-32768, -32768};
	int exc[2] = {EXC_TABLE_MID, EXC_TABLE_MID};

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
		au_decompress(&fq[0], &q[0], &s[0], &lt[0], &exc[0], &lq[0], 128, rawbuf, cmpbuf);

		fwrite(rawbuf, 128*16*(STEREO ? 1 : 2), 1, outfp);
	}

	fclose(outfp);
	fclose(infp);

	return 0;
}

