/*
  Simple_RTC.cpp - Library for communication and data interpretation with Timekeeping Chip DS1302.
  Created by Tilen Stermecki, April 8th 2023.
  Released into the public domain.

  Datasheet used to write this library: https://docs.rs-online.com/46c9/0900766b80025c86.pdf
  (Accessed on date 9. of Apr 2023)
*/

#ifndef SIMPLE_RTC_CPP
#define SIMPLE_RTC_CPP
#include "Simple_RTC.h"

bool Date::operator>(const Date a) {
  return year > a.year     ? true
         : year < a.year   ? false
         : month > a.month ? true
         : month < a.month ? false
         : day > a.day     ? true
         : day < a.day     ? false
         : hr > a.hr       ? true
         : hr < a.hr       ? false
         : min > a.min     ? true
         : min < a.min     ? false
                           : sec > a.sec;
}
bool Date::operator<(const Date a) {
  return year < a.year     ? true
         : year > a.year   ? false
         : month < a.month ? true
         : month > a.month ? false
         : day < a.day     ? true
         : day > a.day     ? false
         : hr < a.hr       ? true
         : hr > a.hr       ? false
         : min < a.min     ? true
         : min > a.min     ? false
                           : sec < a.sec;
}
bool Date::operator==(const Date a) {
  return ((containingOnlyTime || a.containingOnlyTime) || (year == a.year && month == a.month && day == a.day)) && hr == a.hr && min == a.min && sec == a.sec;
}

