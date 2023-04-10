/*
  Simple_RTC.h - Library for communication and data interpretation with Timekeeping Chip DS1302.
  Created by Tilen Stermecki, April 8th 2023.
  Released into the public domain.

  Datasheet used to write this library: https://docs.rs-online.com/46c9/0900766b80025c86.pdf
  (Accessed on date 9. of Apr 2023)
*/

#ifndef SIMPLE_RTC_H
#define SIMPLE_RTC_H
#include <Arduino.h>

class Date {
  bool containingOnlyTime;
  uint8_t dayOfWeekNow();
public:
  uint8_t year, month, day, dayOfWeek, hr, min, sec;

  Date(){};
  Date(uint8_t, uint8_t, uint8_t); // hr min sec
  Date(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t); // year month day hr min sec
  Date(const char[12], const char[9]);

  bool operator>(const Date);
  bool operator<(const Date);
  bool operator==(const Date);
  Date operator+(const int);
  Date operator-(const int);
  String toStr();

  friend class RTC;
};

class RTC {
  uint8_t clk_pin, dat_pin, rst_pin;
  void write(byte);
  byte readByte();
  void writeProtectionEnabled(bool);
  void startTransfer();
  void endTransfer();

public:
  Date now;
  void begin(uint8_t, uint8_t, uint8_t);
  void readTime();
  void setTime(Date);
  void startClock(bool);
};

#endif
