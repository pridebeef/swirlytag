#include "framebuffer.h"
#include "Arduino.h"
#include <TFT_eSPI.h> 

#ifdef debug
    #define debug_write(...) Serial.printf(__VA_ARGS__)
#else 
    #define debug_write(...) do { } while (0)
#endif

framebuffer_t create_framebuffer(
    framebuffer_color_t c, 
    uint16_t width, 
    uint16_t height
) 
{
    framebuffer_t fb;
    fb.colors = c;
    fb.modified = (uint8_t*)calloc(((width * height) / 8) + 1, sizeof(uint8_t));
    fb.width = width;
    fb.height = height;
    fb.active = false;

    switch (c)
    {
    case framebuffer_color_t::BLACK_AND_WHITE:
        fb.data     = (uint8_t*)calloc(FB_BIT_ARRAY_SIZE(width, height), 1U);
        fb.previous = (uint8_t*)calloc(FB_BIT_ARRAY_SIZE(width, height), 1U);
        fb.palette = (uint16_t*)calloc(2, sizeof(uint16_t));
        break;
    case framebuffer_color_t::PALETTE_256:
        fb.data     = (uint8_t*)calloc(FB_PALETTE_256_SIZE(width, height), 1U);
        fb.previous = (uint8_t*)calloc(FB_PALETTE_256_SIZE(width, height), 1U);
        fb.palette = (uint16_t*)calloc(256, sizeof(uint16_t));
        for (int i = 0; i < 256; ++i) {
            fb.palette[i] = palette256_to_565(i);
        }
        break;
    }    
    debug_write("created framebuffer:\r\n");
    debug_write("created framebuffer:\tw: %i; h: %i\r\n", fb.width, fb.height);
    return fb;
}

framebuffer_t create_framebuffer_2_color(
    uint16_t w, 
    uint16_t h, 
    uint16_t c1, 
    uint16_t c2
) 
{
    framebuffer_t fb = create_framebuffer(framebuffer_color_t::BLACK_AND_WHITE, w, h);
    fb.palette[0] = c1;
    fb.palette[1] = c2;
    return fb;
}

uint16_t get_pixel(framebuffer_t *fb, uint16_t x, uint16_t y)
{
    if (x < 0 || x > fb->width || y < 0 || y > fb->height) {
        return 0; 
    }
    switch (fb->colors)
    {
    case framebuffer_color_t::BLACK_AND_WHITE:
        return fb->data[get_bit_array_index(x, y, fb->width)] & get_bit_array_mask(x, y, fb->width);
        break;

    case framebuffer_color_t::PALETTE_256:
        return fb->data[y * fb->width + x];
        break;
    }
    return 0;
}

void release_framebuffer_data(framebuffer_t *fb) 
{
    free(fb->modified);
    switch (fb->colors)
    {
    case framebuffer_color_t::BLACK_AND_WHITE:
    case framebuffer_color_t::PALETTE_256:
        free(fb->data);
        free(fb->previous);
        free(fb->palette);
        break;
    case framebuffer_color_t::FULL_COLOR:
        free(fb->fullcolor_data);
        free(fb->fullcolor_previous);
        break;
    }
}

void set_pixel(
    framebuffer_t *fb, 
    uint16_t x, 
    uint16_t y, 
    uint16_t color 
) 
{
    if (x < 0 || x > fb->width || y < 0 || y > fb->height) {
        return; 
    }
    switch (fb->colors)
    {
    case framebuffer_color_t::BLACK_AND_WHITE: 
    {
        uint32_t index = get_bit_array_index(x, y, fb->width);
        uint8_t mask = get_bit_array_mask(x, y, fb->width);
        // cheap to check here and can potentially avoid a modified iteration later 
        if (!!(fb->data[index] & mask) != !!color) {
            fb->modified[index] |= mask;
            fb->data[index] ^= mask;
        }
        break;
    }
    case framebuffer_color_t::PALETTE_256:
        fb->data[y * fb->width + x] = color;
        break;
    }
}

