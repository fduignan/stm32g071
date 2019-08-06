#include <stdint.h>
#include "display.h"
#include "spi.h"
#include "../include/STM32G07x.h"
#include "font5x7.h"
void display::begin(volatile uint32_t *ms_counter)
{    
    this->ms_counter = ms_counter;
    initSPI();
    // Turn on Port A 
    RCC->IOPENR |= (1 << 0);     
    // Reset pin is on PA6 so make it an output
    GPIOA->MODER |= (1 << 12);
    GPIOA->MODER &= ~(1 << 13);
        
    // D/C pin is on PA4 so make it an output    
    GPIOA->MODER |= (1 << 8);
    GPIOA->MODER &= ~(1 << 9);
    
    
    RSTHigh(); // Drive reset high
    sleep(25); // wait   
    RSTLow(); // Drive reset low
    sleep(25); // wait   
    RSTHigh(); // Drive reset high
    sleep(25); // wait    
    
    
    writeCommand(0x1);  // software reset
    sleep(150); // wait   
    
    writeCommand(0x11);  //exit SLEEP mode
    sleep(25); // wait   
    
    writeCommand(0x3A); // Set colour mode        
    writeData8(0x55); // 16bits / pixel @ 64k colors 5-6-5 format 
    sleep(25); // wait   
    
    writeCommand(0x36);
    writeData8(0x08);  // RGB Format
    sleep(25); // wait   
    
    
    writeCommand(0x51); // maximum brightness
    sleep(25); // wait   
    
    writeCommand(0x21);    // display inversion off (datasheet is incorrect on this point)
    writeCommand(0x13);    // partial mode off                 
    writeCommand(0x29);    // display on
    sleep(25); // wait   
    writeCommand(0x2c);   // put display in to write mode
    fillRectangle(0,0,SCREEN_WIDTH, SCREEN_HEIGHT, 0x00);  // black out the screen
}
void display::putPixel(uint16_t x, uint16_t y, uint16_t colour)
{
    openAperture(x, y, x + 1, y + 1);
    writeData16(colour); 
}
void display::putImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *Image)
{
    uint16_t Colour;
    openAperture(x, y, x + width - 1, y + height - 1);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            Colour = *(Image++);
            writeData16(Colour);
        }
    }
}
void display::fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t Colour)
{
    openAperture(x, y, x + width - 1, y + height - 1);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            writeData16(Colour);
        }
    }
}
void display::openAperture(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // open up an area for drawing on the display
    writeCommand(0x2A); // Set X limits
    writeData8(x1>>8);
    writeData8(x1&0xff);    
    writeData8(x2>>8);
    writeData8(x2&0xff);
    writeCommand(0x2B); // Set Y limits
    writeData8(y1>>8);
    writeData8(y1&0xff);    
    writeData8(y2>>8);
    writeData8(y2&0xff);
    
    writeCommand(0x2c); // put display in to data write mode
}
void display::RSLow()
{
    GPIOA->ODR &= ~(1 << 4); // drive D/C pin low
}
void display::RSHigh()
{ 
    GPIOA->ODR |= (1 << 4); // drive D/C pin high
}
void display::RSTLow()
{
    GPIOA->ODR &= ~(1 << 6); // Drive reset low
}
void display::RSTHigh()
{
    GPIOA->ODR |= (1 << 6); // Drive reset high
}
void display::writeCommand(uint8_t Cmd)
{
    RSLow();
    transferSPI8(Cmd);
}
void display::writeData8(uint8_t Data)
{
    RSHigh();
    transferSPI8(Data);
}
void display::writeData16(uint16_t Data)
{
    RSHigh();
    transferSPI16(Data);
}
void display::sleep(uint32_t dly)
{
    uint32_t end_time = *ms_counter + dly;
    while(*ms_counter  < end_time)
        asm(" wfi ");
        
}


