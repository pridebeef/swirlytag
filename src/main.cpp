#include <TFT_eSPI.h> 
#include <SPI.h>
#include "framebuffer.h"
#include "circles.h"
#include "swirl.h"
#include "modes.h"

//#define debug 

//#define demo_mode
#ifdef demo_mode
const static uint64_t frames[] = {};
#endif

TFT_eSPI tft = TFT_eSPI();  

framebuffer_t fb;
framebuffer_t fb256;

const uint8_t NUM_MODES = 5;
static uint8_t mode = 0;
static uint8_t last_mode = 255;
static uint8_t button_reset_trigger = 0;

static uint32_t prev_time = 0;

// general purpose button for mode selection
const uint8_t BUTTON_PIN = 14;

void setup(void)
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT);
    // battery and backlight related. no clue what these do on board 
    pinMode(15, OUTPUT);
    pinMode(38, OUTPUT);
    digitalWrite(15, HIGH); 
    ledcSetup(0, 10000, 8);
    ledcAttachPin(38, 0);
    ledcWrite(0, 255); // 0..255 

    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_BLACK);
    tft.setRotation(3);

    prev_time = millis();

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0);
}

void loop()
{
  static uint32_t frame = 0;
  while (1) {
    #ifdef debug
        uint32_t ts = millis();
    #endif

    // vtable-based state machine was too tricky for args changing between modes and i'm lazy 
    if (mode != last_mode) {
        switch (mode) {
            case 0: mode_0_enter(&fb, &tft); break;
            case 1: mode_1_enter(&fb); break;
            case 2: mode_2_enter(&fb); break;
            case 3: mode_3_enter(&fb); break;
            case 4: mode_4_enter(&fb); break;
        }
    }

    switch(mode) {
        case 0: mode_0_update(&fb, frame); break;
        case 1: mode_1_update(&fb, frame); break;
        case 2: mode_2_update(&fb, frame); break;
        case 3: mode_3_update(&fb, &tft, frame); break;
        case 4: mode_4_update(&fb, frame); break;
    }
    
    last_mode = mode;
    if (digitalRead(BUTTON_PIN) == 0 ) {
        button_reset_trigger = 1;
    }
    if (digitalRead(BUTTON_PIN) == 1 && button_reset_trigger) {
        mode += 1;
        if (mode == NUM_MODES) {
            tft.setTextColor(TFT_WHITE);
            tft.fillScreen(TFT_BLACK);
            tft.drawString("going to sleep", 0, 0, 4);
            tft.drawString("press again to wake", 0, 40, 4);
            delay(2500);
            esp_deep_sleep_start();
        }
        mode = mode % NUM_MODES; 
        button_reset_trigger = 0;
    }

    #ifdef demo_mode
    for (int i = 0; i <= NUM_MODES; ++i) {
        if (frame == frames[i]) mode = (mode + 1) % NUM_MODES;
    }
    #endif 

    #ifdef debug
        uint32_t midpoint = millis() - ts;
    #endif

    if (fb.active) draw_frame(&fb, &tft);
    if (fb256.active) draw_frame(&fb256, &tft);

    #ifdef debug
        // print averaged fps over last 60 frames with delineation between update/draw loops
        if (frame % 60 == 0) {
            uint32_t ntime = millis();
            uint16_t delta = ntime - prev_time;
            Serial.printf(
                "average fps: %f\r\n  this frame (%i):\r\n\tupdate ms: %i\r\n\tdraw ms: %i\r\n", 
                1000.0 / (delta / 60.0),
                frame, 
                midpoint, 
                (ntime - ts) - midpoint
            );
            prev_time = millis();
        } 
    #endif

    if (mode != last_mode) {
        switch(last_mode) {
            case 0: mode_0_exit(&fb); break;
            case 1: mode_1_exit(&fb); break;
            case 2: mode_2_exit(&fb); break;
            case 3: mode_3_exit(&fb); break;
            case 4: mode_4_exit(&fb); break;
        }
    }

    frame = frame + 1;
  }
}

