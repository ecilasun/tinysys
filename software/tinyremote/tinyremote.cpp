#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <chrono>
#include <thread>
#include <filesystem>

#include "tinyremote.h"

static bool s_alive = true;
static SDL_Window* s_window;
static SDL_Surface* s_outputSurface;
static SDL_Surface* s_surface;
static uint8_t *s_videodata;
static int s_videoWidth;
static int s_videoHeight;
static int s_windowWidth, s_prevWidth;
static int s_windowHeight, s_prevHeight;
static bool s_maximized;
static bool s_restored;
static float s_uploadProgress = 0.f;
static int s_showProgress = 0;

uint32_t videoCallback(uint32_t interval, void* param)
{
	VideoCapture* video_capture = (VideoCapture*)param;

	bool haveFrame = video_capture ? video_capture->CaptureFrame(s_videodata) : false;
	if (haveFrame)
	{
		if (SDL_MUSTLOCK(s_outputSurface))
			SDL_LockSurface(s_outputSurface);

		uint32_t* pixels = (uint32_t*)s_outputSurface->pixels;
		uint32_t* vid = (uint32_t*)s_videodata;
		for (int y = 0; y < s_videoHeight; ++y)
		{
			for (int x = 0; x < s_videoWidth; ++x)
			{
				pixels[(y * s_outputSurface->w + x)] = vid[y*s_videoWidth+x];
			}
		}

		if (SDL_MUSTLOCK(s_outputSurface))
			SDL_UnlockSurface(s_outputSurface);

		if (s_windowWidth != s_prevWidth || s_windowHeight != s_prevHeight)
		{
			s_prevWidth = s_windowWidth;
			s_prevHeight = s_windowHeight;

			SDL_FreeSurface(s_surface);

			s_surface = SDL_GetWindowSurface(s_window);

			fprintf(stderr, "Window resized to %dx%d", s_windowWidth, s_windowHeight);
			if (s_maximized)
			{
				s_maximized = false;
				//SDL_SetWindowBordered(s_window, SDL_FALSE);
				fprintf(stderr, " and maximized\n");
			}
			else if (s_restored)
			{
				s_restored = false;
				//SDL_SetWindowBordered(s_window, SDL_TRUE);
				fprintf(stderr, " and restored\n");
			}
			else
				fprintf(stderr, "\n");
		}

		SDL_Rect srcRect = { 0, 0, s_videoWidth, s_videoHeight };
		SDL_Rect dstRect = { 0, 0, s_windowWidth, s_windowHeight };
		SDL_BlitScaled(s_outputSurface, &srcRect, s_surface, &dstRect);

		if (s_showProgress)
		{
			// Show a 512 pixel wide 20 pixel high progress bar centered inside the current window
			int progressWidth = 512;
			int progressHeight = 20;
			int progressX = (s_windowWidth - progressWidth) / 2;
			int progressY = (s_windowHeight - progressHeight) / 2;
			SDL_Rect progressRect = { progressX-1, progressY-1, progressWidth+2, progressHeight+2 };
			SDL_FillRect(s_surface, &progressRect, SDL_MapRGB(s_surface->format, 0, 0, 0));
			SDL_Rect progressRectInner = { progressX, progressY, progressWidth, progressHeight };
			progressRectInner.w = int((progressWidth * s_uploadProgress) / 100.f);
			SDL_FillRect(s_surface, &progressRectInner, SDL_MapRGB(s_surface->format, 255, 255, 255));
		}

		SDL_UpdateWindowSurface(s_window);
	}

	return interval;
}

uint32_t audioCallback(uint32_t interval, void* param)
{
	AudioCapture* audio_capture = (AudioCapture*)param;

	audio_capture->Update();

	return interval;
}

