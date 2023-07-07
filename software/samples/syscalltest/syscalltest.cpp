#define _GNU_SOURCE 1

#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct
{
    // Should be "IWAD" or "PWAD".
    char                identification[4];
    int                 numlumps;
    int                 infotableofs;

} wadinfo_t;

int main()
{
	char pathbuffer[512] = "";
	getcwd(pathbuffer, 512);
	printf("Current work directory:%s\n", pathbuffer);

	printf("Changing it to sd:/\n");
	chdir("sd:/");

	FILE *fp = fopen("sd:test.jpg", "rb");
	if (fp)
	{
		printf("File's there, doing seek/read check.\n");

		// Grab a tiny memory chunk
		uint8_t *buffer = new uint8_t[8];

		// Read 16 byte blocks
		for (uint32_t i=0;i<5;++i)
		{
			fread(buffer, 8, 1, fp);
			for (uint32_t j=0;j<8;++j)
				printf("%.2X ", buffer[j]);
			printf("\n");
		}
		// Read non-adjacent blocks
		for (uint32_t i=0;i<5;++i)
		{
			fseek(fp, i*32, SEEK_SET);
			fread(buffer, 8, 1, fp);
			for (uint32_t j=0;j<8;++j)
				printf("%.2X ", buffer[j]);
			printf("\n");
		}

		delete [] buffer;

		fclose(fp);

		printf("Now the same with read() instead of fread()\n");
		int handle;
		if ( (handle = open ("sd:/doom1.wad", O_RDONLY /*| O_BINARY*/)) != -1)
		{
			printf("Open succeeded, reading\n");
			wadinfo_t header;
        	read (handle, &header, sizeof(header));
			printf("header: %c%c%c%c\n",
			header.identification[0],
			header.identification[1],
			header.identification[2],
			header.identification[3] );
			close (handle);
		}
		else
			printf("Oops, can't open sd:doom1.wad\n");
	}
	else
		printf("File's not there.\n");

	printf("Let's see how writing files go\n");
	fp = fopen("sd:/testout.txt", "w");
	if (fp)
	{
		fprintf(fp, "Hello, world!\n");
		fclose(fp);
	}
	else
		printf("Failed to open file for write.\n");

	return 0;
}
