#include "stdint.h"
#include "fms.h"
#include "systick.h"
#include "uart.h"

uint8_t INFINITE_BLINK=0;
int LED_LIGHT_MASK=0;
int latest_update=0;


void run_fms(BUTTON b, int* state){
    switch (*state) {
        case 0:
            switch (b) {
                case LEFT: *state = 2;  UART_print(USART2, "Turning Left\r\n"); break;
                case RIGHT: *state = 1; UART_print(USART2, "Turning Right\r\n"); break;
                case HAZARD: *state = 3; UART_print(USART2, "Hazard Lights toggled\r\n");break;
            };
            break;
        case 1:
            switch (b) {
                case LEFT: *state = 0; break;
                case RIGHT: *state = 1; break;
                case HAZARD: *state = 3; UART_print(USART2, "Hazard Lights toggled\r\n"); break;
            };
            break;
        case 2:
            switch (b) {
                case LEFT: *state = 2; break;
                case RIGHT: *state = 0; break;
                case HAZARD: *state = 3; UART_print(USART2, "Hazard Lights toggled\r\n"); break;
            };
            break;
        case 3:
            switch (b) {
                case HAZARD: *state = 0; UART_print(USART2, "Hazard Lights toggled\r\n"); break;
                default: break;
            };
            break;
    }
}

void update_fms(BUTTON b){
    if (latest_update==0){
        latest_update = systick_GetTick();
    }
    int previous_state = LED_LIGHT_MASK;
    run_fms(b, &LED_LIGHT_MASK);
    if (previous_state != LED_LIGHT_MASK){
        INFINITE_BLINK=0;
    }
    else if (systick_GetTick()-latest_update <= LONG_BUTTON_PERIOD_MAX_MS) {
        INFINITE_BLINK |= 3 & LED_LIGHT_MASK;
        UART_print(USART2, "Infinite blink activated\n\r");
    }
    latest_update = systick_GetTick();
}
