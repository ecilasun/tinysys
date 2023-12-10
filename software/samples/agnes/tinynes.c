#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include "dma.h"
#include "leds.h"
#include "task.h"

#include "agnes.h"

#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 480

static void get_input(agnes_input_t *out_input);
static void* read_file(const char *filename, size_t *out_len);
static struct EVideoContext s_vx;
static uint32_t *s_framebuffer = NULL;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s game.nes\n", argv[0]);
        return 1;
    }

    const char *ines_name = argv[1];

    size_t ines_data_size = 0;
    void* ines_data = read_file(ines_name, &ines_data_size);
    if (ines_data == NULL) {
        fprintf(stderr, "Reading %s failed.\n", ines_name);
        return 1;
    }
    
    agnes_t *agnes = agnes_make();
    if (agnes == NULL) {
        fprintf(stderr, "Making agnes failed.\n");
        return 1;
    }

    bool ok = agnes_load_ines_data(agnes, ines_data, ines_data_size);
    if (!ok) {
        fprintf(stderr, "Loading %s failed.\n", ines_name);
        return 1;
    }

	// Start video (single buffered for now)
	s_framebuffer = (uint32_t*)GPUAllocateBuffer(320 * AGNES_SCREEN_HEIGHT);
	GPUSetWriteAddress(&s_vx, (uint32_t)s_framebuffer);
	GPUSetScanoutAddress(&s_vx, (uint32_t)s_framebuffer);
	GPUSetDefaultPalette(&s_vx);

    s_vx.m_vmode = EVM_320_Wide;
    s_vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&s_vx, EVS_Enable);
	GPUClear(&s_vx, 0xFFFFFFFF);

	// Apply the NES color palette to our 12bit device
	agnes_color_t *palette = agnes_get_palette(agnes);
	for (uint32_t i=0; i<256; ++i)
		GPUSetPal(i, palette[i].r>>4, palette[i].g>>4, palette[i].b>>4);

    agnes_input_t input;

	uint32_t ledState = 0;
    while (true)
	{
		LEDSetState(ledState);

        get_input(&input);
        agnes_set_input(agnes, &input, NULL);
        agnes_next_frame(agnes);
		TaskYield();

		ledState ^= 0xFFFFFFFF;

		uint32_t source = agnes_get_raw_screen_buffer(agnes);
		for (uint32_t y = 0; y<AGNES_SCREEN_HEIGHT; ++y)
			__builtin_memcpy(s_framebuffer+80*y, (void*)(source+256*y), 256);
		CFLUSH_D_L1;
	}

    agnes_destroy(agnes);

    return 0;
}

static void get_input(agnes_input_t *out_input)
{
    memset(out_input, 0, sizeof(agnes_input_t));

	// Read input from the USB joystick data region
	int32_t *jposxy_buttons = (int32_t*)JOYSTICK_POS_AND_BUTTONS;

    if (jposxy_buttons[2]&0x20)		out_input->a = true;
    if (jposxy_buttons[2]&0x40)		out_input->b = true;
    if (jposxy_buttons[0]==0x00)	out_input->left = true;		// NOTE: 0x7F is 'centered' for direction buttons
    if (jposxy_buttons[0]==0xFF)	out_input->right = true;
    if (jposxy_buttons[1]==0x00)	out_input->up = true;
    if (jposxy_buttons[1]==0xFF)	out_input->down = true;
    if (jposxy_buttons[3]&0x10)		out_input->select = true;
    if (jposxy_buttons[3]&0x20)		out_input->start = true;
}

static void* read_file(const char *filename, size_t *out_len)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
	{
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    long pos = ftell(fp);
    if (pos < 0)
	{
        fclose(fp);
        return NULL;
    }
    size_t file_size = pos;
    rewind(fp);
    unsigned char *file_contents = (unsigned char *)malloc(file_size);
    if (!file_contents)
	{
        fclose(fp);
        return NULL;
    }
    if (fread(file_contents, file_size, 1, fp) < 1)
	{
        if (ferror(fp))
		{
            fclose(fp);
            free(file_contents);
            return NULL;
        }
    }
    fclose(fp);
    *out_len = file_size;
    return file_contents;
}
