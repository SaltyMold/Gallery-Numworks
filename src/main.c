#include "libs/eadk.h"
#include "display.h"
#include "macro.h"

#include <stdint.h>
#include <stdlib.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Test";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

uint64_t external_data_size = 0;
uint32_t nb_previews = 0;
uint32_t nb_total_images = 0;
uint32_t nb_images = 0;


int main(void) {
    short_clear_screen();

    char buf[8];

    external_data_size = eadk_external_data_size;
    
    // 0-3 bytes of external data are nb of previews
    // 4-7 bytes of external data are nb of images
    nb_previews = *(uint32_t*)eadk_external_data;
    nb_total_images = *(uint32_t*)(eadk_external_data + 4);
    nb_images = nb_total_images - nb_previews;

    if (nb_total_images == 0) {
        short_display_draw_string("No image data", (eadk_point_t){0, 0});
    } else {
        images = malloc(nb_total_images * sizeof(image_t));
        if (images == NULL) {
            short_display_draw_string("Memory allocation failed", (eadk_point_t){0, 0});
            eadk_timing_msleep(1000);
            return 1;
        }

        for (uint32_t i = 0; i < nb_total_images; i++) {
            images[i].offset = *(uint32_t*)(eadk_external_data + 8 + i * 8);
            images[i].size = *(uint32_t*)(eadk_external_data + 12 + i * 8);
        }
    }

    /*-----------------------------------------------------*/

    uint32_t current_index = 0;
    bool preview_mode = true;

    bool ok_pressed = false;
    bool back_pressed = false;
    bool left_pressed = false;
    bool right_pressed = false;
    bool up_pressed = false;
    bool down_pressed = false;

    eadk_keyboard_state_t state;
    eadk_keyboard_state_t last_state;

    while (1) {
        state = eadk_keyboard_scan();

        if (eadk_keyboard_key_down(state, eadk_key_home )) break;

        if (eadk_keyboard_key_down(state, eadk_key_ok   )) ok_pressed = true;
        else ok_pressed = false;
        if (eadk_keyboard_key_down(state, eadk_key_back )) back_pressed = true;
        else back_pressed = false;
        if (eadk_keyboard_key_down(state, eadk_key_left )) left_pressed = true;
        else left_pressed = false;
        if (eadk_keyboard_key_down(state, eadk_key_right)) right_pressed = true;
        else right_pressed = false;
        if (eadk_keyboard_key_down(state, eadk_key_up   )) up_pressed = true;
        else up_pressed = false;
        if (eadk_keyboard_key_down(state, eadk_key_down )) down_pressed = true;
        else down_pressed = false;

        if (state != last_state) {
            if (preview_mode) {
                if (ok_pressed) preview_mode = false;
                if (right_pressed && current_index < nb_images - 1) current_index++;
                if (left_pressed && current_index > 0) current_index--;
                if (up_pressed) { if (current_index >= 5) current_index -= 5; else current_index = 0; }
                if (down_pressed) { if (current_index + 5 < nb_images) current_index += 5; else current_index = nb_images - 1; }
                

                uint8_t column = ((current_index % 25) % 5);
                uint8_t line = ((current_index % 25) / 5);

                safe_display_image((current_index) / 25);

                short_draw_empty_rectangle((eadk_rect_t){column * 64, line * 48, 64 - 1, 48 - 1});

                eadk_keyboard_scan();
            }
            else {
                if (back_pressed) preview_mode = true;
                safe_display_image(current_index + nb_previews);
                eadk_keyboard_scan();
            }
            FORMAT_SIZE(buf, current_index);
            short_display_draw_string(buf, (eadk_point_t){0,0});
        }
        
        last_state = state;
    }

    free(images);
    return 0;
}