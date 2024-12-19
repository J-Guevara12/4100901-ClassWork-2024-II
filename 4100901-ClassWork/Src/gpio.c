#include "gpio.h"
#include "rcc.h"
#include "uart.h"
#include "fms.h"
#include "systick.h"

#define EXTI_BASE 0x40010400
#define EXTI ((EXTI_t *)EXTI_BASE)

#define EXTI15_10_IRQn 40
#define NVIC_ISER1 ((uint32_t *)(0xE000E104)) // NVIC Interrupt Set-Enable Register
#define MIN_PRESS_TIME_MS 50


#define SYSCFG_BASE 0x40010000
#define SYSCFG ((SYSCFG_t *)SYSCFG_BASE)

#define TOGGLE_LED()           (GPIOC->ODR ^= (1 << 8))

#define HEART_BEAT_LED_PIN 5 // Pin 5 of GPIOA
#define RIGHT_LED_PIN 6      // Pin 6 of GPIOA
#define LEFT_LED_PIN 7       // Pin 7 of GPIOA

#define RIGHT_PIN       13  // Pin 13 of GPIOB
#define HAZARD_PIN      14  // Pin 14 of GPIOB
#define LEFT_PIN        15  // Pin 15 of GPIOB

int last_time_pressed=0;

void configure_interrupt(GPIO_t *GPIOx, int PIN) {
    int EXTICR_idx = (PIN/4);
    int EXTICR_offset = PIN%4;
    // Configure SYSCFG EXTICR to map EXTIX to PxX
    SYSCFG->EXTICR[EXTICR_idx] &= ~(0xF << 4* EXTICR_offset); // Clear bits for EXTI13
    SYSCFG->EXTICR[EXTICR_idx] |= ((int )(GPIOx-GPIOA)/400 << 4*EXTICR_offset);  // Map EXTI13 to Port B

    // Configure EXTI13 for falling edge trigger
    EXTI->FTSR1 |= (1 << PIN);  // Enable falling trigger
    EXTI->RTSR1 &= ~(1 << PIN); // Disable rising trigger

    // Unmask EXTI13
    EXTI->IMR1 |= (1 << PIN);

    // Configure PA2 and PA3 as no pull-up, no pull-down
    GPIOx->PUPDR |= (1 << (PIN * 2)); // No pull-up, no pull-down for PA2
    init_gpio_pin(GPIOx, PIN, 0x0); // Set BUTTON pin as input

}

void configure_gpio_for_usart(void) {
    // Enable GPIOA clock
    *RCC_AHB2ENR |= (1 << 0);

    // Configure PA2 (TX) as alternate function
    GPIOA->MODER &= ~(3U << (2 * 2)); // Clear mode bits for PA2
    GPIOA->MODER |= (2U << (2 * 2));  // Set alternate function mode for PA2

    // Configure PA3 (RX) as alternate function
    GPIOA->MODER &= ~(3U << (3 * 2)); // Clear mode bits for PA3
    GPIOA->MODER |= (2U << (3 * 2));  // Set alternate function mode for PA3

    // Set alternate function to AF7 for PA2 and PA3
    GPIOA->AFR[0] &= ~(0xF << (4 * 2)); // Clear AFR bits for PA2
    GPIOA->AFR[0] |= (7U << (4 * 2));   // Set AFR to AF7 for PA2
    GPIOA->AFR[0] &= ~(0xF << (4 * 3)); // Clear AFR bits for PA3
    GPIOA->AFR[0] |= (7U << (4 * 3));   // Set AFR to AF7 for PA3

    // Configure PA2 and PA3 as very high speed
    GPIOA->OSPEEDR |= (3U << (2 * 2)); // Very high speed for PA2
    GPIOA->OSPEEDR |= (3U << (3 * 2)); // Very high speed for PA3

    // Configure PA2 and PA3 as no pull-up, no pull-down
    GPIOA->PUPDR &= ~(3U << (2 * 2)); // No pull-up, no pull-down for PA2
    GPIOA->PUPDR &= ~(3U << (3 * 2)); // No pull-up, no pull-down for PA3
}

void init_gpio_pin(GPIO_t *GPIOx, uint8_t pin, uint8_t mode)
{
    GPIOx->MODER &= ~(0x3 << (pin * 2)); // Clear MODER bits for this pin
    GPIOx->MODER |= (mode << (pin * 2)); // Set MODER bits for this pin
}

void configure_gpio(void)
{
    *RCC_AHB2ENR |= (1 << 0) | (1 << 1); // Enable clock for GPIOA and GPIOB

    // Enable clock for SYSCFG
    *RCC_APB2ENR |= (1 << 0); // RCC_APB2ENR_SYSCFGEN


    init_gpio_pin(GPIOA, HEART_BEAT_LED_PIN, 0x1); // Set LED pin as output
    init_gpio_pin(GPIOA, LEFT_LED_PIN, 0x1); // Set LED pin as output
    init_gpio_pin(GPIOA, RIGHT_LED_PIN, 0x1); // Set LED pin as output
    init_gpio_pin(GPIOC, 8, 0x1); // Set LED pin as output

    configure_interrupt(GPIOB, RIGHT_PIN);
    configure_interrupt(GPIOB, HAZARD_PIN);
    configure_interrupt(GPIOB, LEFT_PIN);

    // Enable EXTI15_10 interrupt
    *NVIC_ISER1 |= (1 << (EXTI15_10_IRQn - 32));

    configure_gpio_for_usart();
}

void EXTI15_10_IRQHandler(void)
{
    TOGGLE_LED();
    int data = EXTI->PR1;
    EXTI->PR1 = data;
    if (last_time_pressed-systick_GetTick()<MIN_PRESS_TIME_MS){
        return;
    }
    last_time_pressed = systick_GetTick();
    BUTTON b;
    switch (data) {
        case (1<<RIGHT_PIN):
            b = RIGHT; break;
        case (1<<HAZARD_PIN):
            b = HAZARD; break;
        case (1<<LEFT_PIN):
            b = LEFT; break;
    }
    update_fms(b);
}
