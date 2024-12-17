#include <stdint.h>

#include "systick.h"
#include "gpio.h"
#include "uart.h"


char str[10];
int main(void)
{
    configure_systick_and_start();
    configure_gpio();
    
    UART_Init(USART2);

    UART_print(USART2, "Hello World, from main!\r\n");

    while (1) {
    }
}

