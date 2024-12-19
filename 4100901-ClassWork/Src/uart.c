#include "uart.h"
#include "rcc.h"
#include "gpio.h"
#include "fms.h"

#include "nvic.h"
#include <stdint.h>


void UART_enable_nvic_it(USART_TypeDef * UARTx) {
    if (UARTx == USART1) {
        NVIC->ISER[1] |= (1 << 5);
    } else if (UARTx == USART2) {
        NVIC->ISER[1] |= (1 << 6);
    } else if (UARTx == USART3) {
        NVIC->ISER[1] |= (1 << 7);
    }
}


void UART_clock_enable(USART_TypeDef * UARTx) {
    if (UARTx == USART1) {
        *RCC_APB2ENR |= RCC_APB2ENR_USART1EN;
    } else if (UARTx == USART2) {
        *RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    } else if (UARTx == USART3) {
        *RCC_APB1ENR1 |= RCC_APB1ENR1_USART3EN;
    }
}

void UART_Init (USART_TypeDef * UARTx) {
    UART_clock_enable(UARTx);
    // Disable USART
    UARTx->CR1 &= ~USART_CR1_UE;

    // Set data length to 8 bits (clear M bit)
    UARTx->CR1 &= ~USART_CR1_M;

    // Select 1 stop bit (clear STOP bits in CR2)
    UARTx->CR2 &= ~USART_CR2_STOP;

    // Set parity control as no parity (clear PCE bit)
    UARTx->CR1 &= ~USART_CR1_PCE;

    // Oversampling by 16 (clear OVER8 bit)
    UARTx->CR1 &= ~USART_CR1_OVER8;

    // Set Baud rate to 9600 using APB frequency (4 MHz)
    UARTx->BRR = BAUD_9600_4MHZ;

    // Enable transmission and reception
    UARTx->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    UART_enable_nvic_it(UARTx);
    UARTx->CR1 |= (1 << 5);
    UARTx->CR1 &= ~(1 << 7);

    // Enable USART
    UARTx->CR1 |= USART_CR1_UE;

    // Verify that USART is ready for transmission
    while ((UARTx->ISR & USART_ISR_TEACK) == 0);

    // Verify that USART is ready for reception
    while ((UARTx->ISR & USART_ISR_REACK) == 0);

}

uint8_t *rx_buffer;
uint8_t rx_len;
uint8_t rx_index;
uint8_t rx_ready;

char *USART2_Buffer_Rx;
uint8_t Tx2_Counter;
uint8_t tx_ready=1;


void UART_receive_it(USART_TypeDef * UARTx, uint8_t *buffer, uint8_t len)
{
    rx_buffer = buffer;
    rx_len = len;
    rx_index = 0;
}


void UART_print(USART_TypeDef *USARTx, char *str){
    USARTx->CR1 |= USART_CR1_TXEIE;
    USART2_Buffer_Rx = str;
    Tx2_Counter = 0;
    tx_ready = 0;
    USARTx->TDR = '\0';
}



void USART2_IRQHandler(void) {
    // Check if the USART2 receive interrupt flag is set
    if (USART2->ISR & USART_ISR_RXNE) {
        // Clear the interrupt flag
        USART2->ICR |= USART_ISR_RXNE;
        uint32_t data = USART2->RDR;
        // Read received data
        switch (data) {
            case LEFT:
            case RIGHT:
            case HAZARD:
                update_fms(data);
                break;
            default:
                UART_print(USART2, "Unknown command, please use either 'L', 'R', or 'P'\r\n");
                return;
        }
    }

    if (USART2->ISR & USART_ISR_TXE) {
        if (USART2_Buffer_Rx[Tx2_Counter] != '\0') {
            USART2->TDR = USART2_Buffer_Rx[Tx2_Counter];
            Tx2_Counter++;
        }
        else {
            tx_ready=1;
            USART2->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}