Date Date::operator+(const int a) {
  Date output;
  static uint8_t daysInMonthLookup[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  uint64_t totalSeconds = year * 31536000UL;
  totalSeconds += day * 86400UL + hr * 3600UL + min * 60UL + sec + a;
  for (uint8_t m = 1; m < month; ++m)
    totalSeconds += daysInMonthLookup[m - 1] * 86400L;

  output.sec = totalSeconds % 60;
  totalSeconds /= 60;
  output.min = totalSeconds % 60;
  totalSeconds /= 60;
  output.hr = totalSeconds % 24;
  totalSeconds /= 24;

  output.year = totalSeconds / 365;
  totalSeconds = totalSeconds % 365;

  uint8_t i = 0;
  for (; totalSeconds > daysInMonthLookup[i]; ++i)
    totalSeconds -= daysInMonthLookup[i];

  output.month = i + 1;
  output.day = totalSeconds;
  output.dayOfWeek = output.dayOfWeekNow();
  output.containingOnlyTime = false;
  return output;
}


Date Date::operator-(const int a) {
  return this->operator+(-a);
}

uint8_t Date::dayOfWeekNow() {
  uint16_t trueYear = year + 2000;
  static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  trueYear -= month < 3;
  return (trueYear + trueYear / 4 - trueYear / 100 + trueYear / 400 + t[month - 1] + day) % 7;
}

String dayNames[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
String Date::toStr() {
  String dayOfWeekStr = dayNames[dayOfWeek], dateStr = "";
  if (!containingOnlyTime)
    dateStr = (String)day + ". " + (String)month + ". " + (String)((uint16_t)year + 2000) + " at ";
  return dateStr + (String)hr + ":" + (String)min + ":" + (String)sec + "(" + dayOfWeekStr + ")";
}

Date::Date(uint8_t _hr, uint8_t _min, uint8_t _sec) {
  containingOnlyTime = true;
  hr = _hr;
  min = _min;
  sec = _sec;
}
Date::Date(uint16_t _year, uint8_t _month, uint8_t _day, uint8_t _hr, uint8_t _min, uint8_t _sec) {
  containingOnlyTime = false;

  year = _year - 2000;
  month = _month;
  day = _day;
  hr = _hr;
  min = _min;
  sec = _sec;

  dayOfWeek = dayOfWeekNow();
}

/*
__DATE__: MMM DD YYYY
Jan, Jun, Jul
Feb
Mar, May
Apr, Aug
Sep
Oct
Nov
Dec

__TIME__: HH:MM:SS
*/


Date::Date(const char date[12], const char time[9]) {  // MMM DD YYYY, HH:MM:SS using __DATE__, __TIME__
  containingOnlyTime = false;
  if (date[0] == 'F') month = 2;
  else if (date[0] == 'S') month = 9;
  else if (date[0] == 'O') month = 10;
  else if (date[0] == 'N') month = 11;
  else if (date[0] == 'D') month = 12;
  else if (date[0] == 'J') {
    if (date[1] == 'a') month = 1;
    else if (date[2] == 'n') month = 6;
    else month = 7;
  } else if (date[0] == 'M') {
    if (date[2] == 'r') month = 3;
    else month = 5;
  } else {  // date[0] = 'A'
    if (date[1] == 'p') month = 4;
    else month = 8;
  }

  day = date[5] - '0';
  if (date[4] != ' ')
    day += (date[4] - '0') * 10;

  // year = (date[7] - '0') * 1000 + (date[8] - '0') * 100 + (date[9] - '0') * 10 + (date[10] - '0');
  year = (date[9] - '0') * 10 + (date[10] - '0');

  hr = (time[0] - '0') * 10 + (time[1] - '0');
  min = (time[3] - '0') * 10 + (time[4] - '0');
  sec = (time[6] - '0') * 10 + (time[7] - '0');

  dayOfWeek = dayOfWeekNow();
}
/*
RTC::RTC(uint8_t a, uint8_t b, uint8_t c) {
  begin(a, b, c);
}
*/
void RTC::begin(uint8_t clk, uint8_t dat, uint8_t rst) {
  clk_pin = clk;
  dat_pin = dat;
  rst_pin = rst;

  pinMode(clk_pin, OUTPUT);
  pinMode(dat_pin, OUTPUT);
  pinMode(rst_pin, OUTPUT);

  digitalWrite(rst_pin, 0);
  digitalWrite(clk_pin, 0);

  now.year = 0;
  now.month = 0;
  now.day = 0;
  now.hr = 0;
  now.min = 0;
  now.sec = 0;

  writeProtectionEnabled(false);
  startClock(true);
}

void RTC::write(byte a) {
  for (byte mask = 0b00000001; mask != 0; mask = mask << 1) {
    digitalWrite(dat_pin, a & mask);
    digitalWrite(clk_pin, 1);
    digitalWrite(clk_pin, 0);
  }
}

byte RTC::readByte() {
  byte out = 0;
  for (byte mask = 0b00000001; mask != 0; mask = mask << 1) {
    digitalWrite(clk_pin, 1);
    if (digitalRead(dat_pin))
      out |= mask;
    digitalWrite(clk_pin, 0);
  }
  return out;
}

void RTC::startTransfer() {
  digitalWrite(clk_pin, 0);
  digitalWrite(dat_pin, 0);
  digitalWrite(rst_pin, 1);
  delayMicroseconds(1);  // from datasheet (min)
}
void RTC::endTransfer() {
  digitalWrite(rst_pin, 0);
  delayMicroseconds(1);  // from datasheet (min)
}

void RTC::readTime() {
  startTransfer();
  write(0b10111111);  // burst read

  pinMode(dat_pin, INPUT);
  byte burst[8];
  for (uint8_t i = 0; i < 8; ++i)
    burst[i] = readByte();

  pinMode(dat_pin, OUTPUT);
  endTransfer();

  /*
  // raw response from chip readings
  Serial.println("response:");
  for (uint8_t i = 0; i < 8; ++i) {
    Serial.print((String)i + ". ");
    Serial.println(burst[i], BIN);
  }
  */

  now.year = (uint8_t)(burst[6] >> 4) * 10 + (uint8_t)(burst[6] & 0b00001111);
  now.dayOfWeek = burst[5];
  now.month = (uint8_t)(burst[4] >> 4) * 10 + (uint8_t)(burst[4] & 0b00001111);
  now.day = (uint8_t)(burst[3] >> 4) * 10 + (uint8_t)(burst[3] & 0b00001111);
  now.hr = (uint8_t)((burst[2] & 0b00110000) >> 4) * 10 + (uint8_t)(burst[2] & 0b00001111);
  now.min = (uint8_t)(burst[1] >> 4) * 10 + (uint8_t)(burst[1] & 0b00001111);
  now.sec = (uint8_t)((burst[0] & 0b01110000) >> 4) * 10 + (uint8_t)(burst[0] & 0b00001111);

  return now;
}

void RTC::writeProtectionEnabled(bool state) {
  byte a = state << 7;
  startTransfer();
  write(0b10001110);
  write(a);
  endTransfer();
}
void RTC::startClock(bool state) {
  startTransfer();
  write(0b10000001);
  pinMode(dat_pin, INPUT);
  byte r = readByte() & 0b01111111;
  pinMode(dat_pin, OUTPUT);
  endTransfer();

  if (!state)
    r |= 0b10000000;

  startTransfer();
  write(0b10000000);
  write(r);
  endTransfer();
}

void RTC::setTime(Date a) {
  startTransfer();
  //write(0b00010111);
  if (a.containingOnlyTime) {  // write each register individualy
    write(0b10000000);         // second reg
    byte data = (a.sec / 10) << 4 | a.sec % 10;
    write(data);

    write(0b10000010);  // minute reg
    data = (a.min / 10) << 4 | a.min % 10;
    write(data);
    write(0b10000100);  // hour reg
    data = (a.hr / 10) << 4 | a.hr % 10;
    write(data);
  } else {  // clock burst (all first 8 registers)
    write(0b10111110);
    byte data = (a.sec / 10) << 4 | a.sec % 10;
    write(data);
    data = (a.min / 10) << 4 | a.min % 10;
    write(data);
    data = (a.hr / 10) << 4 | a.hr % 10;
    write(data);


    data = (a.day / 10) << 4 | a.day % 10;
    write(data);
    data = (a.month / 10) << 4 | a.month % 10;
    write(data);
    write(a.dayOfWeek);

    data = (a.year / 10) << 4 | a.year % 10;
    write(data);

    write(0b00000000);  // control register (b7 = write prot. en.)
  }
  endTransfer();
}
#endif
