#ifndef SHORT_H
#define SHORT_H

#include "libs/eadk.h"

#define LARGE_FONT false
#define TEXT_COLOR eadk_color_black
#define BACKGROUND_COLOR eadk_color_white

void short_display_draw_string(const char* text, eadk_point_t point);
void short_clear_screen();

#endif