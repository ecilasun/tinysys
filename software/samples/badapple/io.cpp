#include "io.h"
#include <stdlib.h>
#include <assert.h>

#include "vpu.h"

int st_niccc_open(ST_NICCC_IO* io, const char* filename, int mode)
{
    io->f = fopen(filename, (mode == ST_NICCC_WRITE) ? "wb" : "rb");
    if(io->f == NULL)
	{
        return 0;
    }

	// Read the whole thing into memory
	{
		fpos_t pos, endpos;
		fgetpos(io->f, &pos);
		fseek(io->f, 0, SEEK_END);
		fgetpos(io->f, &endpos);
		fsetpos(io->f, &pos);
		uint32_t fsize = (uint32_t)endpos;
		io->scenedata = (uint8_t*)malloc(fsize);
		fread(io->scenedata, 1, fsize, io->f);
		fclose(io->f);
		io->f = NULL;
	}

    io->mode = mode;
    st_niccc_rewind(io);

    return 1;
}

void st_niccc_close(ST_NICCC_IO* io)
{
    //fclose(io->f);
    //io->f = NULL;
	delete [] io->scenedata;
}

void st_niccc_rewind(ST_NICCC_IO* io){
    io->addr = 0;
    io->word_addr = (uint32_t)(-1);
    io->eof = 0;
    //fseek(io->f, 0, SEEK_SET);
}

uint8_t st_niccc_read_byte(ST_NICCC_IO* io)
{
   uint8_t result;
   if(io->word_addr != io->addr >> 2) {
       io->word_addr = io->addr >> 2;
	   uint32_t* uint32dat = (uint32_t*)io->scenedata;
	   io->u.word = uint32dat[io->word_addr];
       //fseek(io->f, io->word_addr*4, SEEK_SET);
       /*size_t nb_read = fread(&(io->u.word), 4, 1, io->f);
       if(nb_read != 1) {
           // TODO:
           // Seems that sometimes I got an error, this
           // appeared when I started to merge triangles
           // into polygons (for now, ignored, so that
           // player can loop)
           //
           // fprintf(stderr,"ST_NICCC: read error\n");
           // abort();
           // exit(-1);
           return END_OF_STREAM;
       }*/
   }
   result = io->u.bytes[(io->addr)&3];
   ++(io->addr);
   return result;
}

uint16_t st_niccc_read_word(ST_NICCC_IO* io)
{
    /* In the ST-NICCC file,  
     * words are stored in big endian format.
     * (see DATA/scene_description.txt).
     */
    uint16_t hi = (uint16_t)st_niccc_read_byte(io);
    uint16_t lo = (uint16_t)st_niccc_read_byte(io);
    return (hi << 8) | lo;
}

/*void st_niccc_write_byte(ST_NICCC_IO* io, uint8_t b) {
    fwrite(&b, 1, 1, io->f);
    ++(io->addr);
}

void st_niccc_write_word(ST_NICCC_IO* io, uint16_t w) {
    uint8_t hi = (uint8_t)(w >> 8);
    uint8_t lo = (uint8_t)(w & 255);
    st_niccc_write_byte(io,hi);
    st_niccc_write_byte(io,lo);
}*/

void st_niccc_next_block(ST_NICCC_IO* io) {
    /*if(io->mode == ST_NICCC_WRITE) {
        while(io->addr & 65535) {
            st_niccc_write_byte(io,0);
        }
    } else {*/
        io->addr &= ~65535;
        io->addr +=  65536;
    //}
}

/*********************************************************************/

int st_niccc_read_frame(ST_NICCC_IO* io, ST_NICCC_FRAME* frame)
{
    if(io->eof)
        return 0;

    frame->flags = st_niccc_read_byte(io);

    // Load palette data
    if(frame->flags & PALETTE_BIT)
	{
		frame->cmap_flags = st_niccc_read_word(io);
		for(int b=15; b>=0; --b)
		{
			if(frame->cmap_flags & (1 << b))
			{
				int rgb = st_niccc_read_word(io);

				// Get the three 3-bits per component R,G,B
				int b3 = (rgb & 0x007);
				int g3 = (rgb & 0x070) >> 4;
				int r3 = (rgb & 0x700) >> 8;

				// Set the actual hardware color register
				VPUSetPal(15-b, r3*2,g3*2,b3*2);

				// Get the three 3-bits per component R,G,B
				/*int b3 = (rgb & 0x007);
				int g3 = (rgb & 0x070) >> 4;
				int r3 = (rgb & 0x700) >> 8;
						frame->cmap_r[15-b] = r3 << 5;
						frame->cmap_g[15-b] = g3 << 5;
						frame->cmap_b[15-b] = b3 << 5;*/
			}
		}
	}

    // Load vertices
    if(frame->flags & INDEXED_BIT) {
	frame->nb_vertices = st_niccc_read_byte(io);
	for(int v=0; v<frame->nb_vertices; ++v) {
	    frame->X[v] = st_niccc_read_byte(io);
	    frame->Y[v] = st_niccc_read_byte(io);
	}
    }
    return 1;
}

