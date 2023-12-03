/*
 * mini-scanf - Minimal scanf() implementation for embedded projects. 
 * Copyright (c) 2023 Aleksej Muratov
 */

#ifndef _C_SCAN_H_
#define	_C_SCAN_H_

// conf
// sscanf / scanf
#define C_SSCANF
// %[..]
#define LENSCANS 10

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef EOF
#define EOF -1
#endif

int c_isspace(const int c);
int	c_isdigit(int c);

// scan
#ifdef C_SSCANF
	int c_sscanf(const char* buff, char* format, ...);
#else
	int c_scanf(char* format, ...);
#endif
char c_getch(); //custom
bool c_getbackch(char b); //new

// HW
int getch();

#endif /* _C_SCAN_H_ */