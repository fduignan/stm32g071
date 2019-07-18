// Simple blinky example : blink an LED on PA0
#include "../include/STM32G07x.h"
#include <stdint.h>

void delay(uint32_t dly)
{
    while(dly--);
}
int main()
{
    RCC->IOPENR |= (1 << 0); // enable Port A    
    GPIOA->MODER &= ~(1 << 1); // Make bit 0 an output
    GPIOA->MODER |= (1 << 0);
    while(1)
    {
        GPIOA->ODR |= 1;
        delay(100000);
        GPIOA->ODR &= ~1;
        delay(100000);
    }
}
