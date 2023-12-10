#pragma once 
#include "framebuffer.h"

void draw_swirl_2color(framebuffer_t *fb, uint32_t frame);
void setup_swirl(framebuffer_t *fb);
void setup_swirl(framebuffer_t *fb, TFT_eSPI *tft);
void teardown_swirl();
void prerender_swirl_2color_frame(uint32_t frame, uint8_t *dst, uint16_t width, uint16_t height);