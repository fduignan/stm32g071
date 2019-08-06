#include "i2c.h"
#include "../include/STM32G07x.h"
#include <stdint.h>
// I2C class for the STM32G071 LQFP32 package.  Assuming I2C1 and pin as follows:
// PA9 (Pin 19) = SCL, alternate function 6
// PA10 (Pin 21) = SDA, alternate function 6
static i2c* pI2C1;
void i2c::begin() 
{
    pI2C1 = this; // need a pointer to this for the interrupt handler (global scope)
    // ensure Port A is powered up
    RCC->IOPENR |= BIT0;
    // Power up I2C1
    RCC->APBENR1 |= BIT21;
    // Configure port pins.  Note, external Pull-ups are assumed to be fitted.
    // Select alternate function mode
    GPIOA->MODER &= ~(BIT20 + BIT18);
    GPIOA->MODER |= (BIT21 + BIT19);
    GPIOA->AFRH  &= 0xfffff00f; // clear all AF bits
    GPIOA->AFRH  |= (BIT10 + BIT9 + BIT6 + BIT5); // set some AF bits 
    // Configure the I2C port
    RCC->APBRSTR1 &= ~BIT21; // Ensure I2C1 is out of reset        
	// Timing figures worked out by experiment.
	// Give desired I2C rate of 100kHz	
	I2C1->TIMINGR = 0x10229898;//0x10420f13;        
	NVIC->ISER |= BIT23; // enable I2C1 interrupts in NVIC
	I2C1->ICR = BIT13+BIT12+BIT11+BIT10+BIT9+BIT8+BIT5+BIT4+BIT3; // clear all pending interrupts
	I2C1->CR1 = BIT6+BIT2+BIT1; // enable the various interrupts    
}
int i2c::write(uint8_t dev_addr,  uint8_t reg_num, uint8_t *data, uint8_t count)
{
    // To write to a register you first have to write the register number and
    // then the data.
    Transaction.Mode = 'w';
    Transaction.SlaveAddress = dev_addr;
    Transaction.TXCount = count+1;
    Transaction.TXData[0] = reg_num;
    Transaction.RXCount = 0;
    int i;        
    for (i = 0; i < count; i++)
    {
        Transaction.TXData[i+1] = data[i];
    }
    return doTransaction();
        
}    
int i2c::read(uint8_t dev_addr, uint8_t reg_num, uint8_t *data, int count)
{   
    // To read from a register you have to write the register number to the
    // device, send a repeat-start and then read back the required data.
    Transaction.Mode = 'r';
    Transaction.SlaveAddress = dev_addr;
    Transaction.TXCount = 1;
    Transaction.TXData[0] = reg_num;
    Transaction.RXCount = count;
    
    doTransaction();
    int i;    
    for (i = 0; i < count; i++)
    {
        data[i]=Transaction.RXData[i];
    }

    return 0;
}
void i2c::start()
{
	I2C1->CR2 |= BIT13; // send a start signal
}
void i2c::stop(void)
{
	I2C1->CR2 |= BIT14; // send a stop signal
}
void I2C1_Handler()
{
    static unsigned ISRReg;
    ISRReg=I2C1->ISR;
    // Assuming master mode read write only (+faults)
    
    if (ISRReg & BIT1)  // TXIS Interrupt
    { 
        // Write next byte to TXDR
        I2C1->TXDR = pI2C1->Transaction.TXData[pI2C1->Transaction.TXIndex];
        if (pI2C1->Transaction.TXIndex < pI2C1->Transaction.TXCount)
            pI2C1->Transaction.TXIndex++;
        
        
    }
    if (ISRReg & BIT2)  // RXNE Interrupt
    {
        // Read next byte from RXDR        
        pI2C1->Transaction.RXData[pI2C1->Transaction.RXIndex++]=I2C1->RXDR;
        
        if (pI2C1->Transaction.RXIndex >= pI2C1->Transaction.RXCount)            
        {
            pI2C1->Transaction.RXIndex = 0; // prevent run past buffer end (precaution)
            pI2C1->stop();            
            pI2C1->Transaction.Status = I2C_READ_COMPLETE;
            pI2C1->Transaction.Complete = 1;
        }
        
    }

    if (ISRReg & BIT6)  // TC Interrupt 
    { 
        // Just finished sending a block of bytes out.
        // Next action depends on mode        
        if (pI2C1->Transaction.Mode=='w')
        {               
            pI2C1->Transaction.Status = I2C_WRITE_COMPLETE; // write complete
            // Simple write complete so issue stop
            pI2C1->stop();             
            pI2C1->Transaction.Complete = 1;
        } 
        else
        {
            // This is part of a read operation so set up registers for 
            // reception of the expected number of bytes and issue repeat start.
            unsigned TempVal = I2C1->CR2 & 0xff00ffff;   // read current value of CR2
            TempVal |= (pI2C1->Transaction.RXCount & 0xff) << 16; // assuming transfer length of < 256
            // A read is due so set read flag
            TempVal |= BIT10; // set WRN=1 =>Read
            I2C1->CR2 = TempVal; // write register value out
            
            pI2C1->start(); // send repeat start
        }        

    }

    if (ISRReg & BIT7)  // TCR Interrupt
    {
        // Receive complete
        pI2C1->Transaction.Status = I2C_READ_COMPLETE; // read complete
        pI2C1->stop();
        pI2C1->Transaction.Complete = 1;
    }
    if (ISRReg & (BIT11+BIT10+BIT9+BIT8))  // Error Interrupt
    {
        // Something has gone wrong, flag an error and finish up
        pI2C1->Transaction.Status = I2C_ERROR; // Flag an error
        pI2C1->Transaction.Complete = 1;
    }   
}
int i2c::doTransaction()
{   // Assuming MASTER mode
    // Blocking call for I2C comms.  Power save sleep while waiting to be included.
    
    unsigned TempVal;
	int Timeout=0xfffff;
    I2C1->CR1 &= ~BIT0;   // disable the I2C Interface	
	Transaction.TXIndex=0;
    Transaction.RXIndex=0;
	Transaction.Status=0;
    Transaction.Complete = 0;
    // Must program length of outgoing transfer into NBYTES in I2C_CR2
    TempVal = I2C1->CR2 & 0xff00ffff;   // read current value of CR2
    TempVal |= (Transaction.TXCount & 0xff) << 16; // assuming transfer length of < 256
    TempVal &= 0xffffff00;
    TempVal |= (Transaction.SlaveAddress & 0x7f) << 1;
    TempVal &= ~BIT11; // 7 bit addressing
    Transaction.Mode |= 32; // enforce lower case    
    TempVal &= ~BIT10; // set WRN=0 =>Write : all transactions start with a write
        
    // Will do software end mode (i.e. software will send the stop signal)
    TempVal &= ~BIT25;
    I2C1->CR2 = TempVal;            // Write new value out    
    I2C1->CR1 |= BIT0;              // enable the I2C Interface
    start();                        // Begin transmission
    
	// Now need to wait for the transaction to complete.      
	while ( (Transaction.Complete ==  0) && (Timeout--) ); 	
        //cpu_sleep();      // Can't sleep when using deep-sleep mode of CPU - I2C can't wake CPU back up.     	
    if (Timeout <= 0)
        return I2C_ERROR; 
    else
        return Transaction.Status;
	
}