bool WACK(CSerialPort *_serial, const uint8_t waitfor, uint8_t& received)
{
	int ack = 0;
	uint8_t dummy;
	uint8_t bytes = 0;
	uint32_t timeout = 0;
	do
	{
		if (_serial->Receive(&dummy, 1))
		{
			received = dummy;
			++bytes;
			if (dummy == waitfor)
				ack = 1; // Valid ACK
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		++timeout;
	} while (!bytes);

	return (timeout>16384) ? false : (ack ? true : false);
}

unsigned int getfilelength(const fpos_t &endpos)
{
#if defined(CAT_LINUX)
	return (unsigned int)endpos.__pos;
#elif defined(CAT_MACOS)
	return (unsigned int)endpos;
#else // CAT_WINDOWS
	return (unsigned int)endpos;
#endif
}

void ConsumeInitialTraffic(CSerialPort* _serial)
{
	// When we first open the port, ESP32 will respond with a "ESP-ROM:esp32xxxxxx" and tinysys version message
	// or sometimes with nothing. We need to consume this traffic before we can send any commands
	uint8_t startup = 0;
	while (_serial->Receive(&startup, 1))
	{
		// This usually reads something similar to "ESP-ROM:esp32s3-*"
		// It is likely that some other unfinished traffic will show up here
		printf("%c", startup);
	}
}

void SendFile(char *_filename, CSerialPort* _serial)
{
	char tmpstring[129];

	FILE *fp;
	fp = fopen(_filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "ERROR: can't open ELF file %s\n", _filename);
		return;
	}

	char cleanfilename[129];
	{
		using namespace std::filesystem;
		path p = absolute(_filename);
		strcpy(cleanfilename, p.filename().string().c_str());
	}

	unsigned int filebytesize = 0;
	fpos_t pos, endpos;
	fgetpos(fp, &pos);
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fsetpos(fp, &pos);
	filebytesize = getfilelength(endpos);

	uint8_t *filedata = new uint8_t[filebytesize + 64];
	fread(filedata, 1, filebytesize, fp);
	fclose(fp);

	// Send the file bytes in chunks
	uint32_t packetSize = 1024;
	int worstSize = LZ4_compressBound(filebytesize);

	// Pack the data before sending it across
	char* encoded = new char[worstSize];
	uint32_t encodedSize = LZ4_compress_default((const char*)filedata, encoded, filebytesize, worstSize);
	fprintf(stderr, "Compression ratio = %.2f%% (%d->%d bytes)\n", 100.f*float(encodedSize)/float(filebytesize), filebytesize, encodedSize);

	// Now we can work out how many packets we need to send
	uint32_t numPackets = encodedSize / packetSize;
	uint32_t leftoverBytes = encodedSize % packetSize;

	uint8_t received;

	ConsumeInitialTraffic(_serial);

	s_uploadProgress = 0.f;
	s_showProgress = 1;

	// Start the receiver app on the other end
	snprintf(tmpstring, 128, "recv");
	_serial->Send((uint8_t*)tmpstring, 4);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	snprintf(tmpstring, 128, "\n");
	_serial->Send((uint8_t*)tmpstring, 1);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "Transfer initiation error: '%c'\n", received);
		delete [] filedata;
		s_showProgress = 0;
		return;
	}

	// Send encoded length
	uint32_t encodedLen = encodedSize;
	_serial->Send(&encodedLen, 4);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "Encoded buffer length error: '%c'\n", received);
		delete [] filedata;
		s_showProgress = 0;
		return;
	}

	// Send actual length
	uint32_t decodedLen = filebytesize;
	_serial->Send(&decodedLen, 4);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "Decoded buffer length error: '%c'\n", received);
		delete [] filedata;
		s_showProgress = 0;
		return;
	}

	// Send name length
	uint32_t nameLen = strlen(cleanfilename);
	_serial->Send(&nameLen, 4);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "File name length error: '%c'\n", received);
		delete [] filedata;
		s_showProgress = 0;
		return;
	}

	// Send name
	_serial->Send(cleanfilename, nameLen);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "File name error: '%c'\n", received);
		delete [] filedata;
		s_showProgress = 0;
		return;
	}

	uint32_t i = 0;
	uint32_t packetOffset = 0;
	char progress[65];
	for (i=0; i<64; ++i)
		progress[i] = ' ';//176;
	progress[64] = 0;
	fprintf(stderr, "Uploading '%s' (packet size: %dx%d+%d bytes, packer buffer: %d bytes)\n", cleanfilename, numPackets, packetSize, leftoverBytes, worstSize);
	for (i=0; i<numPackets; ++i)
	{
		const char* source = (const char*)(encoded + packetOffset);

		s_uploadProgress = (i*100)/float(numPackets);
		int idx = (i*64)/numPackets;
		for (int j=0; j<=idx; ++j) // Progress bar
			progress[j] = '=';//219;
		fprintf(stderr, "\r [%s] %.2f%%\r", progress, s_uploadProgress);

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet size error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			s_showProgress = 0;
			return;
		}

		_serial->Send(&packetSize, 4);

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			s_showProgress = 0;
			return;
		}

		_serial->Send((void*)source, packetSize);

		packetOffset += packetSize;
	}

	if (leftoverBytes)
	{
		const char* source = (const char*)(encoded + packetOffset);

		s_uploadProgress = 100.f;
		for (int j=0; j<64; ++j) // Progress bar
			progress[j] = '=';//219;
		fprintf(stderr, "\r [%s] %.2f%%\r", progress, s_uploadProgress);

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet size error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			s_showProgress = 0;
			return;
		}

		_serial->Send(&leftoverBytes, 4);

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			s_showProgress = 0;
			return;
		}

		_serial->Send((void*)source, packetSize);

		packetOffset += leftoverBytes;
	}

	fprintf(stderr, "\r\n");

	delete[] encoded;
	s_showProgress = 0;

	fprintf(stderr, "%d bytes uploaded\n", packetOffset);

	delete [] filedata;
}

