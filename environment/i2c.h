#ifndef __I2C_H
#define __I2C_H
// i2c.h
#include "../include/STM32G07x.h"
#include <stdint.h>
#define MAX_I2C_DATA 130

#define I2C_WRITE_COMPLETE 1
#define I2C_READ_COMPLETE  2 
#define I2C_ERROR          -1

typedef struct {
	char Mode; // can be 'r' or 'w'	
	uint8_t SlaveAddress;  // Use the 7 bit address here
	unsigned TXIndex;
    unsigned RXIndex;
	unsigned TXCount;
    unsigned RXCount;
    volatile int Status;
    volatile int Complete;
	uint8_t TXData[MAX_I2C_DATA];
    uint8_t RXData[MAX_I2C_DATA];
    
} I2CTransaction;

class i2c 
{
public:
    i2c() {};
    void begin();
    int write(uint8_t dev_addr, uint8_t reg_num, uint8_t *data, uint8_t count);    
    int read(uint8_t dev_addr, uint8_t reg_num, uint8_t *data, int count);    
private:
    I2CTransaction Transaction;
    void start();
    void stop();
    int doTransaction(void);
    friend void I2C1_Handler(void);
};

#endif
