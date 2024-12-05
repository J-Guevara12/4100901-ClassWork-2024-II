#include <stdint.h>

#include "systick.h"
#include "gpio.h"
#include "fms.h"

int main(void)
{
    configure_systick_and_start();
    configure_gpio();

    uint8_t state = 0; // state of the FSM

    while (1) {
        run_state_machine(&state);
    }
}
