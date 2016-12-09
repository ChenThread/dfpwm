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

#include "dfpwmodel.h"

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