/*void console::fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour)
{
    Display.fillRectangle(x,y,w,h,Colour);
}*/
void display::drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour)
{
    drawLine(x,y,x+w,y,Colour);
    drawLine(x,y,x,y+h,Colour);
    drawLine(x+w,y,x+w,y+h,Colour);
    drawLine(x,y+h,x+w,y+h,Colour);
}
void display::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
// Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);
    if (radius > x0)
        return; // don't draw even parially off-screen circles
    if (radius > y0)
        return; // don't draw even parially off-screen circles
        
    if ((x0+radius) > SCREEN_WIDTH)
        return; // don't draw even parially off-screen circles
    if ((y0+radius) > SCREEN_HEIGHT)
        return; // don't draw even parially off-screen circles    
    while (x >= y)
    {
        putPixel(x0 + x, y0 + y, Colour);
        putPixel(x0 + y, y0 + x, Colour);
        putPixel(x0 - y, y0 + x, Colour);
        putPixel(x0 - x, y0 + y, Colour);
        putPixel(x0 - x, y0 - y, Colour);
        putPixel(x0 - y, y0 - x, Colour);
        putPixel(x0 + y, y0 - x, Colour);
        putPixel(x0 + x, y0 - y, Colour);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}
void display::fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
// Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
// Similar to drawCircle but fills the circle with lines instead
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    if (radius > x0)
        return; // don't draw even parially off-screen circles
    if (radius > y0)
        return; // don't draw even parially off-screen circles
        
    if ((x0+radius) > SCREEN_WIDTH)
        return; // don't draw even parially off-screen circles
    if ((y0+radius) > SCREEN_HEIGHT)
        return; // don't draw even parially off-screen circles        
    while (x >= y)
    {
        drawLine(x0 - x, y0 + y,x0 + x, y0 + y, Colour);        
        drawLine(x0 - y, y0 + x,x0 + y, y0 + x, Colour);        
        drawLine(x0 - x, y0 - y,x0 + x, y0 - y, Colour);        
        drawLine(x0 - y, y0 - x,x0 + y, y0 - x, Colour);        

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}
void display::drawLineLowSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm    
  int dx = x1 - x0;
  int dy = y1 - y0;
  int yi = 1;
  if (dy < 0)
  {
    yi = -1;
    dy = -dy;
  }
  int D = 2*dy - dx;
  
  int y = y0;

  for (int x=x0; x <= x1;x++)
  {
    putPixel(x,y,Colour);    
    if (D > 0)
    {
       y = y + yi;
       D = D - 2*dx;
    }
    D = D + 2*dy;
    
  }
}

void display::drawLineHighSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
{
        // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

  int dx = x1 - x0;
  int dy = y1 - y0;
  int xi = 1;
  if (dx < 0)
  {
    xi = -1;
    dx = -dx;
  }  
  int D = 2*dx - dy;
  int x = x0;

  for (int y=y0; y <= y1; y++)
  {
    putPixel(x,y,Colour);
    if (D > 0)
    {
       x = x + xi;
       D = D - 2*dy;
    }
    D = D + 2*dx;
  }
}
void display::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    if ( iabs(y1 - y0) < iabs(x1 - x0) )
    {
        if (x0 > x1)
        {
            drawLineLowSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineLowSlope(x0, y0, x1, y1, Colour);
        }
    }
    else
    {
        if (y0 > y1) 
        {
            drawLineHighSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineHighSlope(x0, y0, x1, y1, Colour);
        }
        
    }
}
void display::print(const char *Text, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
{
        // This function draws each character individually.  It uses an array called TextBox as a temporary storage
    // location to hold the dots for the character in question.  It constructs the image of the character and then
    // calls on putImage to place it on the screen
    uint8_t Index = 0;
    uint8_t Row, Col;
    const uint8_t *CharacterCode = 0;    
    uint16_t TextBox[FONT_WIDTH * FONT_HEIGHT];
    while(Text[Index]) 
    //for (Index = 0; Index < len; Index++)
    {
        CharacterCode = &Font5x7[FONT_WIDTH * (Text[Index] - 32)];
        Col = 0;
        while (Col < FONT_WIDTH)
        {
            Row = 0;
            while (Row < FONT_HEIGHT)
            {
                if (CharacterCode[Col] & (1 << Row))
                {
                    TextBox[(Row * FONT_WIDTH) + Col] = ForeColour;
                }
                else
                {
                    TextBox[(Row * FONT_WIDTH) + Col] = BackColour;
                }
                Row++;
            }
            Col++;
        }
        putImage(x, y, FONT_WIDTH, FONT_HEIGHT, (const uint16_t *)TextBox);
        x = x + FONT_WIDTH + 2;
        Index++;
    }
}
void display::print(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
{
     // This function converts the supplied number into a character string and then calls on puText to
    // write it to the display
    char Buffer[6]; // Maximum value = 65535
    Buffer[5]=0;
    Buffer[4] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[3] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[2] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[1] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[0] = Number % 10 + '0';
    print(Buffer, x, y, ForeColour, BackColour);
}
