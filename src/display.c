#include "display.h"
#include <string.h>
#include "short.h"

image_t *images = NULL;
const uint8_t* jpeg_stream_data = NULL;
uint32_t jpeg_stream_size = 0;
uint32_t jpeg_stream_index = 0;
uint8_t jpeg_work_buffer[TJPGD_WORKSPACE_SIZE] __attribute__((aligned(4)));

static size_t jpeg_input(JDEC* jd, uint8_t* buf, size_t len) {
    (void)jd;
    if (jpeg_stream_index >= jpeg_stream_size) return 0;
    size_t remain = jpeg_stream_size - jpeg_stream_index;
    if (len > remain) len = remain;
    if (buf) memcpy(buf, jpeg_stream_data + jpeg_stream_index, len);
    jpeg_stream_index += len;
    return len;
}

static int jpeg_output(JDEC* jd, void* bitmap, JRECT* jrect) {
    (void)jd;
    eadk_rect_t rect = {
        .x = (uint16_t)jrect->left,
        .y = (uint16_t)jrect->top,
        .width = (uint16_t)(jrect->right + 1 - jrect->left),
        .height = (uint16_t)(jrect->bottom + 1 - jrect->top)
    };
    eadk_display_push_rect(rect, (const eadk_color_t*)bitmap);
    return 1;
}

int display_image(uint32_t index) {
    if (!images || index >= nb_total_images) {
        return IMAGE_DISPLAY_OUT_OF_RANGE;
    }

    jpeg_stream_data = (const uint8_t*)eadk_external_data + images[index].offset;
    jpeg_stream_size = images[index].size;
    jpeg_stream_index = 0;

    JDEC jd;
    jd.swap = 0; // Use native RGB565 byte order for the calculator display
    JRESULT result = jd_prepare(&jd, jpeg_input, jpeg_work_buffer,
                                sizeof(jpeg_work_buffer), NULL);
    if (result == JDR_OK) {
        result = jd_decomp(&jd, jpeg_output, 0);
    }

    return result == JDR_OK ? IMAGE_DISPLAY_OK : IMAGE_DISPLAY_DECODE_FAILED;
}

void safe_display_image(uint32_t index) {
    if (images[0].size == 0) {
        short_display_draw_string("No image data", (eadk_point_t){0, 0});
    } else {
        int result = display_image(index);
        if (result == IMAGE_DISPLAY_OUT_OF_RANGE) {
            short_display_draw_string("Index out of range", (eadk_point_t){0, 0});
        } else if (result == IMAGE_DISPLAY_DECODE_FAILED) {
            short_display_draw_string("JPEG decode failed", (eadk_point_t){0, 0});
        }
    }

}