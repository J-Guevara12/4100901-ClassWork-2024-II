#include "systick.h"
#include "gpio.h"
#include "uart.h"

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;

} SysTick_t;

#define SysTick ((SysTick_t *)0xE000E010) // Base address of SysTick


volatile uint32_t ms_counter = 0; // Counter for milliseconds
extern int LED_LIGHT_MASK;
extern int INFINITE_BLINK;
int led_state=0;
int counter=0;


void configure_systick_and_start(void)
{
    SysTick->CTRL = 0x4;     // Disable SysTick for configuration, use processor clock
    SysTick->LOAD = 3999;    // Reload value for 1 ms (assuming 4 MHz clock)
    SysTick->CTRL = 0x7;     // Enable SysTick, processor clock, no interrupt
}

uint32_t systick_GetTick(void)
{
    return ms_counter;
}

void systick_reset(void)
{
    ms_counter = 0;
}

void SysTick_Handler(void)
{
    ms_counter = (ms_counter + 1) % (100*BLINK_PERIOD_MS+1);
    if(ms_counter%(BLINK_PERIOD_MS/2) == 0){
        if(led_state==0){
            led_state = 7;
        }
        else {
            led_state = 0;
        }
        int temporal_ouptut = led_state & (2*LED_LIGHT_MASK+1);
        GPIOA->ODR = (temporal_ouptut)<<5;
        if(temporal_ouptut & 6 && LED_LIGHT_MASK != 3){
            counter = (counter + 1)%3;
            if (counter==0 && !(INFINITE_BLINK & LED_LIGHT_MASK)){
                LED_LIGHT_MASK=0;
                INFINITE_BLINK=0;
            }
        }
    }
}
