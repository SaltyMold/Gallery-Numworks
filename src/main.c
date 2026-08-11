#include "libs/eadk.h"
#include "short.h"
#include "display.h"

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

        safe_display_image(0);
    }

    uint32_t current_index = 0;
    while (1) {
        if (eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_back)) {
            break;
        }
        if (eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_left)) {
            if (current_index > 0) {
                current_index--;
                safe_display_image(current_index);
                eadk_timing_msleep(200);
            }
        }
        if (eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_right)) {
            if (current_index < nb_total_images - 1) {
                current_index++;
                safe_display_image(current_index);
                eadk_timing_msleep(200);
            }
        }
    }

    free(images);
    return 0;
}