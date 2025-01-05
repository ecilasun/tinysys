/*
 * Copyright (C) 2022 National Cheng Kung University, Taiwan.
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
#include <quakedef.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "basesystem.h"
#include "serialinringbuffer.h"
#include "task.h"
#include "keyboard.h"

enum {
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_MIDDLE = 2,
	MOUSE_BUTTON_RIGHT = 3,
};

enum {
	KEY_EVENT = 0,
	MOUSE_MOTION_EVENT = 1,
	MOUSE_BUTTON_EVENT = 2,
	QUIT_EVENT = 3,
};

typedef struct {
	uint8_t button;
	uint8_t state;
} mouse_button_t;

typedef struct {
	uint32_t type;
	union {
		key_event_t key_event;
		union {
			mouse_motion_t motion;
			mouse_button_t button;
		} mouse;
	};
} event_t;

/*typedef struct {
	event_t *base;
	size_t start;
} event_queue_t;*/

enum {
	RELATIVE_MODE_SUBMISSION = 0,
	WINDOW_TITLE_SUBMISSION = 1,
};

typedef struct {
	uint8_t enabled;
} mouse_submission_t;

typedef struct {
	uint32_t title;
	uint32_t size;
} title_submission_t;

/*typedef struct {
	uint32_t type;
	union {
	   mouse_submission_t mouse;
	   title_submission_t title;
	};
} submission_t;*/

/*typedef struct {
	submission_t *base;
	size_t end;
} submission_queue_t;*/

/*static const int queues_capacity = 128;
static unsigned int event_count;
static event_queue_t event_queue = {
	.base = NULL,
	.start = 0,
};*/
/*static submission_queue_t submission_queue = {
	.base = NULL,
	.end = 0,
};*/
//static event_t event;
static mouse_movement_t mouse_movement;

uint64_t qembd_get_us_time()
{
	//uint64_t cur_time = (uint64_t) clock();
	uint64_t cur_time = E32ReadTime();
	cur_time = ClockToUs(cur_time);
	static int secbase;

	if (!secbase) {
		secbase = cur_time / 1000000;
		return (uint64_t) secbase;
	}

	return cur_time;
}

void qembd_udelay(uint32_t us)
{
	uint64_t start = qembd_get_us_time(), end;
	end = start;
	while (end - start < 1)
		end = qembd_get_us_time();
}

/*void qembd_set_relative_mode(bool enabled) {
	submission_t submission;
	submission.type = RELATIVE_MODE_SUBMISSION;
	submission.mouse.enabled = enabled;
	submission_queue.base[submission_queue.end++] = submission;
	submission_queue.end &= queues_capacity - 1;
	register int a0 asm("a0") = 1;
	register int a7 asm("a7") = 0xfeed;
	asm volatile("scall" : "+r"(a0) : "r"(a7));
}*/

int main(int c, char **v)
{
	/*void *base = malloc(sizeof(event_t) * queues_capacity + 
						sizeof(submission_t) * queues_capacity);
	event_queue.base = base;
	submission_queue.base = base + sizeof(event_t) * queues_capacity;
	register int a0 asm("a0") = (uintptr_t) base;
	register int a1 asm("a1") = queues_capacity;
	register int a2 asm("a2") = (uintptr_t) (&event_count);
	register int a7 asm("a7") = 0xc0de;
	asm volatile("scall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
	qembd_set_relative_mode(true);*/

	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 1;

	return qembd_main(c, v);
}

void *qembd_allocmain(size_t size)
{
	return malloc(size);
}

static int poll_event()
{
	/*if (event_count <= 0)
		return 0;
	event = event_queue.base[event_queue.start++];
	event_queue.start &= queues_capacity - 1;
	--event_count;

	if (event.type == MOUSE_MOTION_EVENT) {
		mouse_movement.x += event.mouse.motion.xrel;
		mouse_movement.y += event.mouse.motion.yrel;
	}*/

	return 0;
}

int qembd_dequeue_key_event(key_event_t *e)
{
	uint8_t byte;
	if (SerialInRingBufferRead(&byte, 1))
	{
		// Keyboard packet
		if (byte == '^')
		{
			uint8_t scandata[KEYBOARD_PACKET_SIZE];
			ReadKeyState(scandata);

			uint8_t scancode = scandata[KEYBOARD_SCANCODE_INDEX];
			uint8_t modifiers_lower = scandata[KEYBOARD_MODIFIERS_LOWER_INDEX];
			uint8_t modifiers_upper = scandata[KEYBOARD_MODIFIERS_UPPER_INDEX];
			uint32_t modifiers = (modifiers_upper << 8) | modifiers_lower;

			// if (ascii == 27) // ESC
			// 	done = 1;

			// See keyboard scan codes for more info
			switch(scancode)
			{
				case 88:	{ e->keycode = K_ENTER; break; }
				case 40:	{ e->keycode = K_ENTER; break; }
				case 79:	{ e->keycode = K_RIGHTARROW; break; }
				case 80:	{ e->keycode = K_LEFTARROW; break; }
				case 81:	{ e->keycode = K_DOWNARROW; break; }
				case 82:	{ e->keycode = K_UPARROW; break; }
				case 41:	{ e->keycode = K_ESCAPE; break; }
				case 43:	{ e->keycode = K_TAB; break; }
				case 42:	{ e->keycode = K_BACKSPACE; break; }
				case 229:	{ e->keycode = K_SHIFT; break; }
				case 228:	{ e->keycode = K_CTRL; break; }
				case 230:	{ e->keycode = K_ALT; break; }
				case 226:	{ e->keycode = K_ALT; break; }
				case 72:	{ e->keycode = K_PAUSE; break; }
				case 58:	{ e->keycode = K_F1; break; }
				case 59:	{ e->keycode = K_F2; break; }
				case 60:	{ e->keycode = K_F3; break; }
				case 61:	{ e->keycode = K_F4; break; }
				case 62:	{ e->keycode = K_F5; break; }
				case 63:	{ e->keycode = K_F6; break; }
				case 64:	{ e->keycode = K_F7; break; }
				case 65:	{ e->keycode = K_F8; break; }
				case 66:	{ e->keycode = K_F9; break; }
				case 67:	{ e->keycode = K_F10; break; }
				case 68:	{ e->keycode = K_F11; break; }
				case 69:	{ e->keycode = K_F12; break; }
				case 74:	{ e->keycode = K_HOME; break; }
				case 77:	{ e->keycode = K_END; break; }
				case 75:	{ e->keycode = K_PGUP; break; }
				case 78:	{ e->keycode = K_PGDN; break; }
				case 73:	{ e->keycode = K_INS; break; }
				case 76:	{ e->keycode = K_DEL; break; }
				default:	{ e->keycode = KeyboardScanCodeToASCII(scancode, 0); break; }
			}
			e->state = scandata[KEYBOARD_STATE_INDEX];
			return 0;
		}
	}
	return -1;
}

int qembd_get_mouse_movement(mouse_movement_t *movement)
{
	*movement = mouse_movement;
	mouse_movement.x = 0;
	mouse_movement.y = 0;
	return 0;
}
