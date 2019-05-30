//
// Created by Alexander Galilov on 30.05.2019.
// E-Mail: alexander.galilov@gmail.com 

#include <time.h>
#include <zconf.h>
#include "main.h"
#include "Clock.h"

// brights
#define HIGH_LIGHT0      127
#define LOW_WHITE        15,15,15

void showClock() {

    leds.clear();
    time_t t = ::time(nullptr);
    tm *l_t = ::localtime(&t);

    int sec = l_t->tm_sec;
    int min = l_t->tm_min;
    int hour = l_t->tm_hour;

    for (size_t hourDividerIndex = 0; hourDividerIndex <= 12; hourDividerIndex++) {
        leds.setColor(WS2812B::RGB(LOW_WHITE), hourDividerIndex * (NUM_LEDS / 12));
    }

    leds.getColor(sec).setB(HIGH_LIGHT0);
    leds.getColor(min).setG(HIGH_LIGHT0);
    leds.getColor((NUM_LEDS / 12) * (hour % 12) + (min / 15)).setR(HIGH_LIGHT0);
    leds.show();
    sleep(1);
}