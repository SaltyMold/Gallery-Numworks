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

#endif