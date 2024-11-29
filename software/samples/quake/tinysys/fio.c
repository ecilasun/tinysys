/*
 * Copyright (C) 2020 Shotaro Uchida <fantom@xmaker.mx>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <quakembd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define	MAX_HANDLES 10
FILE *sys_handles[MAX_HANDLES];

static int find_handle(void)
{
	for (int i = 1; i < MAX_HANDLES; i++) {
		if (!sys_handles[i])
			return i;
	}

	qembd_error("out of handles");
	return -1;
}

static int Qfilelength (FILE *f)
{
	int pos = ftell(f);
	fseek (f, 0, SEEK_END);
	int end = ftell(f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead(char *path, int *handle)
{
	int i = find_handle();

	FILE *f = fopen(path, "rb");
	if (!f) {
		*handle = -1;
		return -1;
	}

	sys_handles[i] = f;
	*handle = i;

	return Qfilelength(f);
}

int Sys_FileOpenWrite(char *path)
{
	int i = find_handle();

	FILE *f = fopen(path, "wb");
	if (!f)
		qembd_error("Error opening %s: %s", path, strerror(errno));

	sys_handles[i] = f;
	return i;
}

void Sys_FileClose(int handle)
{
	if (handle < 0)
		return;

	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek(int handle, int position)
{
	if (handle < 0)
		return;

	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead(int handle, void *dest, int count)
{
	int size = 0;
	if (handle >= 0) {
		char *data = dest;
		while (count > 0) {
			int done = fread(data, 1, count, sys_handles[handle]);
			if (done == 0)
				break;

			data += done;
			count -= done;
			size += done;
		}
	}
	return size;
}

int Sys_FileWrite(int handle, void *src, int count)
{
	int size = 0;
	if (handle >= 0) {
		char *data = src;
		while (count > 0) {
			int done = fread(data, 1, count, sys_handles[handle]);
			if (done == 0)
				break;

			data += done;
			count -= done;
			size += done;
		}
	}
	return size;
}

int Sys_FileTime(char *path)
{
	FILE *f = fopen(path, "rb");
	if (f) {
		fclose(f);
		return 1;
	}

	return -1;
}

void Sys_mkdir(char *path)
{
}

void Sys_FileSync(int handle)
{
}

void Sys_File_gets(int handle, char *buf, int len)
{
	FILE *fp = fdopen(handle, "r");
	fgets(buf, len, fp);
	fclose(fp);
}
