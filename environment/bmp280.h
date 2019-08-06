#include <stdint.h>
#include "i2c.h"
#define BMP280_R_ADDRESS (0x76)
class bmp280 {
public:
    bmp280() {};
    void begin(i2c * I2C);
    int32_t readPressure(); // returns Pressure * 100
    int32_t readTemperature(); // returns Temperature * 100
    
private:
    int readRegister(uint8_t RegNum, uint8_t *Value);
    int writeRegister(uint8_t RegNum, uint8_t Value);
    void readCalibrationData();
    int32_t t_fine;
    uint16_t dig_T1;//calibration for temperature
    int16_t  dig_T2;//calibration for temperature
    int16_t  dig_T3;//calibration for temperature
    uint16_t dig_P1;//calibration for pressure
    int16_t  dig_P2;//calibration for pressure
    int16_t  dig_P3;//calibration for pressure
    int16_t  dig_P4;//calibration for pressure
    int16_t  dig_P5;//calibration for pressure
    int16_t  dig_P6;//calibration for pressure
    int16_t  dig_P7;//calibration for pressure
    int16_t  dig_P8;//calibration for pressure
    int16_t  dig_P9;//calibration for pressure
    i2c * I2C;
};
