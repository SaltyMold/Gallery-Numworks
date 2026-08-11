#ifndef DISPLAY_H
#define DISPLAY_H

#include "libs/eadk.h"
#include "libs/TJpg_Decoder/tjpgd.h"

#define IMAGE_DISPLAY_OK 0
#define IMAGE_DISPLAY_OUT_OF_RANGE 1
#define IMAGE_DISPLAY_DECODE_FAILED 2

typedef struct {
    uint32_t offset;
    uint32_t size;
} image_t;

extern image_t *images;
extern const uint8_t* jpeg_stream_data;
extern uint32_t jpeg_stream_size;
extern uint32_t jpeg_stream_index;
extern uint8_t jpeg_work_buffer[TJPGD_WORKSPACE_SIZE];

extern uint32_t nb_total_images;

int display_image(uint32_t index);

void safe_display_image(uint32_t index);

/*-------------------------------------------*/

#define LARGE_FONT true
#define TEXT_COLOR eadk_color_black
#define BACKGROUND_COLOR eadk_color_white
#define RECTANGLE_COLOR eadk_color_yellow

void short_display_draw_string(const char* text, eadk_point_t point);
void short_clear_screen();
void draw_empty_rectangle(eadk_rect_t rect, eadk_color_t color);
void short_draw_empty_rectangle(eadk_rect_t rect);

#endif