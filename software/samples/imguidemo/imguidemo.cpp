#include <math.h>
#include "core.h"
#include "gpu.h"
#include "xadc.h"
#include "task.h"
#include "basesystem.h"

#include "imgui/imgui.h"
#include "imgui/imgui_sw.h"
#include <stdio.h>

#define min(_x_,_y_) (_x_) < (_y_) ? (_x_) : (_y_)
#define max(_x_,_y_) (_x_) > (_y_) ? (_x_) : (_y_)

static int32_t *s_mposxy_buttons = (int32_t*)MOUSE_POS_AND_BUTTONS;

int main()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

	io.MouseDrawCursor = true;

    imgui_sw::bind_imgui_painting();
    imgui_sw::make_style_fast();

	struct EVideoContext vx;
	vx.m_vmode = EVM_640_Wide;
	vx.m_cmode = ECM_16bit_RGB;
	GPUSetVMode(&vx, EVS_Enable);

	// Set up frame buffers
	uint16_t *framebufferA = (uint16_t*)GPUAllocateBuffer(vx.m_graphicsWidth * vx.m_graphicsHeight * 2);
	uint16_t *framebufferB = (uint16_t*)GPUAllocateBuffer(vx.m_graphicsWidth * vx.m_graphicsHeight * 2);

	memset(framebufferA, 0x0, vx.m_graphicsWidth * vx.m_graphicsHeight * 2);
	memset(framebufferB, 0x0, vx.m_graphicsWidth * vx.m_graphicsHeight * 2);

	GPUSetWriteAddress(&vx, (uint32_t)framebufferA);
	GPUSetScanoutAddress(&vx, (uint32_t)framebufferB);
	GPUClearScreen(&vx, 0x03030303);

	// Set up buffer for 32 bit imgui render output
	// We try to align it to a cache boundary to support future DMA copies
	uint32_t *imguiframebuffer = (uint32_t*)GPUAllocateBuffer(vx.m_graphicsWidth * vx.m_graphicsHeight * 4);

	memset(imguiframebuffer, 0x0, vx.m_graphicsWidth * vx.m_graphicsHeight * 4);

	static float temps[] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };

	uint32_t cycle = 0;
	//uint32_t prevvblankcount = GPUReadVBlankCounter();
	do {
		// Select next r/w pair
		uint16_t *readpage = (cycle%2) ? framebufferA : framebufferB;
		uint16_t *writepage = (cycle%2) ? framebufferB : framebufferA;
		GPUSetWriteAddress(&vx, (uint32_t)writepage);
		GPUSetScanoutAddress(&vx, (uint32_t)readpage);

		//uint32_t vblankcount = GPUReadVBlankCounter();
		//if (vblankcount > prevvblankcount)
		{
			//prevvblankcount = vblankcount;

			uint32_t ADCcode = ADCGetRawTemperature();
			float temp_centigrates = (ADCcode*503.975f)/4096.f-273.15f;

			// Clear write buffer
			//GPUClearScreen((uint8_t*)imguiframebuffer, VIDEOMODE_640PALETTED, 0x0F0F0F0F);

			// Demo
			io.DisplaySize = ImVec2(vx.m_graphicsWidth, vx.m_graphicsHeight);
			io.DeltaTime = 1.0f / 60.0f;

			io.MousePos = ImVec2(s_mposxy_buttons[0], s_mposxy_buttons[1]);
			io.MouseDown[0] = s_mposxy_buttons[2]&1;
			io.MouseDown[1] = s_mposxy_buttons[2]&2;

			ImGui::NewFrame();

			//ImGui::ShowDemoWindow(NULL);

			ImGui::SetNextWindowPos(ImVec2(320,8), ImGuiCond_Always);
			//ImGui::SetNextWindowSize(ImVec2(160, 160));
			ImGui::Begin("Stats");
			ImGui::Text("Frame: %d", (int)cycle);
            ImGui::PlotLines("Temp", temps, IM_ARRAYSIZE(temps));
			ImGui::End();

			for (int i=0;i<9;++i)
				temps[i] = temps[i+1];
			temps[9] = temp_centigrates;

			ImGui::Render();
			imgui_sw::paint_imgui((uint32_t*)imguiframebuffer, vx.m_graphicsWidth, vx.m_graphicsHeight);

			// Convert to a coherent image for our 8bpp display
			// NOTE: This will not be necessary when we support RGB frame buffers
			// or one could add a GPU function to do this in hardware
			// TODO: Could this not be a DMA feature? (i.e. convert-blit?)
			for (uint32_t y=0;y<vx.m_graphicsHeight;++y)
			{
				uint32_t W = y*vx.m_graphicsWidth;
				for (uint32_t x=0;x<vx.m_graphicsWidth;++x)
				{
					uint32_t img	 = imguiframebuffer[x+W];

					// img -> a, b, g, r
					uint8_t B = ((img>>16)&0x000000FF)>>3;
					uint8_t G = ((img>>8)&0x000000FF)>>2;
					uint8_t R = ((img)&0x000000FF)>>3;

					writepage[x+W] = MAKECOLORRGB16(R,G,B);
				}
			}

			// Flush data cache at last pixel so we can see a coherent image when we scan out
			CFLUSH_D_L1;

			// Swap to next page
			++cycle;

			// Back to OS
			TaskYield();
		}
	} while (1);

    ImGui::DestroyContext(); // Won't reach here, but oh well...

	return 0;
}
