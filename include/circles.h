#pragma once
#include "framebuffer.h"

unsigned int rainbow(byte);

void draw_concentric_circles_2_color(
    framebuffer_t *fb,
    uint32_t frame,
    uint8_t width, 
    uint8_t spacing,
    uint8_t speed
);

void draw_circles_rainbow(
    framebuffer_t *fb,
    uint32_t frame,
    uint8_t width
);

void draw_circles_p256(
    framebuffer_t *fb,
    uint32_t frame,
    uint8_t width,
    uint8_t spacing,
    uint8_t palette[],
    uint8_t palette_size,
    uint8_t speed
);

void draw_concentric_squares_2_color(
    framebuffer_t *fb, 
    TFT_eSPI *tft,
    uint32_t frame,
    uint8_t width, 
    uint8_t spacing,
    uint8_t speed
);

void draw_concentric_diamonds_2_color(
    framebuffer_t *fb,
    uint32_t frame,
    uint8_t width, 
    uint8_t spacing,
    uint8_t speed
);