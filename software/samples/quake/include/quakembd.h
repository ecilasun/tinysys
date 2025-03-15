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

#ifndef __QUAKEMBD_H
#define __QUAKEMBD_H

#include <stdint.h>
#include <stdlib.h>

/* Use stdio printf by default */
#ifndef QEMBD_PRINTF
#include <stdio.h>
#define QEMBD_PRINTF printf
#endif

#ifndef QEMBD_LOGGING_TAG
#define QEMBD_LOGGING_TAG "QUAKEMBD"
#endif

//#define FMT(X) "\033[" X "m"
#define FMT(X) 

#ifdef QEMBD_ENABLE_DEBUG
	#define qembd_debug(msg, ...) QEMBD_PRINTF(FMT("36") QEMBD_LOGGING_TAG " [DEBUG]: " msg FMT("0") "\r\n", ##__VA_ARGS__)
	#ifdef QEMBD_ENABLE_TRACE
		#define qembd_trace(msg, ...) QEMBD_PRINTF(QEMBD_LOGGING_TAG " [TRACE]: " msg "\r\n", ##__VA_ARGS__)
	#else
		#define qembd_trace(msg, ...)
	#endif
#else
	#define qembd_debug(msg, ...)
	#define qembd_trace(msg, ...)
#endif
#define qembd_info(msg, ...) QEMBD_PRINTF(FMT("32") QEMBD_LOGGING_TAG " [INFO]: " msg FMT("0") "\r\n", ##__VA_ARGS__)
#define qembd_warn(msg, ...) QEMBD_PRINTF(FMT("33") QEMBD_LOGGING_TAG " [WARN]: " msg FMT("0") "\r\n", ##__VA_ARGS__)
#define qembd_error(msg, ...) QEMBD_PRINTF(FMT("31") QEMBD_LOGGING_TAG " [ERROR]: " msg FMT("0") "\r\n", ##__VA_ARGS__)

#define bail_if_error(X, COND, msg) { if ((X) != (COND)) { qembd_error(msg ": %d", X); goto bail; } }
#define bail_if_null(X, msg) { if ((X) == NULL) { qembd_error(msg); goto bail; } }
#define bail(msg) { qembd_error(msg); goto bail; }

typedef struct {
	uint32_t keycode;
	uint8_t state;
} key_event_t;

typedef struct {
	int32_t x, y, xrel, yrel;
} mouse_motion_t;

typedef struct {
	int32_t x, y;
} mouse_movement_t;

int qembd_get_width();
int qembd_get_height();
void qembd_vidinit();
void qembd_fillrect(uint8_t *src, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize);
void qembd_refresh();
uint64_t qembd_get_us_time();
void qembd_udelay(uint32_t us);
void *qembd_allocmain(size_t size);
int qembd_main(int c, char **v);
int qembd_dequeue_key_event(key_event_t *e);
int qembd_get_mouse_movement(mouse_movement_t *movement);

#endif /* __QUAKEMBD_H */
