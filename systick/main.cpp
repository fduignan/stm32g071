// Simple blinky example : blink an LED on PA0
#include "../include/STM32G07x.h"
#include <stdint.h>

void delay(uint32_t dly)
{
    while(dly--);
}
void initClocks()
{
    // NOTE: this function assumes MCU has just come out of reset with the
    // registers defaulted to values as described in the reference manual.
    
// Use the HSI16 clock as the system clock - allows operation down to 1.5V
    RCC->CR &= ~(1 << 24); // PLL off
    // To get a 64MHz CPU core clock (R) need to run the VCO at 128MHz at least 
    // The minimum divisor for the R clock is 2
    
    FLASH->ACR |= (1 << 1); // 2 wait states for FLASH at 64MHz
    RCC->PLLSYSCFGR = (1 << 29) + (1 << 28); // select minimum R divisor of 2 and enable R output
    RCC->PLLSYSCFGR |= (1 << 12) + (1 << 4); // Divide HSI by 2 and multiply result by 16 to give 128MHz
    RCC->PLLSYSCFGR |= (1 << 1);  // clock source for PLL is HSI16
    RCC->PLLSYSCFGR |= (1 << 16); // enable PLL output
    RCC->CR |= (1 << 24); // PLL on.
    while ((RCC->CR & (1 << 25))==0); // wait for PLL to be ready
    // set PLL R output as CPU core clock source 
    RCC->CFGR |= (1 << 1);
    
}
void initSysTick()
{
    // The systick timer is driven by a 48MHz clock
    // Divide this down to achieve a 1ms timebase
    // Divisor = 64MHz/1000Hz
    // Reload value = 64000-1
    // enable systick and its interrupts
    STK->CSR |= (7); 
    STK->RVR = 64000-1; // generate 1 millisecond time base
    STK->CVR = 5;
    enable_interrupts();    
}
void Systick_Handler()
{
    static uint32_t milliseconds = 0;
    milliseconds++;
    if (milliseconds >= 1000)
    {
        milliseconds = 0;
        GPIOA->ODR ^= 1; // toggle LED
    }
}
void setup()
{
    
    initClocks(); // boost sysclock (core cpu) up to 64MHz
    // Configure PA0 as an output
    RCC->IOPENR |= (1 << 0); // enable Port A    
    GPIOA->MODER &= ~(1 << 1); // Make bit 0 an output
    GPIOA->MODER |= (1 << 0);
    
}

int main()
{
    delay(1000000); // long delay in case I screw up the clocks.
    setup();  
    initSysTick();
    
    while(1)
    {
                
    }
}
