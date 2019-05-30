//
// Created by Alexander Galilov on 30.05.2019.
// E-Mail: alexander.galilov@gmail.com 

#include <stdlib.h>
#include <math.h>
#include <zconf.h>
#include <time.h>

#include "ws2812b/Led.h"
#include "main.h"
#include "Effects.h"

void setAll(WS2812B::RGB &rgb) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds.setColor(rgb, i);
    }
}

void showStrip() {
    leds.show();
}

void delay(unsigned int period) {
    usleep(1000U * period);
}

void setPixel(int ledIndex, unsigned char r, unsigned char g, unsigned char b) {
    leds.setColor(WS2812B::RGB(r, g, b), ledIndex % NUM_LEDS);
}

void setAll(unsigned char r, unsigned char g, unsigned char b) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds.setColor(WS2812B::RGB(r, g, b), i);
    }
}

void RGBLoop() {
    for (int j = 0; j < 3; j++) {
        // Fade IN
        for (int k = 0; k < 256; k++) {
            switch (j) {
                case 0:
                    setAll(k, 0, 0);
                    break;
                case 1:
                    setAll(0, k, 0);
                    break;
                case 2:
                    setAll(0, 0, k);
                    break;
            }
            showStrip();
            delay(3);
        }
        // Fade OUT
        for (int k = 255; k >= 0; k--) {
            switch (j) {
                case 0:
                    setAll(k, 0, 0);
                    break;
                case 1:
                    setAll(0, k, 0);
                    break;
                case 2:
                    setAll(0, 0, k);
                    break;
            }
            showStrip();
            delay(3);
        }
    }
}

int myRandom(int min, int max) {
    return min + (int) ((double )random() * (max - min) / (RAND_MAX + 1.0));
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay) {
    int Pixel =myRandom(0, NUM_LEDS);
    setPixel(Pixel,red,green,blue);
    showStrip();
    delay(SpeedDelay);
    setPixel(Pixel,0,0,0);
}

void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay) {
    setAll(red,green,blue);

    int Pixel = myRandom(0, NUM_LEDS);
    setPixel(Pixel,0xff,0xff,0xff);
    showStrip();
    delay(SparkleDelay);
    setPixel(Pixel,red,green,blue);
    showStrip();
    delay(SpeedDelay);
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
    int Position=0;

    for(int j=0; j<NUM_LEDS*2; j++)
    {
        Position++;
        for(int i=0; i<NUM_LEDS; i++) {
            setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
                     ((sin(i+Position) * 127 + 128)/255)*green,
                     ((sin(i+Position) * 127 + 128)/255)*blue);
        }

        showStrip();
        delay(WaveDelay);
    }
}

byte * Wheel(byte WheelPos) {
    static byte c[3];

    if(WheelPos < 85) {
        c[0]=WheelPos * 3;
        c[1]=255 - WheelPos * 3;
        c[2]=0;
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        c[0]=255 - WheelPos * 3;
        c[1]=0;
        c[2]=WheelPos * 3;
    } else {
        WheelPos -= 170;
        c[0]=0;
        c[1]=WheelPos * 3;
        c[2]=255 - WheelPos * 3;
    }

    return c;
}


void rainbowCycle(int SpeedDelay) {
    byte *c;
    uint16_t i, j;

    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
        for(i=0; i< NUM_LEDS; i++) {
            c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
            setPixel(i, *c, *(c+1), *(c+2));
        }
        showStrip();
        delay(SpeedDelay);
    }
}


void setPixelHeatColor (int Pixel, byte temperature) {
    // Scale 'heat' down from 0-255 to 0-191
    byte t192 = round((temperature/255.0)*191);

    // calculate ramp up from
    byte heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252

    // figure out which third of the spectrum we're in:
    if( t192 > 0x80) {                     // hottest
        setPixel(Pixel, 255, 255, heatramp);
    } else if( t192 > 0x40 ) {             // middle
        setPixel(Pixel, 255, heatramp, 0);
    } else {                               // coolest
        setPixel(Pixel, heatramp, 0, 0);
    }
}

void Fire(int Cooling, int Sparking, int SpeedDelay) {
    static byte heat[NUM_LEDS];
    int cooldown;

    // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
        cooldown = myRandom(0, ((Cooling * 10) / NUM_LEDS) + 2);

        if(cooldown>heat[i]) {
            heat[i]=0;
        } else {
            heat[i]=heat[i]-cooldown;
        }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if( myRandom(0, 255) < Sparking ) {
        int y = myRandom(0, 7);
        heat[y] = heat[y] + myRandom(160,255);
        //heat[y] = random(160,255);
    }

    // Step 4.  Convert heat to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
        setPixelHeatColor(j, heat[j] );
    }

    showStrip();
    delay(SpeedDelay);
}

long long millis() {
    timespec t;
    ::clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * 1000LL + t.tv_nsec / 1000000LL;
}

void BouncingBalls(byte red, byte green, byte blue, int BallCount) {
    float Gravity = -9.81;
    int StartHeight = 1;

    float Height[BallCount];
    float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
    float ImpactVelocity[BallCount];
    float TimeSinceLastBounce[BallCount];
    int   Position[BallCount];
    long long  ClockTimeSinceLastBounce[BallCount];
    float Dampening[BallCount];

    for (int i = 0 ; i < BallCount ; i++) {
        ClockTimeSinceLastBounce[i] = millis();
        Height[i] = StartHeight;
        Position[i] = 0;
        ImpactVelocity[i] = ImpactVelocityStart;
        TimeSinceLastBounce[i] = 0;
        Dampening[i] = 0.90 - float(i)/pow(BallCount,2);
    }

    while (true) {
        for (int i = 0 ; i < BallCount ; i++) {
            TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
            Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;

            if ( Height[i] < 0 ) {
                Height[i] = 0;
                ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
                ClockTimeSinceLastBounce[i] = millis();

                if ( ImpactVelocity[i] < 0.01 ) {
                    ImpactVelocity[i] = ImpactVelocityStart;
                }
            }
            Position[i] = round( Height[i] * (NUM_LEDS - 1) / StartHeight);
        }

        for (int i = 0 ; i < BallCount ; i++) {
            setPixel(Position[i],red,green,blue);
        }

        showStrip();
        setAll(0,0,0);
    }
}

void fadeToBlack(int ledNo, byte fadeValue) {

    // NeoPixel
    WS2812B::RGB oldColor;
    uint8_t r, g, b;
    int value;

    oldColor = leds.getColor(ledNo);
    r = (oldColor._r & 0x00ff0000UL) >> 16;
    g = (oldColor._g & 0x0000ff00UL) >> 8;
    b = (oldColor._b & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);

    leds.setColor(WS2812B::RGB(r,g,b), ledNo);

}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay) {
    setAll(0,0,0);

    for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {


        // fade brightness all LEDs one step
        for(int j=0; j<NUM_LEDS; j++) {
            if( (!meteorRandomDecay) || (myRandom(0, 10)>5) ) {
                fadeToBlack(j, meteorTrailDecay );
            }
        }

        // draw meteor
        for(int j = 0; j < meteorSize; j++) {
            if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
                setPixel(i-j, red, green, blue);
            }
        }

        showStrip();
        delay(SpeedDelay);
    }
}
