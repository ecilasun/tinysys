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

#include <quakedef.h>
#include <quakembd.h>

#define DEFAULT_MEM_SIZE (8 * 1024 * 1024)
#define DEFAULT_BASEDIR "quakembd"
#define DEFAULT_CACHEDIR "/tmp"

qboolean isDedicated;

cvar_t  sys_linerefresh = {"sys_linerefresh","0"};// set for entity display

// =======================================================================
// Logging
// =======================================================================

#ifdef WINQUAKE_LOGGING_EXTERNAL
#include <stdio.h>
#define DEFAULT_LOG_BUFFER_SIZE 1024

void _Sys_Printf(const char *fmt, ...)
{
	va_list argptr;
	char text[DEFAULT_LOG_BUFFER_SIZE];
	unsigned char *p;

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (strlen(text) > DEFAULT_LOG_BUFFER_SIZE) {
		qembd_error("memory overwrite in Sys_Printf");
		return;
	}

	for (p = (unsigned char *) text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			fprintf(stdout, "[%02x]", *p);
		else
			putc(*p, stdout);
	}
}
#endif

void Sys_Error(char *error, ...)
{ 
	va_list argptr;
	char string[1024];

	va_start (argptr, error);
	vsprintf (string, error, argptr);
	va_end (argptr);
	fprintf(stderr, "Error: %s\n", string);

	Host_Shutdown ();
	exit (1);
}

// =======================================================================
// General routines
// =======================================================================

void Sys_Quit(void)
{
	Host_Shutdown();
	exit(0);
}

double Sys_FloatTime(void)
{
	return qembd_get_us_time() / 1000000.0;
}

char *Sys_ConsoleInput(void)
{
	// TODO
	return NULL;
}

void Sys_Sleep(void)
{
}

void Sys_SendKeyEvents(void)
{
	key_event_t e;
	while (qembd_dequeue_key_event(&e) == 0)
		Key_Event(e.keycode, e.state == 1);
}

void Sys_HighFPPrecision(void)
{
}

void Sys_LowFPPrecision(void)
{
}

void Sys_MakeCodeWriteable(unsigned long startaddr, unsigned long length)
{
}

int qembd_main(int c, char **v)
{
	float time, oldtime, newtime;
	quakeparms_t parms = {0};
	int j;

	COM_InitArgv(c, v);
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = DEFAULT_MEM_SIZE;
	j = COM_CheckParm("-mem");
	if (j)
		parms.memsize = (int) (Q_atof(com_argv[j+1]) * 1024 * 1024);
	parms.membase = qembd_allocmain(parms.memsize);
	if (!parms.membase) {
		qembd_error("Memory cannot be allocated");
		return -1;
	}

	parms.basedir = DEFAULT_BASEDIR;
// caching is disabled by default, use -cachedir to enable
//	parms.cachedir = DEFAULT_CACHEDIR;

	Host_Init(&parms);

//	if (COM_CheckParm("-nostdout")) {
//		nostdout = 1;
//	}

	qembd_info("QuakEMBD - Based on WinQuake %0.3f", VERSION);

	oldtime = Sys_FloatTime() - 0.1;
	while (1) {
		// find time spent rendering last frame
		newtime = Sys_FloatTime();
		time = newtime - oldtime;

		if (time > sys_ticrate.value*2)
			oldtime = newtime;
		else
			oldtime += time;

		Host_Frame(time);

#if 0
		// graphic debugging aids
		if (sys_linerefresh.value)
			Sys_LineRefresh ();
#endif
	}
}
