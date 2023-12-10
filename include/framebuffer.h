#pragma once
#include "Arduino.h"

#define FB_BIT_ARRAY_SIZE(width, height) (((width * height) / 8) + 1)
#define FB_PALETTE_256_SIZE(width, height) (width * height)

enum framebuffer_color_t {
    // color: least significant bit 1/0 
    // data:  8 pixels, left to right mapping from lsb to msb 
    //       
    BLACK_AND_WHITE,  //.......W         -- 1 bit, on off
    PALETTE_256,      //RRRGGGBB         -- 3:3:2    256-color
    FULL_COLOR        //RRRRRGGGGGGBBBBB -- 5:6:5     
};

struct framebuffer_t {
    framebuffer_color_t colors;
    union {
        uint8_t *data;
        uint16_t *fullcolor_data;
    };
    union {
        uint8_t *previous;
        uint16_t *fullcolor_previous;
    };
    uint8_t *modified;
    uint16_t width; 
    uint16_t height;
    uint16_t *palette; 
    bool active;
};

struct TFT_eSPI;

// framebuffer initialization functions 

// create new framebuffer of specified color set with width and height
framebuffer_t create_framebuffer(framebuffer_color_t color, uint16_t w, uint16_t h);
framebuffer_t create_framebuffer_2_color(uint16_t w, uint16_t h, uint16_t c1, uint16_t c2);
inline framebuffer_t create_framebuffer_palette_256(uint16_t w, uint16_t h) {
    return create_framebuffer(framebuffer_color_t::PALETTE_256, w, h);
}
void release_framebuffer_data(framebuffer_t *fb);

// inlined utilities for data


// convert screen x, y coords to bit array index 
// `data[get_bit_array_index(...)] & get_bit_array_mask(...)`
inline uint32_t get_bit_array_index(uint16_t x, uint16_t y, uint16_t width) 
{
    return ((x + ((y * width) % 8u)) / 8u) + ((y * width) / 8u);
}

// convert screen x, y coords to bit array bit-within-index
inline uint8_t get_bit_array_bit(uint16_t x, uint16_t y, uint16_t width) 
{
    return (x + ((y * width) % 8u)) % 8u;
}

// convert screen x, y coords to bit array mask for access 
// `data[get_bit_array_index(...)] & get_bit_array_mask(...)`
inline uint8_t get_bit_array_mask(uint16_t x, uint16_t y, uint16_t width) 
{
    return 1u << get_bit_array_bit(x, y, width);
}

// convert bit array specific bit (position/mask) to screen x coordinate
inline uint16_t get_screen_x_from_bit_array(uint32_t index, uint8_t bit, uint16_t width) 
{
    return (bit + (index * 8u) % width) % width;
}

// convert bit array specific bit (position/mask) to screen y coordinate
inline uint16_t get_screen_y_from_bit_array(uint32_t index, uint8_t bit, uint16_t width) 
{
    return ((index * 8u) / width) + (bit + (index * 8u) % width) / width;
}


// drawing utilities 


// returns the color of a single pixel in framebuffer data
uint16_t get_pixel(framebuffer_t *fb, uint16_t x, uint16_t y);
// assign color to framebuffer pixel
void set_pixel(framebuffer_t *fb, uint16_t x, uint16_t y, uint16_t color);

// draw the changes from `previous` to `data` to tft screen
void draw_frame(framebuffer_t *fb, TFT_eSPI* tft);
void draw_frame_black_and_white(framebuffer_t *fb, TFT_eSPI* tft);
void draw_frame_palette_256(framebuffer_t *fb, TFT_eSPI* tft);

// fill screen with given color 
void clear_screen(framebuffer_t *fb, uint32_t color); 

void draw_horizontal_line(framebuffer_t *fb, int32_t, int32_t, int32_t, uint32_t);
void draw_filled_circle(framebuffer_t *fb, int32_t, int32_t, int32_t, uint32_t);
void draw_filled_diamond(framebuffer_t *fb, int32_t, int32_t, int32_t, uint32_t);

// turn RGB 8:8:8 color -> "close" approximation of 256-color palette
inline uint8_t rgb_to_palette256(uint16_t r, uint16_t g, uint16_t b)
{
    return (((r + 0u) / 32u) << 5u) + (((g + 0u) / 32u) << 2u) + (((b + 0u) / 64u));
}

// change 256-color palette to TFT 5:6:5 color representation.
// use this mostly for caching, as it expands pretty ugly 
// and more than a couple instructions.
inline uint16_t palette256_to_565(uint8_t color) {
    #define R_3_BIT(c)     ((c & (7u << 5u)) >> 5u)
    #define G_3_BIT(c)     ((c & (7u << 2u)) >> 2u)
    #define B_2_BIT(c)     (c & 3u)
    #define CONV_3_TO_5(a) (a * 4u + a / 2u)
    #define CONV_3_TO_6(a) (a * 8u + a)
    #define CONV_2_TO_5(a) (a * 8u + a * 2u + 1u)
    #define CONV_ALL(c) (                    \
        (CONV_3_TO_5(R_3_BIT(c)) << 11u))  |  \
        (CONV_3_TO_6(G_3_BIT(c)) << 5u)    |  \
        CONV_2_TO_5(B_2_BIT(c))
    
    return CONV_ALL(color);

    #undef R_3_BIT
    #undef G_3_BIT
    #undef B_2_BIT
    #undef CONV_3_TO_5
    #undef CONV_3_TO_6
    #undef CONV_2_TO_5
    #undef CONV_ALL   
}
