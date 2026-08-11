#include "libs/eadk.h"

#include "short.h"

void short_display_draw_string(const char* text, eadk_point_t point){
    eadk_display_draw_string(text, point, LARGE_FONT, TEXT_COLOR, BACKGROUND_COLOR);
}
void short_clear_screen(){
    eadk_display_push_rect_uniform(eadk_screen_rect, BACKGROUND_COLOR);
}