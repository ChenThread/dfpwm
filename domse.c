#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	FILE *fp1 = fopen(argv[1], "rb");
	FILE *fp2 = fopen(argv[2], "rb");

	int64_t mse = 0;
	for(;;)
	{
		int v1 = fgetc(fp1);
		int v2 = fgetc(fp2);
		if(v1 < 0 || v2 < 0) {
			break;
		}

		v1 = (int)(int8_t)v1;
		v2 = (int)(int8_t)v2;
		v1 -= v2;
		v1 *= v1;
		mse += (int64_t)v1;
	}

	printf("%lld", (long long)mse);
	fflush(stdout);

	fclose(fp2);
	fclose(fp1);
	return 0;
}

