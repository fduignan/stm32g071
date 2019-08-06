// tls2561.h
#include <stdint.h>
#include "i2c.h"
#define TLS2561_ADDRESS (0x39)
class tls2561 {
public:
    tls2561() {};
    void begin(i2c * I2C);
    int32_t readCH0();
    int32_t readCH1();
    
    
private:
    int readRegister(uint8_t RegNum, uint8_t *Value);
    int writeRegister(uint8_t RegNum, uint8_t Value);
    i2c * I2C;
};
