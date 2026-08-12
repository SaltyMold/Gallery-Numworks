#ifndef INPUT_H
#define INPUT_H

#include "libs/eadk.h"

bool keyboard_only_key_down(eadk_keyboard_state_t state, eadk_key_t key);

extern eadk_keyboard_state_t state;

extern uint32_t current_index;
extern uint32_t nb_total_images;
extern uint32_t nb_images;

void handle_held_key(eadk_key_t key, void (*action)(void), int initial_delay, int repeat_delay);
void move_left();
void move_right();
void move_up();
void move_down();

#endif