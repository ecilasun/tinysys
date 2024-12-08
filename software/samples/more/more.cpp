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
		UARTPrintf("Usage: more filename\n");
	else
	{
		FILE *fp = fopen(argv[1], "rb");
		if (fp)
		{
			UARTPrintf("\n");
			uint8_t *buffer = new uint8_t[512];
			int readsize = 0;
			do
			{
				readsize = fread(buffer, 1, 128, fp);
				for (int i=0; i<readsize; ++i)
					UARTPrintf("%c", buffer[i]);
			} while (readsize > 0);
			fclose(fp);
		}
		else
			UARTPrintf("File '%s' not found, please use full path\n", argv[1]);
	}

	return 0;
}
