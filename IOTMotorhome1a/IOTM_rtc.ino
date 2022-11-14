// Date and time functions using a PCF8563 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_PCF8563 rtc;
uint16_t rtcYear;
uint16_t  rtcMonth;
uint16_t  rtcDay;
uint16_t  rtcHour;
uint16_t  rtcMinute;
uint16_t  rtcSecond;

void setupRtc () {

 rtc.begin();
  }


void getRtc () {
    DateTime now = rtc.now();
    rtcYear = now.year();
    rtcMonth = now.month();
    rtcDay = now.day();
    rtcHour = now.hour();
    rtcMinute = now.minute();
    rtcSecond = now.second();
}