// note
// only the body that sets color inside of the innermost loop changes between
// color specializations. the reason we don't dispatch a switch inside of that
// is that it significantly slowed down the loop even with -O2 -- the hit to 
// readibility is worth the speedup. 
void draw_frame(framebuffer_t *fb, TFT_eSPI* tft)
{
    if (fb->colors == framebuffer_color_t::BLACK_AND_WHITE) {
        draw_frame_black_and_white(fb, tft); 
        return;
    } else if (fb->colors == framebuffer_color_t::PALETTE_256) {
        draw_frame_palette_256(fb, tft); 
        return;
    }
}

void draw_frame_black_and_white(framebuffer_t *fb, TFT_eSPI *tft) 
{
    uint16_t color = 0, previous_color = 0, desired_color = 0;
    for (uint32_t i = 0; i < (fb->width * fb->height) / 8; ++i) {
        if (fb->modified[i] == 0) {
            continue;
        }
        for (uint8_t j = 0; j < 8; ++j) {
            uint8_t mask = (1 << j);
            if ((fb->modified[i] & mask) != 0) {
                uint16_t screen_x = get_screen_x_from_bit_array(i, j, fb->width);
                uint16_t screen_y = get_screen_y_from_bit_array(i, j, fb->width);
                color = fb->data[i] & mask;
                previous_color = fb->previous[i] & mask;
                desired_color = fb->palette[!!color]; 
                if (color != previous_color) {
                    tft->drawPixel(screen_x, screen_y, desired_color);
                }
            }

        }
    }
    memcpy(fb->previous, fb->data, FB_BIT_ARRAY_SIZE(fb->width, fb->height));
    memset(fb->modified, 0, FB_BIT_ARRAY_SIZE(fb->width, fb->height));
}

void draw_frame_palette_256(framebuffer_t *fb, TFT_eSPI *tft) 
{
    uint16_t color = 0, previous_color = 0, desired_color = 0;
    for (uint32_t i = 0; i < (fb->width * fb->height) / 8; ++i) {
        if (fb->modified[i] == 0) {
            continue;
        }
        for (uint8_t j = 0; j < 8; ++j) {
            uint8_t mask = (1 << j);
            if ((fb->modified[i] & mask) != 0) {
                uint16_t screen_x = get_screen_x_from_bit_array(i, j, fb->width);
                uint16_t screen_y = get_screen_y_from_bit_array(i, j, fb->width);

                color = fb->data[screen_x + (screen_y * fb->width)];
                previous_color = fb->previous[screen_x + (screen_y * fb->width)];
                desired_color = fb->palette[color];
                fb->previous[screen_x + (screen_y * fb->width)] = color;

                if (color != previous_color) {
                    tft->drawPixel(screen_x, screen_y, desired_color);
                }
            }
        }
    }
    memset(fb->modified, 0, FB_BIT_ARRAY_SIZE(fb->width, fb->height));
}

void clear_screen(framebuffer_t *fb, uint32_t color)
{
    if (fb->colors == framebuffer_color_t::BLACK_AND_WHITE)
        color = !!color;
    for (int x = 0; x < fb->width; ++x) {
        for (int y = 0; y < fb->height; ++y) {
            if (get_pixel(fb, x, y) != color) {
                set_pixel(fb, x, y, color);
            }
        }
    }
} 