int st_niccc_read_polygon(ST_NICCC_IO* io, ST_NICCC_FRAME* frame, ST_NICCC_POLYGON* polygon)
{
    uint8_t poly_desc = st_niccc_read_byte(io);
    if(poly_desc == END_OF_FRAME) {
        return 0;
    }
    if(poly_desc == NEXT_BLOCK) {
        st_niccc_next_block(io);
        return 0; 
    }
    if(poly_desc == END_OF_STREAM) {
        io->eof = 1;
        return 0; 
    }
    polygon->nb_vertices = poly_desc & 15;
    polygon->color = poly_desc >> 4;
    for(int i=0; i<polygon->nb_vertices; ++i) {
        if(frame->flags & INDEXED_BIT) {
            uint8_t index = st_niccc_read_byte(io);
            polygon->XY[2*i]   = frame->X[index];
            polygon->XY[2*i+1] = frame->Y[index];
        } else {
            polygon->XY[2*i]   = st_niccc_read_byte(io);
            polygon->XY[2*i+1] = st_niccc_read_byte(io);
        }
    }
    return 1;
}

/*********************************************************************/

/*void st_niccc_frame_init(
    ST_NICCC_FRAME* frame
) {
    frame->flags = 0;
    frame->cmap_flags = 0;
    frame->nb_vertices = 0;
}

void st_niccc_frame_clear(
    ST_NICCC_FRAME* frame
) {
    frame->flags |= CLEAR_BIT;
}

void st_niccc_frame_set_color(
    ST_NICCC_FRAME* frame,
    uint8_t index, uint8_t r, uint8_t g, uint8_t b
) {
    frame->flags |= PALETTE_BIT;
    frame->cmap_r[index] = r;
    frame->cmap_g[index] = g;
    frame->cmap_b[index] = b;
    frame->cmap_flags |= (1 << (int)(15-index));
}

void st_niccc_frame_set_vertex(
    ST_NICCC_FRAME* frame,
    uint8_t index, uint8_t x, uint8_t y
) {
    frame->flags |= INDEXED_BIT;
    frame->X[index] = x;
    frame->Y[index] = y;
    if(index+1 > frame->nb_vertices) {
        frame->nb_vertices = index+1;
    }
}

void st_niccc_write_frame_header(
    ST_NICCC_IO* io, ST_NICCC_FRAME* frame
) {
    st_niccc_write_byte(io,frame->flags);

    // write palette data    
    if(frame->flags & PALETTE_BIT) {
        st_niccc_write_word(io, frame->cmap_flags);
        if(frame->flags & PALETTE_BIT) {
            for(int b=15; b>=0; --b) {
                if(frame->cmap_flags & (1 << b)) {
                    uint16_t r3 = frame->cmap_r[15-b]>>5;
                    uint16_t g3 = frame->cmap_g[15-b]>>5;
                    uint16_t b3 = frame->cmap_b[15-b]>>5;
                    uint16_t rgb = b3 | (g3 << 4) | (r3 << 8);
                    st_niccc_write_word(io,rgb);
                }
            }
        }
    }

    // write vertices
    if(frame->flags & INDEXED_BIT) {
        st_niccc_write_byte(io,frame->nb_vertices);
	for(int v=0; v<frame->nb_vertices; ++v) {
	    st_niccc_write_byte(io,frame->X[v]);
	    st_niccc_write_byte(io,frame->Y[v]);
	}
    }
}

void st_niccc_write_polygon_indexed(
    ST_NICCC_IO* io, 
    uint8_t color, uint8_t nb_vertices, uint8_t* vertices
) {
    assert(nb_vertices <= 15);
    st_niccc_write_byte(io, nb_vertices | (color << 4));
    for(int i=0; i<nb_vertices; ++i) {
        st_niccc_write_byte(io, vertices[i]);
    }
}

void st_niccc_write_polygon(
    ST_NICCC_IO* io, 
    uint8_t color, uint8_t nb_vertices,
    uint8_t* x, uint8_t* y
) {
    assert(nb_vertices <= 15);    
    st_niccc_write_byte(io, nb_vertices | (color << 4));
    for(int i=0; i<nb_vertices; ++i) {
        st_niccc_write_byte(io, x[i]);
        st_niccc_write_byte(io, y[i]);        
    }
}

void st_niccc_write_triangle_indexed(
    ST_NICCC_IO* io, 
    uint8_t color, uint8_t v1, uint8_t v2, uint8_t v3
) {
    uint8_t v[3] = { v1, v2, v3 };
    st_niccc_write_polygon_indexed(io, color, 3, v);
}

void st_niccc_write_triangle(
    ST_NICCC_IO* io, 
    uint8_t color,
    uint8_t x1, uint8_t y1,
    uint8_t x2, uint8_t y2,
    uint8_t x3, uint8_t y3
) {
    int nb_vertices = 3;
    st_niccc_write_byte(io, nb_vertices | (color << 4));
    st_niccc_write_byte(io, x1);
    st_niccc_write_byte(io, y1);
    st_niccc_write_byte(io, x2);
    st_niccc_write_byte(io, y2);
    st_niccc_write_byte(io, x3);
    st_niccc_write_byte(io, y3);
}

void st_niccc_write_end_of_frame(ST_NICCC_IO* io) {
    st_niccc_write_byte(io, END_OF_FRAME);
}

void st_niccc_write_end_of_stream(ST_NICCC_IO* io) {
    st_niccc_write_byte(io, END_OF_STREAM);
}*/