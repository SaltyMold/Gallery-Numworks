#include "libs/eadk.h"
#include "display.h"
#include "macro.h"
#include "input.h"

#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Test";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

uint64_t external_data_size = 0;
uint32_t nb_previews = 0;
uint32_t nb_total_images = 0;
uint32_t nb_images = 0;

uint32_t current_index = 0;
bool preview_mode = true;

eadk_keyboard_state_t state;


int main(void) {
    short_clear_screen();

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

    preview_screen();
    eadk_timing_msleep(300);

    while (1) {
        state = eadk_keyboard_scan();

        if (keyboard_only_key_down(state, eadk_key_home )) break;

        if (keyboard_only_key_down(state, eadk_key_ok   )) {
            if (preview_mode){
                preview_mode = false;
                image_screen();
            }
        }
        if (keyboard_only_key_down(state, eadk_key_back )) {
            if (!preview_mode) {
                preview_mode = true;
                preview_screen();
            }
        }

        if (keyboard_only_key_down(state, eadk_key_left )) {
            if (preview_mode) {
                move_left();
                handle_held_key(eadk_key_left, move_left, 300, 100);
            }
        }
        if (keyboard_only_key_down(state, eadk_key_right)) {
            if (preview_mode) {
                move_right();
                handle_held_key(eadk_key_right, move_right, 300, 100);
            }
        }
        if (keyboard_only_key_down(state, eadk_key_up   )) {
            if (preview_mode) {
                move_up();
                handle_held_key(eadk_key_up, move_up, 300, 100);
            }

        }
        if (keyboard_only_key_down(state, eadk_key_down )) {
            if (preview_mode) {
                move_down();
                handle_held_key(eadk_key_down, move_down, 300, 100);
            }
        }

        if (keyboard_only_key_down(state, eadk_key_shift )) {
            char buf[64];
            snprintf(buf, sizeof buf, "external_data_size = %llu", (unsigned long long)external_data_size);
            short_display_draw_string(buf, (eadk_point_t){0, 0});
            snprintf(buf, sizeof buf, "nb_previews = %u", (unsigned)nb_previews);
            short_display_draw_string(buf, (eadk_point_t){0, 20});
            snprintf(buf, sizeof buf, "nb_images = %u", (unsigned)nb_images);
            short_display_draw_string(buf, (eadk_point_t){0, 40});
            snprintf(buf, sizeof buf, "nb_total_images = %u", (unsigned)nb_total_images);
            short_display_draw_string(buf, (eadk_point_t){0, 60});
            snprintf(buf, sizeof buf, "current_index = %u", (unsigned)current_index);
            short_display_draw_string(buf, (eadk_point_t){0, 80});
            snprintf(buf, sizeof buf, "preview_mode = %s", BOOL_STR(preview_mode));
            short_display_draw_string(buf, (eadk_point_t){0, 100});
        }
    }

    free(images);
    return 0;
}