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

// Looks pretty ordinary, doesn't it? :)
int main(int argc, char *argv[])
{
	printf("Welcome to tinysys!\n");

	// This program simply prints a welcome message and exits.
	// Alternatively, one could sit in an infinite loop here, waiting for the user to hit CTRL+C to exit.
	return 0;
}
