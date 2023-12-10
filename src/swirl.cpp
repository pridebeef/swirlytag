#include "swirl.h"
#include <TFT_eSPI.h> 
#include <FastTrig.h>

// static storage:
// every frame is 6800 bytes, we have 327680 (minus a lot)
// 45 is basically the theoretical max, no 60 for us

static float *uv_coords_x;
static float *uv_coords_y;
static uint8_t **prerendered_frames;

// drawing params

// higher arms -> more dense, hides screen tearing -- more frames -> smaller arm# and still smooth
static constexpr uint8_t arms = 6;
static constexpr float arm_angle = 360.0 / arms;
static constexpr float color_angle = arm_angle / 2.0;
static constexpr uint8_t prerender_framecount = 45;
static constexpr double prerender_deltatime = (double)1.0 / prerender_framecount;

// arm curl functions
#define F(x) (log(x))
//#define F(x) ((0.5) * x)


void fill_uv_coords_table(uint16_t width, uint16_t height)
{
    uv_coords_x = (float*)malloc(sizeof(float) * width);
    uv_coords_y = (float*)malloc(sizeof(float) * height);
    float min_axis = min(width, height);
    for (int x = 0; x < width; ++x) {
      uv_coords_x[x] = (2.0 * (float)x - (float)width) / min_axis;
    }
    for (int y = 0; y < height; ++y) {
      uv_coords_y[y] = (2.0 * (float)y - (float)height) / min_axis;
    }
}

void setup_swirl(framebuffer_t *fb)
{
    setup_swirl(fb, nullptr);
}

void setup_swirl(framebuffer_t *fb, TFT_eSPI *tft) 
{
    fill_uv_coords_table(fb->width, fb->height);
    prerendered_frames = (uint8_t**)malloc(sizeof(uint8_t*) * prerender_framecount);
    if (tft != nullptr) {
        tft->fillScreen(TFT_BLACK);
        tft->setTextColor(TFT_WHITE);
        tft->drawString("loading...", 0, 0, 4);
    }
    for (int i = 0; i < prerender_framecount; ++i) {
        prerendered_frames[i] = (uint8_t*)malloc(FB_BIT_ARRAY_SIZE(fb->width, fb->height));
        // frame addition hack: not sure why this works/matters! 
        prerender_swirl_2color_frame(i + 10000, prerendered_frames[i], fb->width, fb->height);
        if (tft != nullptr) {
            tft->fillRect(0, 40, 100, 80, TFT_BLACK);
            String s = String(i) + String("/") + String(prerender_framecount);
            tft->drawString(s, 0, 40, 4);
        }
    }
    free(uv_coords_x);
    free(uv_coords_y);
}

void teardown_swirl() 
{
    for (int i = 0; i < prerender_framecount; ++i) {
        free(prerendered_frames[i]);
    }
    free(prerendered_frames);
}

void prerender_swirl_2color_frame(uint32_t frame, uint8_t *dst, uint16_t width, uint16_t height) 
{
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            float pos_x = uv_coords_x[x];
            float pos_y = uv_coords_y[y];
            float angle = 0.0;
            float radius = hypot(pos_x, pos_y);
            angle = RAD_TO_DEG * (atan2(pos_y, pos_x));
            float amod = fmod(angle + arm_angle * (frame * prerender_deltatime) - 120.0 * F(radius), arm_angle);
            uint32_t idx = get_bit_array_index(x, y, width);
            uint8_t bit = get_bit_array_bit(x, y, width);
            dst[idx] = (dst[idx] & ~(1u << bit)) | (((amod < color_angle) ? 1u : 0u) << bit);
        }
    }
}

void draw_swirl_2color(framebuffer_t *fb, uint32_t frame) {
    memcpy(fb->data, 
           prerendered_frames[frame % prerender_framecount], 
           FB_BIT_ARRAY_SIZE(fb->width, fb->height));
    fb->data[frame % prerender_framecount] = 0;
    memset(fb->modified, 255, FB_BIT_ARRAY_SIZE(fb->width, fb->height));
}