void draw_horizontal_line(
    framebuffer_t *fb, 
    int32_t x0, 
    int32_t x1, 
    int32_t y, 
    uint32_t color
) 
{
    if (x0 < 0) {
        x0 = 0;
    }
    if (x1 > fb->width) {
        x1 = fb->width;
    }
    if (y < 0 || y >= fb->height) {
        return; 
    }

    // long line memset optimization - we can color 8px at a time by 0xFF or 0x00ing the bytes
    if (fb->colors == framebuffer_color_t::BLACK_AND_WHITE) {
        // roughly 9x speedup for memsetting straight lines all at once
        //
        // to speed this up we need width > (rem bits + 8)
        // in order to fill the next index entirely. 
        // anything else should just pass through. 
        uint16_t i0 = get_bit_array_index(x0, y, fb->width);
        uint16_t b0 = get_bit_array_bit(x0, y, fb->width);
        uint16_t i1 = get_bit_array_index(x1, y, fb->width);
        uint16_t b1 = get_bit_array_bit(x1, y, fb->width);
        uint8_t color_byte = !!(color) ? 0xFF : 0x00;
        if (b0 > 0) {
            uint8_t mask = (~0u << b0);
            if (color) {
                fb->data[i0] |= mask;
            } else {
                fb->data[i0] &= ~mask;
            }
            fb->modified[i0] |= mask;
            i0 += 1;
        }
        if (i0 < i1) {
            memset(&fb->data[i0], color_byte, i1 - i0);
            memset(&fb->modified[i0], 0xFF, i1 - i0);
        }
        if (b1 > 0) {
            if (color) {
                fb->data[i1] |= ~(~0u << b1);
            } else {
                fb->data[i1] &= (~0u << b1);
            }
            fb->modified[i1] |= ~(~0u << b1);
        }
            return;
    }

    if (fb->colors == framebuffer_color_t::PALETTE_256) {
        memset(&fb->data[y * fb->width + x0], (uint8_t)color, x1 - x0);
        // set modified, just refactor it later...
        uint16_t i0 = get_bit_array_index(x0, y, fb->width);
        uint16_t b0 = get_bit_array_bit(x0, y, fb->width);
        uint16_t i1 = get_bit_array_index(x1, y, fb->width);
        uint16_t b1 = get_bit_array_bit(x1, y, fb->width);
        if (b0 > 0) {
            uint8_t mask = (~0u << b0);
            fb->modified[i0] |= mask; 
            i0 += 1;
        }
        if (i0 < i1)
            memset(&fb->modified[i0], 0xFF, i1 - i0);
        if (b1 > 0) {
            fb->modified[i1] |= ~(~0u << b1);
        }
        return;
    }

    for (int32_t i = x0; i <= x1; ++i) {
        set_pixel(fb, i, y, color);
    }
}

void draw_filled_circle(
    framebuffer_t *fb, 
    int32_t x0, 
    int32_t y0, 
    int32_t r, 
    uint32_t color
)
{
    // adapted from library (bodmer/TFT_eSPI)
    // modified for different framebuffer line drawing 
    if (r <= 0) return;
    int32_t x  = 0;
    int32_t dx = 1;
    int32_t dy = r + r;
    int32_t p  = -(r >> 1);
    // x y w color
    draw_horizontal_line(fb, x0 - r, x0 - r + dy+1, y0, color);

    while (x < r){
        if (p >= 0) {
            draw_horizontal_line(fb, x0 - x, x0 - x + dx, y0 + r, color);
            draw_horizontal_line(fb, x0 - x, x0 - x + dx, y0 - r, color);
            dy -= 2;
            p -= dy;
            r--;
        }
        dx += 2;
        p += dx;
        x++;
        draw_horizontal_line(fb, x0 - r, x0 - r + dy+1, y0 + x, color);
        draw_horizontal_line(fb, x0 - r, x0 - r + dy+1, y0 - x, color);
    }
}

void draw_filled_diamond(
    framebuffer_t *fb, 
    int32_t x0, 
    int32_t y0, 
    int32_t r, 
    uint32_t color 
)
{
    if (r <= 0) return;
    draw_horizontal_line(fb, x0 - r, x0 + r, y0, color);
    for (int i = 1; i < r; ++i) {
        draw_horizontal_line(fb, x0 - r + i, x0 + r - i, y0 + i, color);
        draw_horizontal_line(fb, x0 - r + i, x0 + r - i, y0 - i, color);
    }
}