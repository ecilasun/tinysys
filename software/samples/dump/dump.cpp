/** \file
 * Hexadecimal and ASCII dump of a file.
 *
 * \ingroup examples
 * This program reads a file and dumps its content in hexadecimal and ASCII format.
 * It is useful to inspect the content of a file, especially binary files.
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
		UARTPrintf("Usage: dump filename\n");
	else
	{
		FILE *fp = fopen(argv[1], "rb");
		if (fp)
		{
			UARTPrintf("\n");
			uint8_t *buffer = new uint8_t[16];
			int readsize = 0;
			do
			{
				readsize = fread(buffer, 1, 16, fp);

				for (int i=0; i<readsize; ++i)
					UARTPrintf("%02X ", buffer[i]);

				for (int i=0; i<readsize; ++i)
					if (buffer[i]>=32)
						UARTPrintf("%c", buffer[i]);
					else
						UARTPrintf(".");
				UARTPrintf("\n");
			} while (readsize > 0);
			fclose(fp);
		}
		else
			UARTPrintf("File '%s' not found, please use full path\n", argv[1]);
	}

	return 0;
}
