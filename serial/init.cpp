#include <stdint.h>
#include "../include/STM32G07x.h"

void init(void);
void Default_Handler(void);
void Systick_Handler(void);
void USART2_Handler(void);
void Line_State_Handler(void);
int main(void);
// The following are 'declared' in the linker script
extern unsigned char  INIT_DATA_VALUES;
extern unsigned char  INIT_DATA_START;
extern unsigned char  INIT_DATA_END;
extern unsigned char  BSS_START;
extern unsigned char  BSS_END;


extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

typedef void (*fptr)(void);
// the section "vectors" is placed at the beginning of flash 
// by the linker script
#pragma GCC push_options
#pragma GCC optimize ("O0")

const fptr Vectors[] __attribute__((section(".vectors"))) ={
	(fptr)0x20001000, 	/* Top of stack (4k) */ 
	init,   		/* Reset Handler */
	Default_Handler,	/* NMI */
	Default_Handler,	/* Hard Fault */
	Default_Handler,	/* MemManage */
	Default_Handler,	/* Reserved  */
	Default_Handler,	/* Reserved */
	Default_Handler,	/* Reserved */ 
	Default_Handler,	/* Reserved */
	Default_Handler,	/* Reserved */
	Default_Handler,	/* Reserved */
	Default_Handler,	/* SVCall */
	Default_Handler,	/* Reserved */
	Default_Handler,	/* Reserved */
	Default_Handler,	/* PendSV */
	Systick_Handler,	/* SysTick */	
/* External interrupt handlers follow */
	Default_Handler, 	/* 0: WWDG */
	Default_Handler, 	/* 1: PVD */
	Default_Handler, 	/* 2: RTC */
	Default_Handler, 	/* 3: FLASH */
	Default_Handler, 	/* 4: RCC */
	Default_Handler, 	/* 5: EXTI0_1 */
	Default_Handler, 	/* 6: EXTI2_3 */
	Default_Handler,    /* 7: EXTI4_15 */
	Default_Handler, 	/* 8: UCDP1/UCDP2 */
	Default_Handler, 	/* 9: DMA_CH1 */
	Default_Handler, 	/* 10: DMA_CH2_3 */
	Default_Handler, 	/* 11: DMA_CH4_5_6_7 */
	Default_Handler, 	/* 12: ADC/COMP */
	Default_Handler, 	/* 13: TIM1_BRK_UP_TRG_COM */
	Default_Handler, 	/* 14: TIM1_CC */
	Default_Handler, 	/* 15: TIM2 */
	Default_Handler, 	/* 16: TIM3 */
	Default_Handler, 	/* 17: TIM6_DAC/LPTIM1 */
	Default_Handler, 	/* 18: TIM7/LPTIM2 */
	Default_Handler, 	/* 19: TIM14 */
	Default_Handler, 	/* 20: TIM15 */
	Default_Handler, 	/* 21: TIM16 */
	Default_Handler, 	/* 22: TIM17 */
	Default_Handler, 	/* 23: I2C1 */
	Default_Handler, 	/* 24: I2C2 */
	Default_Handler, 	/* 25: SPI1 */
	Default_Handler, 	/* 26: SPI2 */
	Default_Handler, 	/* 27: USART1 */
	USART2_Handler, 	/* 28: USART2 */
	Default_Handler, 	/* 29: USART3/USART4/LPUART1 */
	Default_Handler, 	/* 30: CEC */
	Default_Handler 	/* 31: AES/RNG */
	
};
void initClock()
{
    return;
    // After reset, CPU clock is set to HSISYS = 16MHz
    
// This is potentially a dangerous function as it could
// result in a system with an invalid clock signal - result: a stuck system
        // Set the PLL up
        // First ensure PLL is disabled
        RCC->CR &= ~(1<<24);
        while( (RCC->CR & (1 <<25))); // wait for PLL ready to be cleared
        
  // Warning here: if system clock is greater than 24MHz then wait-state(s) need to be
        // inserted into Flash memory interface
        FLASH->ACR |= (1 << 0);
        FLASH->ACR &=~((1 << 2) | (1<<1));
        // Turn on FLASH prefetch buffer
        FLASH->ACR |= (1 << 4);
        // set PLL multiplier to 12 (yielding 48MHz)
        RCC->CFGR &= ~((1<<21) | (1<<20) | (1<<19) | (1<<18));
        RCC->CFGR |= ((1<<21) | (1<<19) ); 

        // and turn the PLL back on again
        RCC->CR |= (1<<24);        
        // set PLL as system clock source 
        RCC->CFGR |= (1<<1);
}
void init_array()
{
    // This function calls constructors for global and static objects
    uint32_t count;
    uint32_t i;
    
    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start[i] ();
    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++)
        __init_array_start[i] (); 
}
void init()
{
// do global/static data initialization
	unsigned char *src;
	unsigned char *dest;
	unsigned len;
    initClock();
	src= &INIT_DATA_VALUES;
	dest= &INIT_DATA_START;
	len= &INIT_DATA_END-&INIT_DATA_START;
	while (len--)
		*dest++ = *src++;
// zero out the uninitialized global/static variables
	dest = &BSS_START;
	len = &BSS_END - &BSS_START;
	while (len--)
		*dest++=0;
    init_array();
	main();
}

void Default_Handler()
{
	while(1);
}
#pragma GCC pop_options
