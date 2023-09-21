#pragma once

#include "graphics.hpp"

class Console {
    public:
        //this is an area of console columns.
        static const int kRows = 25, kColumns = 80; //common member variavle among all classes.
        
        Console(PixelWriter& writer,
            const PixelColor& fg_color, const PixelColor& bg_color);
        //output the string to console
        void PutString(const char* s);
    
    private:
        void Newline(); 
    
        PixelWriter& writer_;
        const PixelColor fg_color_, bg_color_; //fg_color is a color for string,bg_color is back ground color
        char buffer_[kRows][kColumns + 1]; //save the string which is displayed on console area
        int cursor_row_, cursor_column_; 
};