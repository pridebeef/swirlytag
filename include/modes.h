#pragma once
#include <TFT_eSPI.h> 
#include <SPI.h>
#include "framebuffer.h"
#include "circles.h"
#include "swirl.h"
#include "modes.h"

// every mode needs 3 functions -- enter, update, exit

void mode_0_enter(framebuffer_t *fb, TFT_eSPI *tft);
void mode_0_update(framebuffer_t *fb, uint32_t frame);
void mode_0_exit(framebuffer_t *fb);

void mode_1_enter(framebuffer_t *fb);
void mode_1_update(framebuffer_t *fb, uint32_t frame);
void mode_1_exit(framebuffer_t *fb);

void mode_2_enter(framebuffer_t *fb256);
void mode_2_update(framebuffer_t *fb256, uint32_t frame);
void mode_2_exit(framebuffer_t *fb256);

void mode_3_enter(framebuffer_t *fb);
void mode_3_update(framebuffer_t *fb, TFT_eSPI *tft, uint32_t frame);
void mode_3_exit(framebuffer_t *fb);

void mode_4_enter(framebuffer_t *fb);
void mode_4_update(framebuffer_t *fb, uint32_t frame);
void mode_4_exit(framebuffer_t *fb);