#ifndef STNICCC_IO_H
#define STNICCC_IO_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Masks for frame flags.
 */
#define CLEAR_BIT   1
#define PALETTE_BIT 2
#define INDEXED_BIT 4

/*
 * Special polygon codes
 */
#define END_OF_FRAME  0xff
#define NEXT_BLOCK    0xfe
#define END_OF_STREAM 0xfd

/*
 * Constants for st_niccc_open()
 */
#define ST_NICCC_READ  1
#define ST_NICCC_WRITE 2

/*******************************************************************/

/*
 * Low-level IO
 */

typedef struct {
    FILE* f;
    uint32_t addr;
    uint32_t word_addr;
	uint8_t* scenedata;
    union {
        uint32_t word;
        uint8_t bytes[4];
    } u;
    int mode;
    int eof;
} ST_NICCC_IO ;

int      st_niccc_open(ST_NICCC_IO* io, const char* filename, int mode);
void     st_niccc_close(ST_NICCC_IO* io);
void     st_niccc_rewind(ST_NICCC_IO* io);
uint8_t  st_niccc_read_byte(ST_NICCC_IO* io);
uint16_t st_niccc_read_word(ST_NICCC_IO* io);
void     st_niccc_write_byte(ST_NICCC_IO* io, uint8_t b);
void     st_niccc_write_word(ST_NICCC_IO* io, uint16_t w);
void     st_niccc_next_block(ST_NICCC_IO* io);

/*******************************************************************/

/*
 * High-level IO
 */

typedef struct {
    uint8_t flags;
    uint16_t cmap_flags;
    uint8_t cmap_r[16];
    uint8_t cmap_g[16];
    uint8_t cmap_b[16];
    uint8_t nb_vertices;
    uint8_t X[256];
    uint8_t Y[256];
} ST_NICCC_FRAME;

typedef struct {
    uint8_t nb_vertices;
    uint8_t color;
    int XY[32]; // interleaved x,y
} ST_NICCC_POLYGON;

int st_niccc_read_frame(
    ST_NICCC_IO* io, ST_NICCC_FRAME* frame
);

int st_niccc_read_polygon(
    ST_NICCC_IO* io, ST_NICCC_FRAME* frame, ST_NICCC_POLYGON* polygon
);

/***********/

void st_niccc_frame_init(ST_NICCC_FRAME* frame);
void st_niccc_frame_clear(ST_NICCC_FRAME* frame);

void st_niccc_frame_set_color(
    ST_NICCC_FRAME* frame,
    uint8_t index, uint8_t r, uint8_t g, uint8_t b
);

void st_niccc_frame_set_vertex(
    ST_NICCC_FRAME* frame,
    uint8_t index, uint8_t x, uint8_t y
);

void st_niccc_write_frame_header(
    ST_NICCC_IO* io, ST_NICCC_FRAME* frame
);

void st_niccc_write_end_of_frame(ST_NICCC_IO* io);

void st_niccc_write_end_of_stream(ST_NICCC_IO* io);

void st_niccc_write_polygon_indexed(
    ST_NICCC_IO* io, 
    uint8_t color, uint8_t nb_vertices, uint8_t* vertices
);

void st_niccc_write_polygon(
    ST_NICCC_IO* io, 
    uint8_t color, uint8_t nb_vertices, uint8_t* x, uint8_t* y
);
    
void st_niccc_write_triangle_indexed(
    ST_NICCC_IO* io, 
    uint8_t color, uint8_t v1, uint8_t v2, uint8_t v3
);

void st_niccc_write_triangle(
    ST_NICCC_IO* io, 
    uint8_t color,
    uint8_t x1, uint8_t y1,
    uint8_t x2, uint8_t y2,
    uint8_t x3, uint8_t y3
);
    

/*******************************************************************/

#ifdef __cplusplus
}
#endif
    
#endif