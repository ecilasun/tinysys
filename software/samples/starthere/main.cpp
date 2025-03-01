/** \file
 * Start here!
 *
 * \ingroup examples
 * This is the main file for your first tinysys example. It's a simple program that prints a welcome message
 * and exits. You can use this as a starting point for your own programs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "core.h"
#include "basesystem.h"
#include "leds.h"

// Looks pretty ordinary, doesn't it? :)
int main(int argc, char *argv[])
{
	printf("Welcome to tinysys!\n");

	int counter = 0;
	while (1)
	{
		LEDSetState(counter);
		E32Sleep(1000);
		++counter;
	}

	return 0;
}
