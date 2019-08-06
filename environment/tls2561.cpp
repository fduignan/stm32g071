// tls2561.cpp
#include <stdint.h>
#include "tls2561.h"
#include "i2c.h"


void tls2561::begin(i2c * I2C)
{
    this->I2C = I2C;    
    writeRegister(0x80,3);
}
int32_t tls2561::readCH0()
{
    uint8_t ch0_lo,ch0_hi;    
    readRegister(0x8c,&ch0_lo);
    readRegister(0x8d,&ch0_hi);
    uint32_t ch0;
    ch0 = (uint32_t)ch0_lo + (((uint32_t)ch0_hi)<<8);    
    return ch0;
}
int32_t tls2561::readCH1()
{
    
    uint8_t ch1_lo,ch1_hi;
    readRegister(0x8e,&ch1_lo);
    readRegister(0x8f,&ch1_hi);
    uint32_t ch1;
    ch1 = (uint32_t)ch1_lo + (((uint32_t)ch1_hi)<<8);
    return ch1;
}

int tls2561::readRegister(uint8_t RegNum, uint8_t *Value)
{
    //reads a series of bytes, starting from a specific register
    int nack;
    nack = I2C->read(TLS2561_ADDRESS,RegNum,Value,1); // read a byte from the register and store in buffer
    return(nack);
}
int tls2561::writeRegister(uint8_t RegNum, uint8_t Value)
{
    //sends a byte to a specific register
    uint8_t Buffer[2];    
    Buffer[0]= Value;    
    int nack;
    nack = I2C->write(TLS2561_ADDRESS, RegNum, Buffer,1);
    return(nack);
}    

