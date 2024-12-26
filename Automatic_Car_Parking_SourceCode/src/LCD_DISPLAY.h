#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h> 
#include <rgb_lcd.h>

rgb_lcd lcd;

static unsigned long previous_time_lcd = 0;
static unsigned int interval = 2000;

namespace LCD_display {
    String position[4] = {"Empty", "Empty", "Empty", "Empty"};
    enum LCD_state {
        Welcome,
        Author,
        Position
    };
    LCD_state lcd_state = Welcome;
    void LCD_display() {
        if (millis() - previous_time_lcd >= 3000){
            previous_time_lcd = millis();
            lcd.clear();
            switch (lcd_state)
            {
            case Welcome:
                lcd.setCursor(1, 0);
                lcd.print("Welcome to....");
                lcd.setCursor(1, 1);
                lcd.print("My Parking Car");
                lcd_state = Author;
                break;
            case Author:
                lcd.setCursor(4, 0);
                lcd.print("Author:");
                lcd.setCursor(1, 1);
                lcd.print("Tu Van Hoai Nam");
                lcd_state = Position;
                break;
            case Position:
                lcd.setCursor(0, 0);
                lcd.print("1:" + position[0] + "  2:" + position[1]);
                lcd.setCursor(0, 1);
                lcd.print("3:" + position[2] + "  4:" + position[3]);
                lcd_state = Welcome;
                break;
            }
        }
    }
    void update_positions(int pos, String status){
        if (pos>=0 && pos<=3) {
            position[pos] = status;
        }
    }
}

#endif