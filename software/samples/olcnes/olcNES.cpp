#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <list>

#include "core.h"
#include "gpu.h"

#include "olcnes.h"
#include "Bus.h"
#include "Cartridge.h"

static struct EVideoContext s_vx;
static uint8_t *framebuffer = nullptr;

void DrawScreen(olc::Sprite* sprite)
{
	if (sprite == nullptr)
		return;

	for (uint32_t y = 0; y<240; ++y)
	{
		for (uint32_t x = 0; x<256; ++x)
		{
			uint16_t *buffer = (uint16_t*)(framebuffer + (x+y*320)*2);
			*buffer = sprite->data[x+y*(sprite->width)];
		}
	}
}

int main(int argc, char *argv[])
{
	printf("olcNES - OLC's NES emulator port for RISC-V\n");

	if (argc<=1)
	{
		printf("Usage: oldNES [cartridge.nes]\n");
		return 0;
	}

	// TODO: Load given cartridge and run
	printf("Loading %s\n", argv[1]);

	Bus nes;
	std::shared_ptr<Cartridge> cart;

	cart = std::make_shared<Cartridge>(argv[1]);
	if (!cart->ImageValid())
	{
		printf("Failed to load\n");
		return false;
	}

	/*std::list<uint16_t> audio[4];
	for (int i = 0; i < 4; i++)
	{			
		for (int j = 0; j < 120; j++)
			audio[i].push_back(0);
	}*/

	printf("Starting...\n");
	nes.insertCartridge(cart);
	nes.reset();
	nes.SetSampleFrequency(44100);
	// TODO: Initialize APU here

	// Single buffered for now
	framebuffer = GPUAllocateBuffer(2*320*240);
	GPUSetWriteAddress(&s_vx, (uint32_t)framebuffer);
	GPUSetScanoutAddress(&s_vx, (uint32_t)framebuffer);
	GPUSetDefaultPalette(&s_vx);

    s_vx.m_vmode = EVM_320_Wide;
    s_vx.m_cmode = ECM_16bit_RGB;
	GPUSetVMode(&s_vx, EVS_Enable);
	GPUClearScreen(&s_vx, 0x0A0A0A0A);

	//float fAccumulatedTime = 0.f;
	int F = 0;
	while(1)
	{
		printf("frame %d\n", F++);
		do { nes.clock(); } while (!nes.ppu.frame_complete);
		nes.ppu.frame_complete = false;

		/*float fElapsedTime = 1.f/60.f;
		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= 1.0f / 60.0f)
		{
			fAccumulatedTime -= (1.0f / 60.0f);
			audio[0].pop_front();
			audio[0].push_back(nes.apu.pulse1_visual);
			audio[1].pop_front();
			audio[1].push_back(nes.apu.pulse2_visual);
			audio[2].pop_front();
			audio[2].push_back(nes.apu.noise_visual);
		}*/

		// Handle input for controller in port #1
		nes.controller[0] = 0x00;
		/*nes.controller[0] |= GetKey(olc::Key::X).bHeld ? 0x80 : 0x00;     // A Button
		nes.controller[0] |= GetKey(olc::Key::Z).bHeld ? 0x40 : 0x00;     // B Button
		nes.controller[0] |= GetKey(olc::Key::A).bHeld ? 0x20 : 0x00;     // Select
		nes.controller[0] |= GetKey(olc::Key::S).bHeld ? 0x10 : 0x00;     // Start
		nes.controller[0] |= GetKey(olc::Key::UP).bHeld ? 0x08 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::DOWN).bHeld ? 0x04 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::LEFT).bHeld ? 0x02 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::RIGHT).bHeld ? 0x01 : 0x00;*/

		//if (GetKey(olc::Key::R).bPressed) nes.reset();
		//if (GetKey(olc::Key::P).bPressed) (++nSelectedPalette) &= 0x07;

		DrawScreen(&nes.ppu.GetScreen());
		CFLUSH_D_L1;

		//GPUWaitVSync();
	}

	return 0;
}
