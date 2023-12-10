#include "circles.h"
#include "framebuffer.h"
#include <TFT_eSPI.h> 

void draw_concentric_circles_2_color(
    framebuffer_t *fb,
    uint32_t frame,
    uint8_t width, 
    uint8_t spacing,
    uint8_t speed
) 
{
    uint16_t longest_side = max(fb->width, fb->height);
    uint8_t color = 0;
    int16_t radius = ((longest_side / (width + spacing)) / 2 + 1) * (width + spacing);
    while (radius >= 0 - width) {
        uint16_t frame_adjustment = ((frame * speed) % (width + spacing));
        draw_filled_circle(
            fb, 
            fb->width / 2, 
            fb->height / 2, 
            max(radius + frame_adjustment, 0), 
            color
        );
        radius -= color ? spacing : width; 
        color ^= 1;
    }
}

void draw_circles_rainbow(
  framebuffer_t *fb,
  uint32_t frame,
  uint8_t width
)
{
    uint8_t palette_size = 7;
    uint8_t palette[] = {
        rgb_to_palette256(255, 0, 0), 
        rgb_to_palette256(255, 165, 0), 
        rgb_to_palette256(255, 255, 0), 
        rgb_to_palette256(0, 255, 0), 
        rgb_to_palette256(0, 0, 255), 
        rgb_to_palette256(0x4B, 0, 0x82), 
        rgb_to_palette256(0xEE, 0x82, 0xEE)
    };
    draw_circles_p256(fb, frame, width, width, palette, palette_size, 1);
}

void draw_circles_p256(
  framebuffer_t *fb,
  uint32_t frame,
  uint8_t width,
  uint8_t spacing,
  uint8_t palette[],
  uint8_t palette_size,
  uint8_t speed
) 
{
    uint16_t longest_side = max(fb->width, fb->height);
    uint64_t drawn = 0;
    int16_t radius = ((longest_side / (width + spacing)) / 2 + 1) * (width + spacing);
    
    while (radius >= 0 - width) {
        uint16_t frame_adjustment = ((frame * speed) % (width + spacing));
        draw_filled_circle(
            fb, 
            fb->width / 2, 
            fb->height / 2, 
            max(radius + frame_adjustment, 0), 
            palette[(drawn + 2*(frame / (width + spacing))) % palette_size]
        );
        radius -= (drawn % 2 == 0) ? spacing : width; 
        drawn = drawn + 1;
    }
}

// this is just significantly faster using raw TFT calls 
void draw_concentric_squares_2_color(
  framebuffer_t *fb, 
  TFT_eSPI *tft,
  uint32_t frame,
  uint8_t width, 
  uint8_t spacing,
  uint8_t speed
) 
{
    uint16_t longest_side = max(fb->width, fb->height);
    uint8_t color = 0;
    int16_t radius = ((longest_side / (width + spacing)) / 2 + 1) * (width + spacing);
    uint16_t cx = fb->width / 2;
    uint16_t cy = fb->height / 2;
    while (radius >= 0 - width) {
        uint16_t frame_adjustment = ((frame * speed) % (width + spacing));
        uint16_t tft_col = color ? fb->palette[1] : fb->palette[0];
        for (int i = 0; i < width; ++i) {
            tft->drawFastHLine(-radius + (i - frame_adjustment) + cx, 
                               -radius + (i - frame_adjustment) + cy , 
                               2 * radius - 2 * (i - frame_adjustment), 
                               tft_col);
            tft->drawFastHLine(-radius + (i - frame_adjustment) + cx, 
                               radius - (i - frame_adjustment) + cy, 
                               2 * radius - 2 * (i - frame_adjustment), 
                               tft_col);
            tft->drawFastVLine(-radius + (i - frame_adjustment) + cx, 
                               -radius + (i - frame_adjustment) + cy, 
                               2 * radius - 2 * (i - frame_adjustment), 
                               tft_col);
            tft->drawFastVLine(radius - (i - frame_adjustment) + cx, 
                               -radius + (i - frame_adjustment) + cy, 
                               2 * radius - 2 * (i - frame_adjustment) + 1, // + 1 looks odd; it fixes a bug
                               tft_col);
        }
        radius -= color ? spacing : width; 
        color ^= 1;
    }
}

void draw_concentric_diamonds_2_color(
    framebuffer_t *fb,
    uint32_t frame,
    uint8_t width, 
    uint8_t spacing,
    uint8_t speed
) 
{
    uint16_t longest_side = max(fb->width, fb->height);
    uint8_t color = 0;
    int16_t radius = ((longest_side / (width + spacing)) / 2 + 2) * (width + spacing);
    while (radius >= 0 - width) {
        uint16_t frame_adjustment = ((frame * speed) % (width + spacing));
        draw_filled_diamond(
            fb, 
            fb->width / 2, 
            fb->height / 2, 
            max(radius + frame_adjustment, 0), 
            color
        );
        radius -= color ? spacing : width; 
        color ^= 1;
    }
}