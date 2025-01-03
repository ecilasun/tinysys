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
	{
		printf("Usage: dump filename\n");
		UARTPrintf("Usage: dump filename\n");
	}
	else
	{
		FILE *fp = fopen(argv[1], "rb");
		if (fp)
		{
			printf("\n");
			UARTPrintf("\n");
			uint8_t *buffer = new uint8_t[16];
			int readsize = 0;
			do
			{
				readsize = fread(buffer, 1, 16, fp);
				if (readsize != 0)
				{
					for (int i=0; i<readsize; ++i)
					{
						printf("%02X ", buffer[i]);
						UARTPrintf("%02X ", buffer[i]);
					}

					// In case the last line is not full (less than 16 bytes)
					for (int i=readsize; i<16; ++i)
					{
						printf(".. ");
						UARTPrintf(".. ");
					}

					for (int i=0; i<readsize; ++i)
					{
						if (buffer[i]>=32)
						{
							printf("%c", buffer[i]);
							UARTPrintf("%c", buffer[i]);
						}
						else
						{
							printf(".");
							UARTPrintf(".");
						}
					}

					for (int i=readsize; i<16; ++i)
					{
						printf(" ");
						UARTPrintf(" ");
					}
				}
				printf("\n");
				UARTPrintf("\n");
			} while (readsize > 0);
			fclose(fp);
		}
		else
		{
			printf("File '%s' not found, please use full path\n", argv[1]);
			UARTPrintf("File '%s' not found, please use full path\n", argv[1]);
		}
	}

	return 0;
}
