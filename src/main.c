#include "libs/eadk.h"
// #include "libs/storage.h"
#include "short.h"
#include "macro.h"

#include <stdint.h>
#include <stdlib.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Test";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

typedef struct {
    uint32_t offset;
    uint32_t size;
} image_t;

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

    // Allocate array of images dynamically
    image_t *images = (image_t *)malloc(nb_total_images * sizeof(image_t));
    if (images == NULL) {
        short_display_draw_string("Memory allocation failed", (eadk_point_t){0, 0});
        return 1;
    }
    
    for (uint32_t i = 0; i < nb_total_images; i++) {
        images[i].offset = *(uint32_t*)(eadk_external_data + 8 + i * 8);
        images[i].size = *(uint32_t*)(eadk_external_data + 12 + i * 8);
    }

    short_display_draw_string("Press back to exit", (eadk_point_t){0, 0});

    FORMAT_SIZE(buf, external_data_size);
    short_display_draw_string(buf, (eadk_point_t){0,30});

    FORMAT_SIZE(buf, nb_previews);
    short_display_draw_string(buf, (eadk_point_t){0,45});

    FORMAT_SIZE(buf, nb_total_images);
    short_display_draw_string(buf, (eadk_point_t){0,60});

    FORMAT_SIZE(buf, nb_images);
    short_display_draw_string(buf, (eadk_point_t){0,75});

    FORMAT_SIZE(buf, images[0].offset);
    short_display_draw_string(buf, (eadk_point_t){0,100});

    FORMAT_SIZE(buf, images[0].size);
    short_display_draw_string(buf, (eadk_point_t){0,115});

    FORMAT_SIZE(buf, images[1].offset);
    short_display_draw_string(buf, (eadk_point_t){0,140});

    FORMAT_SIZE(buf, images[1].size);
    short_display_draw_string(buf, (eadk_point_t){0,155});

    FORMAT_SIZE(buf, images[2].offset);
    short_display_draw_string(buf, (eadk_point_t){0,180});

    FORMAT_SIZE(buf, images[2].size);
    short_display_draw_string(buf, (eadk_point_t){0,195});

    while (1) {
        if (eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_back)) {
            break;
        }
    }

    free(images);
    return 0;
}