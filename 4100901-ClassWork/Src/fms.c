#include "gpio.h"
#include "fms.h"
#include "systick.h"
#include <stdint.h>


extern uint8_t button_pressed;

void run_state_machine(uint8_t* state){
    switch (*state) {
    case 0: // idle
        /*
        if (button_pressed != 0) { // If button is pressed
            *state = 1;
        } else if (systick_GetTick() >= 500) { // Blink LED every 500 ms
            *state = 2;
        }
        */
        break;
    case 1: // button pressed
        if (BUTTON_IS_RELEASED()) { // If button is released
            //button_pressed = 0; // Clear button pressed flag
            systick_reset(); // Reset counter
            state = 0;
        }
        break;
    case 2: // led toggle
        TOGGLE_LED(); // Toggle LED
        systick_reset(); // Reset counter
        state = 0;
        break;
    default:
        break;
    }
        
}
