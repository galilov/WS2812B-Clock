//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com

#include "main.h"
#include "Effects.h"
#include "Clock.h"

WS2812B::Led leds(0, NUM_LEDS);

int main() {
    for (;;) {
        //RGBLoop();
        //Sparkle(0xff, 0xff, 0xff, 50);
        //SnowSparkle(0x10, 0x10, 0x10, 20, myRandom(100,1000));
        //RunningLights(0xff,0xff,0x00, 50);
        //rainbowCycle(5);
        Fire(55,120,50);
        //BouncingBalls(0xff,0,0, 3);
        //meteorRain(0xff,0xff,0xff,4, 64, true, 30);
        //showClock();
    }
    return 0;
}