float Clamp(float value, float min, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

void ClampAnalogInput(Axis6& input, float innerDeadzone, float outerDeadzone)
{
    float magleft = input.leftx*input.leftx + input.lefty*input.lefty;
	float magright = input.rightx*input.rightx + input.righty*input.righty;

	if (magleft == 0.f)
	{
		input.leftx = 0;
		input.lefty = 0;
	}
	else
	{
		magleft = sqrtf(magleft);
		float scale = (magleft - innerDeadzone) / (outerDeadzone - innerDeadzone);
		input.leftx = input.leftx * Clamp(scale, 0, 1) / magleft;
		input.lefty = input.lefty * Clamp(scale, 0, 1) / magleft;
	}

	if (magright == 0.f)
	{
		input.rightx = 0;
		input.righty = 0;
	}
	else
	{
		magright = sqrtf(magright);
		float scale = (magright - innerDeadzone) / (outerDeadzone - innerDeadzone);
		input.rightx = input.rightx * Clamp(scale, 0, 1) / magright;
		input.righty = input.righty * Clamp(scale, 0, 1) / magright;
	}

	input.lefttrigger = Clamp(input.lefttrigger, 0, 1);
	input.righttrigger = Clamp(input.righttrigger, 0, 1);
}

void ControllerAdd(AppCtx& ctx)
{
	int numJoy = SDL_NumJoysticks();
	if (numJoy <= 0)
		fprintf(stderr, "No joysticks: %d\n", numJoy);
	else
		fprintf(stderr, "Found %d joysticks\n", numJoy);

	for (int i = 0; i < numJoy; ++i)
	{
		if (SDL_IsGameController(i))
		{
			ctx.gamecontroller = SDL_GameControllerOpen(i);
			if (ctx.gamecontroller)
			{
				if (SDL_GameControllerGetAttached(ctx.gamecontroller))
				{
					fprintf(stderr, "Using game controller #%d: '%s'\n", i, SDL_GameControllerName(ctx.gamecontroller));
					// Read button mappings
					for (int j = 0; j < SDL_CONTROLLER_BUTTON_MAX; ++j)
					{
						if (SDL_GameControllerHasButton(ctx.gamecontroller, (SDL_GameControllerButton)j))
						{
							SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(ctx.gamecontroller, (SDL_GameControllerButton)j);
							switch (bind.bindType)
							{
								case SDL_CONTROLLER_BINDTYPE_BUTTON:
									fprintf(stderr, "Button %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								break;
								case SDL_CONTROLLER_BINDTYPE_AXIS:
									fprintf(stderr, "Axis %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								break;
								case SDL_CONTROLLER_BINDTYPE_HAT:
									fprintf(stderr, "Hat %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								break;
								default:
								{
									fprintf(stderr, "Unknown %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								}
							}
						}
					}
					break;
				}
			}
		}
	}
}

void ControllerRemove(AppCtx& ctx)
{
	if (ctx.gamecontroller)
	{
		SDL_GameControllerClose(ctx.gamecontroller);
		ctx.gamecontroller = nullptr;
	}

}

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
	const char* cname = GetCommDeviceName();
	const char* vname = GetVideoDeviceName();
	const char* acname = GetAudioCaptureDeviceName();
	const char* apname = GetAudioPlaybackDeviceName();

	fprintf(stderr, "Usage: tinyremote commdevicenumber videodevname audiocapdevname audioplaydevname\ndefault comm device:%s default capture device:%s\nCtrl+C or PAUSE: quit current remote process\n", cname, vname);

	if (argc > 1)
		SetCommDeviceName(atoi(argv[1]));
	if (argc > 2)
		SetVideoDeviceName(argv[2]);
	if (argc > 3)
		SetAudioCaptureDeviceName(argv[3]);
	if (argc > 4)
		SetAudioPlaybackDeviceName(argv[4]);

	s_videoWidth = 640;
	s_videoHeight = 480;
	s_windowWidth = s_prevWidth = 640;
	s_windowHeight = s_prevHeight = 480;
	s_maximized = false;
	s_restored = false;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL2: %s\n", SDL_GetError());
		return -1;
	}

	s_window = SDL_CreateWindow("tinyremote", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, s_windowWidth, s_windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(s_window, s_videoWidth, s_videoHeight);

	s_surface = SDL_GetWindowSurface(s_window);
	s_outputSurface = SDL_CreateRGBSurfaceWithFormat(0, s_videoWidth, s_videoHeight, 32, SDL_PIXELFORMAT_ARGB8888);

	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1"); // Enable background events for joysticks

	s_videodata = new uint8_t[s_videoWidth*s_videoHeight*4];

	AppCtx ctx;
	ctx.gamecontroller = nullptr;
	ctx.serial = new CSerialPort();
	ctx.serial->AttemptOpen();
	ctx.video = new VideoCapture();
	ctx.video->Initialize(s_videoWidth, s_videoHeight);
	ctx.audio = new AudioCapture();
	ctx.audio->Initialize();

	SDL_TimerID videoTimer = SDL_AddTimer(16, videoCallback, ctx.video); // 60fps
	SDL_TimerID audioTimer = SDL_AddTimer(16, audioCallback, ctx.audio); // 60fps

	const uint8_t *keystates = SDL_GetKeyboardState(nullptr);
	uint8_t *old_keystates = new uint8_t[SDL_NUM_SCANCODES];
	memset(old_keystates, 0, SDL_NUM_SCANCODES);

	Axis6 prev_input;
	prev_input.leftx = -11111.f;
	prev_input.lefty = -11111.f;
	prev_input.rightx = -11111.f;
	prev_input.righty = -11111.f;
	prev_input.lefttrigger = -11111.f;
	prev_input.righttrigger = -11111.f;
	uint32_t prev_buttons = 0xFFFFFFFF;

	SDL_Event ev;
	do
	{
		if (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				s_alive = false;
			if (ev.type == SDL_DROPFILE)
				SendFile(ev.drop.file, ctx.serial);
			if (ev.type == SDL_CONTROLLERDEVICEADDED)
			{
				ControllerAdd(ctx);
			}
			if (ev.type == SDL_CONTROLLERDEVICEREMOVED)
			{
				ControllerRemove(ctx);
			}
			if (ev.type == SDL_WINDOWEVENT)
			{
				/*if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					// Can we clamp size to certain multiples here?
				}*/
				if (ev.window.event == SDL_WINDOWEVENT_MAXIMIZED)
				{
					fprintf(stderr, "Window maximized\n");
					s_maximized = true;
				}
				else if (ev.window.event == SDL_WINDOWEVENT_RESTORED)
				{
					fprintf(stderr, "Window restored\n");
					s_restored = true;
				}
				else if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					s_windowWidth = ev.window.data1;
					s_windowHeight = ev.window.data2;
				}
			}
		}

		// Echo serial data
		char inchar;
		if(ctx.serial->Receive(&inchar, 1))
			fprintf(stderr, "%c", inchar);

		// Read joystick events
		if (ctx.gamecontroller)
		{
			// Buttons
			uint32_t buttons = 0x00000000;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_A) ? 0x00000001 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_B) ? 0x00000002 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_X) ? 0x00000004 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_Y) ? 0x00000008 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_BACK) ? 0x00000010 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_GUIDE) ? 0x00000020 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_START) ? 0x00000040 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_LEFTSTICK) ? 0x00000080 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) ? 0x00000100 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) ? 0x00000200 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) ? 0x00000400 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_UP) ? 0x00000800 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ? 0x00001000 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ? 0x00002000 : 0;
			buttons |= SDL_GameControllerGetButton(ctx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ? 0x00004000 : 0;
			if (buttons != prev_buttons)
			{
				prev_buttons = buttons;

				uint8_t outdata[4];
				outdata[0] = '@';				// joystick button packet marker
				outdata[1] = buttons&0xFF;		// lower byte of modifiers
				outdata[2] = (buttons>>8)&0xFF;	// upper byte of modifiers
				ctx.serial->Send(outdata, 3);
			}

			// Axes
			Axis6 input;
			input.leftx = SDL_GameControllerGetAxis(ctx.gamecontroller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
			input.lefty = SDL_GameControllerGetAxis(ctx.gamecontroller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
			input.rightx = SDL_GameControllerGetAxis(ctx.gamecontroller, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
			input.righty = SDL_GameControllerGetAxis(ctx.gamecontroller, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
			input.lefttrigger = SDL_GameControllerGetAxis(ctx.gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
			input.righttrigger = SDL_GameControllerGetAxis(ctx.gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
			ClampAnalogInput(input, 0.1f, 0.9f);
			if (prev_input.leftx != input.leftx || prev_input.lefty != input.lefty || prev_input.rightx != input.rightx || prev_input.righty != input.righty || prev_input.lefttrigger != input.lefttrigger || prev_input.righttrigger != input.righttrigger)
			{
				prev_input = input;

				uint16_t lx = (uint16_t)((input.leftx * 32767.0f) + 32768);
				uint16_t ly = (uint16_t)((input.lefty * 32767.0f) + 32768);
				uint16_t rx = (uint16_t)((input.rightx * 32767.0f) + 32768);
				uint16_t ry = (uint16_t)((input.righty * 32767.0f) + 32768);
				uint8_t lt = (uint8_t)(input.lefttrigger * 255.0f);
				uint8_t rt = (uint8_t)(input.righttrigger * 255.0f);

				uint8_t outdata[16];
				outdata[0] = '%';				// joystick axis packet marker
				outdata[1] = lx&0xFF;			// lower byte of left x
				outdata[2] = (lx>>8)&0xFF;		// upper byte of left x
				outdata[3] = ly&0xFF;			// lower byte of left y
				outdata[4] = (ly>>8)&0xFF;		// upper byte of left y
				outdata[5] = rx&0xFF;			// lower byte of right x
				outdata[6] = (rx>>8)&0xFF;		// upper byte of right x
				outdata[7] = ry&0xFF;			// lower byte of right y	
				outdata[8] = (ry>>8)&0xFF;		// upper byte of right y
				outdata[9] = lt;				// left trigger
				outdata[10] = rt;				// right trigger
				ctx.serial->Send(outdata, 11);
			}
		}

		// Read key states
		SDL_Keymod modifiers = SDL_GetModState();
		if (memcmp(old_keystates, keystates, SDL_NUM_SCANCODES))
		{
			for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
			{
				if (keystates[i] != old_keystates[i])
				{
					// DEBUG: fprintf(stderr, "key %d: %d mod: %d\n", i, keystates[i], modifiers);

					uint8_t outdata[8];
					outdata[0] = '^';					// scancode packet marker
					outdata[1] = i;						// scancode
					outdata[2] = keystates[i];			// state
					outdata[3] = modifiers&0xFF;		// lower byte of modifiers
					outdata[4] = (modifiers>>8)&0xFF;	// upper byte of modifiers

					// IMPORTANT: We MUST capture ~ key since it's essential for the ESP32 to reboot the device CPUs when stuck
					// NOTE: You must hold down the ~ key for at least 250ms for the reboot to occur
					if (i==53 && keystates[i] == 1 && (modifiers & KMOD_SHIFT))
					{
						fprintf(stderr, "Keep holding down ~ to reboot...\n");
						ctx.serial->Send((uint8_t*)"~", 1);
					}
					else if (i == 0x06 && keystates[i] == 1 && (modifiers & KMOD_CTRL))
					{
						fprintf(stderr, "Attempting to quit remote process\nIf this doesn't work, hold down ~ key to reboot CPUs\n");
						ctx.serial->Send((uint8_t*)"\03", 1);
					}
					else
						ctx.serial->Send(outdata, 5);
				}
			}

			// Copy the new keystates to the old keystates
			memcpy(old_keystates, keystates, SDL_NUM_SCANCODES);
		}
	} while(s_alive);

	ctx.serial->Close();
	fprintf(stderr, "remote connection terminated\n");

	SDL_RemoveTimer(audioTimer);
	SDL_RemoveTimer(videoTimer);
	ctx.video->Terminate();
	ctx.audio->Terminate();

	SDL_FreeSurface(s_surface);
	SDL_DestroyWindow(s_window);
	SDL_Quit();

	return 0;
}
