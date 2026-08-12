#include "input.h"
#include "libs/eadk.h"
#include "display.h"

bool keyboard_only_key_down(eadk_keyboard_state_t state, eadk_key_t key) {
    if (!eadk_keyboard_key_down(state, key)) {
        return false;
    }
    return state == ((eadk_keyboard_state_t)1 << (uint8_t)key);
}

void handle_held_key(eadk_key_t key, void (*action)(void), int initial_delay, int repeat_delay) {
    // Initial delay before repeat starts
    uint64_t start_time = eadk_timing_millis();
    while (eadk_timing_millis() - start_time < initial_delay) {
        state = eadk_keyboard_scan();
        if (!keyboard_only_key_down(state, key)) return;
    }
    
    // Repeat action while key is held
    while (keyboard_only_key_down(state, key)) {
        state = eadk_keyboard_scan();
        action();
        
        uint64_t repeat_start = eadk_timing_millis();
        while (eadk_timing_millis() - repeat_start < repeat_delay) {
            state = eadk_keyboard_scan();
            if (!keyboard_only_key_down(state, key)) return;
        }
    }
}

void move_left() {
    if (current_index > 0) current_index--;
    preview_screen();
}

void move_right(){
    if (current_index < nb_images - 1) current_index++;
    preview_screen();
}

void move_up(){
    if (current_index >= 5) current_index -= 5; 
    else current_index = 0;
    preview_screen();
}

void move_down(){
    if (current_index + 5 < nb_images) current_index += 5;
    else current_index = nb_images - 1;
    preview_screen();
}
