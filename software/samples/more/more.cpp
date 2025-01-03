/** \file
 * Text dump example
 *
 * \ingroup examples
 * This example reads a file and dumps it to the console in ASCII format.
 */

#include <inttypes.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include "uart.h"

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printf("Usage: more filename\n");
		UARTPrintf("Usage: more filename\n");
	}
	else
	{
		FILE *fp = fopen(argv[1], "rb");
		if (fp)
		{
			printf("\n");
			UARTPrintf("\n");
			uint8_t *buffer = new uint8_t[512];
			int readsize = 0;
			do
			{
				readsize = fread(buffer, 1, 128, fp);
				for (int i=0; i<readsize; ++i)
				{
					printf("%c", buffer[i]);
					UARTPrintf("%c", buffer[i]);
				}
			} while (readsize > 0);
			fclose(fp);
		}
		else
		{
			printf("File '%s' not found, please use full path\n", argv[1]);
			UARTPrintf("File '%s' not found, please use full path\n", argv[1]);
		}
	}

	printf("\n");
	UARTPrintf("\n");

	return 0;
}
