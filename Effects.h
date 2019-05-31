//
// Created by Alexander Galilov on 30.05.2019.
// E-Mail: alexander.galilov@gmail.com 

#ifndef EFFECTS_H
#define EFFECTS_H

//Visual effects: https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/

typedef unsigned char byte;
typedef unsigned int uint32_t;
typedef byte uint8_t;
typedef  unsigned  short uint16_t;

void RGBLoop();
void Sparkle(byte red, byte green, byte blue, int SpeedDelay);
void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay);
void RunningLights(byte red, byte green, byte blue, int WaveDelay);
void rainbowCycle(int SpeedDelay);
void Fire(int Cooling, int Sparking, int SpeedDelay);
void BouncingBalls(byte red, byte green, byte blue, int BallCount);
void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay);

#endif //EFFECTS_H
