//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com

#include <zconf.h>
#include <time.h>
#include "ws2812b/Led.h"
// I use 60 leds for clock
#define NUM_LEDS         60
// brights
#define LOW_LIGHT        15
#define MIDDLE_LIGHT0    31
#define MIDDLE_LIGHT1    63
#define HIGH_LIGHT0      127
#define HIGH_LIGHT1      255

// some usable colors
#define PEACH 200,50,5
#define CYAN 10,150,70
#define PURPLE 180,3,180
#define BLUE 5,5,190
#define WHITE 150,150,150
#define LOW_WHITE 15,15,15
#define GREEN 10,180,10

WS2812B::Led leds(0, NUM_LEDS);

void showClock() {

    leds.clear();
    time_t t = ::time(NULL);
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
}

int main() {
    for (;;) {
        showClock();
        sleep(1);
    }
    return 0;
}