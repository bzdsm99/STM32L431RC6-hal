//LCD_Interface.hpp
#pragma once

class LCD_Interface
{
private:
    const char * Product_Name;
    void Interface_Init(void);


public:
    LCD_Interface(void);
    void Lock_screen(void);
    void Start_up(void);

};