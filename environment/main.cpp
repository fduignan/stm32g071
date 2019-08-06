// Dust sensor example based on the DSM501A
// Dust sensor output (output 2) is connected to PB8 (5V tolerant pin)
// This pin is driven low when dust is detected.  The percentage low-time
// gives an indication of the dust concentration

#include "../include/STM32G07x.h"
#include <stdint.h>
#include "display.h"
#include "serial.h"
#include "bmp280.h"
#include "tls2561.h"
volatile int LowTime=0;
volatile int UnstableLowTime = 0;
volatile uint32_t Seconds = 0;
volatile uint32_t Minutes = 0;
volatile uint32_t milliseconds = 0;
i2c I2C;
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

    milliseconds++;
    if (GPIOB->IDR & (1 << 8))
    {
        GPIOA->ODR &= ~1; // LED off when no dust
    }
    else
    {
        GPIOA->ODR |= 1; // LED on for dust
        UnstableLowTime++;
    }
    if (milliseconds >= 1000)
    {
        milliseconds = 0;
        Seconds ++;
        if (Seconds > 59)
        {
        
            LowTime = UnstableLowTime;
            UnstableLowTime = 0;
            Seconds=0;
            Minutes++;
        }
        
        
    }
}
void setup()
{
    
    initClocks(); // boost sysclock (core cpu) up to 64MHz
    // Configure PA0 as an output
    RCC->IOPENR |= (1 << 0); // enable Port A    
    GPIOA->MODER &= ~(1 << 1); // Make bit 0 an output
    GPIOA->MODER |= (1 << 0);
    RCC->IOPENR |= (1 << 1); // enable Port B
    GPIOB->MODER &= ~((1 << 17) + (1 << 16)); // Make PB8 an input
    
}
serial Serial;
display Display;
bmp280 BMP280;
tls2561 TLS2561;
int main()
{
    uint16_t datapoints[200];
    int Temperature;
    int Pressure;
    int Light0,Light1;
    char c;
    uint32_t old_time=0;    
    int index;
    delay(1000000); // long delay in case I screw up the clocks.
    setup();  
    initSysTick();
    Serial.begin();
    Display.begin(&milliseconds);    
    enable_interrupts();
    I2C.begin();    
    BMP280.begin(&I2C);
    TLS2561.begin(&I2C);
    Display.fillRectangle(0,0,239,239,RGBToWord(0,0,0));    
    Display.print("Environment sensor",100,10,RGBToWord(255,255,255),RGBToWord(0,0,0));
    Display.drawLine(20,20,20,220,RGBToWord(255,255,255));
    Display.drawLine(20,220,220,220,RGBToWord(255,255,255));
    Serial.print("Environment sensor\r\n");
    
    for (index=0;index<200;index++)
        datapoints[index]=0;
    while(1)
    {
        
        if (old_time != Minutes)
        {
            Temperature = BMP280.readTemperature();
            Pressure = BMP280.readPressure();
            Light0 = TLS2561.readCH0();
            Light1 = TLS2561.readCH1();
            Serial.print("Low Time: ");              
            Serial.print(LowTime);                     
            Serial.print("\r\n");
            Serial.print("Temp x 100: ");              
            Serial.print(Temperature);                     
            Serial.print("\r\n");
            Serial.print("Pressure: ");              
            Serial.print(Pressure);                     
            Serial.print("\r\n");
            Serial.print("Light0: ");              
            Serial.print(Light0);                     
            Serial.print("\r\n");        
            Serial.print("Light1: ");              
            Serial.print(Light1);                     
            Serial.print("\r\n");        
            
            old_time = Minutes;            
            // Shunt the datapoints
            for (index = 1; index < 200;index++)
            {
                datapoints[index-1] = datapoints[index];                
            }                        
            datapoints[199]=4*LowTime / (60000/200); // This scaling is based on 60000 milliseconds in a minute, 200 dots vertical on the graph, and a max reading of 25% 
            Display.fillRectangle(0,0,239,239,RGBToWord(0,0,0));    
            Display.print("Dust sensor",40,10,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print(LowTime,120,10,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print("Temp x 100",40,25,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print(Temperature,120,25,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print("Pressure",40,40,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print(Pressure,120,40,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print("Light 0",40,55,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print(Light0,120,55,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print("Light 1",40,70,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.print(Light1,120,70,RGBToWord(255,255,255),RGBToWord(0,0,0));
            Display.drawLine(20,20,20,220,RGBToWord(255,255,255));
            Display.drawLine(20,220,220,220,RGBToWord(255,255,255));
            for (index = 0; index < 200;index++)
            {                
                Display.putPixel(21+index,220-datapoints[index],RGBToWord(255,255,0));
            }
            
        }
    }
}
