#include "modes.h"
#include <TFT_eSPI.h> 
#include "swirl.h"

// classic conc rings

void mode_0_enter(framebuffer_t *fb) 
{
    *fb = create_framebuffer_2_color(320, 170, TFT_BLACK, TFT_WHITE);
    fb->active = true;
}
void mode_0_update(framebuffer_t *fb, uint32_t frame) 
{
    draw_concentric_circles_2_color(fb, frame, 25, 25, 1);
}
void mode_0_exit(framebuffer_t *fb) 
{
    release_framebuffer_data(fb);
    fb->active = false; 
}

// rainbow

void mode_1_enter(framebuffer_t *fb256) 
{
    *fb256 = create_framebuffer_palette_256(320, 170);
    fb256->active = true;
}
void mode_1_update(framebuffer_t *fb256, uint32_t frame) 
{
    draw_circles_rainbow(fb256, frame, 25);
}
void mode_1_exit(framebuffer_t *fb256) 
{
    release_framebuffer_data(fb256);
    fb256->active = false; 
}

// classic conc squares

void mode_2_enter(framebuffer_t *fb) 
{
    *fb = create_framebuffer_2_color(320, 170, TFT_CYAN, TFT_BLACK);
    fb->active = true;
}
void mode_2_update(framebuffer_t *fb, TFT_eSPI *tft, uint32_t frame) 
{
    draw_concentric_squares_2_color(fb, tft, frame, 10, 30, 1);
}
void mode_2_exit(framebuffer_t *fb) 
{
    release_framebuffer_data(fb);
    fb->active = false; 
}

// classic conc rings

void mode_3_enter(framebuffer_t *fb) 
{
    *fb = create_framebuffer_2_color(320, 170, TFT_BLACK, TFT_WHITE);
    fb->active = true;
}
void mode_3_update(framebuffer_t *fb, uint32_t frame) 
{
    draw_concentric_diamonds_2_color(fb, frame, 25, 25, 1);
}
void mode_3_exit(framebuffer_t *fb) 
{
    release_framebuffer_data(fb);
    fb->active = false; 
}

// curved swirl

void mode_4_enter(framebuffer_t *fb, TFT_eSPI *tft) 
{
    *fb = create_framebuffer_2_color(320, 170, TFT_BLACK, TFT_WHITE);
    fb->active = true;
    setup_swirl(fb, tft);
}
void mode_4_update(framebuffer_t *fb, uint32_t frame) {
    draw_swirl_2color(fb, frame);
}
void mode_4_exit(framebuffer_t *fb) {
    teardown_swirl();
    release_framebuffer_data(fb);
    fb->active = false;
}
