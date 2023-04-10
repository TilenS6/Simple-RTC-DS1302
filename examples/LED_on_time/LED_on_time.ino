/*
  Simple_RTC - Library for communication and data interpretation with Timekeeping Chip DS1302.
  Created by Tilen Stermecki, April 8th 2023.
  Released into the public domain.
  
  class RTC:
    METHODS:
    - void begin(uint8_t clock_pin, uint8_t data_pin, uint8_t reset_pin); // initializes the class on the given pins
    - void readTime(); // reads the date from the sensor and stores it in the variable 'now' (which can be accessed by 'rtc.now')
    - void setTime(Date set); // sets the time on the chip
    - void startClock(bool); // starts or stops the clock counting (only use it if necessary, as everything is handled by the library)

    PROPERTIES:
    - Date now
  
  class Date:
    METHODS:
    - Date(){};
    - Date(uint8_t hr, uint8_t min, uint8_t sec);
    - Date(uint8_t year, uint8_t month, uint8_t day, uint8_t hr, uint8_t min, uint8_t sec);
    - Date(const char[12] date, const char[9] time); // use like this: Date mydate(__DATE__, __TIME__); for current date/time

    - bool operator>(const Date a);
    - bool operator<(const Date a);
    - bool operator==(const Date a);
    - Date operator+(const int); // adds to seconds
    - Date operator-(const int);
    - String toStr(); // ready to be displayed (e.g. to serial monitor)
    
    PROPERTIES:
    - uint8_t year, month, day, hr, min, sec, dayOfWeek;

    CONSTANTS:
    - String dayNames[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" }; // lookup table for variable 'dayOfWeek'

  Some properties:
    . pooling rate is not defined and can be as high as it is allowed by the library
    . in proprety RTC.now are stored last values got when you called .readTime();


  This example program:
    - Writes compile date/time to chip, after 10 seconds LED on pin 13 turns on.
*/

#include "Simple_RTC.h"

RTC rtc;
Date watch;
void setup() {
  Serial.begin(9600);
  rtc.begin(2, 3, 4); // clk, dat, rst
  
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);

  Date dateNow(__DATE__, __TIME__); // set time to compiled date/time
  Serial.println("\nDate/time now:\n" + dateNow.toStr());
  rtc.setTime(dateNow); // write time to DS1302

  watch = dateNow + 10; // setup time when internal LED turns on
  Serial.println("\nDate/time when LED turns on:\n" + watch.toStr());
}

void loop() {
  rtc.readTime(); // read fresh data from chip
  if (rtc.now == watch) // check if the time read from chip is same as when we want to turn on the LED
    digitalWrite(13, 1);
  delay(500); // 2Hz pooling rate (to prevent skiping a second)
}
