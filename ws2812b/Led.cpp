//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com

#include <unistd.h>
#include <time.h>
#include <stdexcept>
#include <bits/ios_base.h>
#include "Led.h"

using namespace WS2812B;

// See protocol description at https://static.chipdip.ru/lib/554/DOC001554925.pdf or
// https://www.digikey.com/en/datasheets/parallaxinc/parallax-inc-28085-ws2812b-rgb-led-datasheet

#if !USE_SIMPLE_IMPLEMENTATION

// Set bitrate to *about* 5 MHz.
#define SPI_SPEED           5000000

// see constructor code and https://github.com/raspberrypi/linux/issues/2094
double g_realSpiSpeed = 0;

#define LONG_SIGNAL_uS    0.9
#define SHORT_SIGNAL_uS   0.35
#define RESET_SIGNAL_uS   70.0
#define uS_PER_SECOND     1000000.0
#define LONG_SIGNAL_IN_SPI_BITS     ((int)(LONG_SIGNAL_uS*g_realSpiSpeed/uS_PER_SECOND))
#define SHORT_SIGNAL_IN_SPI_BITS    ((int)(SHORT_SIGNAL_uS*g_realSpiSpeed/uS_PER_SECOND))
#define RESET_SIGNAL_IN_SPI_BITS    ((int)(RESET_SIGNAL_uS*g_realSpiSpeed/uS_PER_SECOND))


Led::Led(int spiChannel, int numLeds) :
        _spiChannel(spiChannel), _numLeds(numLeds) {
    _spi.open(_spiChannel, SPI_SPEED, 0);

    // Some magic to fix https://github.com/raspberrypi/linux/issues/2094
    if (g_realSpiSpeed == 0) {
        const auto bufSize = 16000;
        const double blockTransferApproxTime = bufSize * 8.0 / SPI_SPEED;
        const auto nIterations = (size_t)(1.0 / blockTransferApproxTime);
        std::vector<unsigned char> data;
        data.reserve(bufSize);
        for(int i = 0; i < bufSize; i++) {
            data.push_back(0);
        }
        ::timespec t0, t1;
        ::clock_gettime(CLOCK_REALTIME, &t0); //CLOCK_PROCESS_CPUTIME_ID does not work :(
        for (int i = 0; i < nIterations; i++) {
            _spi.write(data);
        }
        ::clock_gettime(CLOCK_REALTIME, &t1);
        long long nSecT0 = t0.tv_sec * 1000000000LL + t0.tv_nsec;
        long long nSecT1 = t1.tv_sec * 1000000000LL + t1.tv_nsec;
        g_realSpiSpeed = nIterations * 8.0 * bufSize * 1000000000L / (nSecT1 - nSecT0);
        //printf("g_realSpeed: %f bits per second\n", g_realSpiSpeed);
    }

    // allocate RGB data
    _rgbData.reserve(numLeds);
    for (int i = 0; i < numLeds; i++) {
        _rgbData.emplace_back(RGB());
    }
    // allocate SPI buffer
    size_t sz = (size_t) ((LONG_SIGNAL_IN_SPI_BITS + SHORT_SIGNAL_IN_SPI_BITS) * 8 * 3 * _numLeds +
                          RESET_SIGNAL_IN_SPI_BITS) / 8;
    _spiData.reserve(sz);
}

void Led::show() {
    const size_t startBitIndex = 7;
    unsigned char current = 0;
    size_t bitIndex = startBitIndex;
    for (auto const &srcRGBValue: _rgbData) {
        unsigned char grb[] = {srcRGBValue._g, srcRGBValue._r, srcRGBValue._b};
        for (auto const &srcValue:grb) {
            for (size_t srcBitIndex = 0; srcBitIndex < 8; srcBitIndex++) {
                size_t srcBitValue = (srcValue << srcBitIndex) & 0x080U;
                int firstSemiWave = srcBitValue != 0 ? LONG_SIGNAL_IN_SPI_BITS : SHORT_SIGNAL_IN_SPI_BITS;
                for (size_t i = 0; i < LONG_SIGNAL_IN_SPI_BITS + SHORT_SIGNAL_IN_SPI_BITS; i++) {
                    unsigned char targetBit = (firstSemiWave-- > 0) ? 1U : 0U;
                    //bitIndex = 7 - (i % 8);
                    if (targetBit) {
                        current |= (1U << bitIndex);
                    }
                    if (bitIndex-- == 0) {
                        bitIndex = startBitIndex;
                        _spiData.push_back(current);
                        current = 0;
                    }
                }
            }
        }
    }
    if (bitIndex > -1) {
        _spiData.push_back(current);
    }
    for (size_t i = 0; i < RESET_SIGNAL_IN_SPI_BITS / 8; i++) {
        _spiData.push_back(0);
    }
    _spi.write(_spiData);
    _spiData.clear();
}

#else
// Simple implementation
// Set SPI bitrate to 3.333 MHz.
#define SPI_SPEED           3333000

// Each PWM bit is projected to 4 SPI bits at 3.333MHz
#define PWM_ZERO                    0b1000
#define PWM_ONE                     0b1110
#define RESET_SIGNAL_IN_SECONDS     0.000060
#define RESET_SIGNAL_IN_SPI_BYTES   ((int)(SPI_SPEED * RESET_SIGNAL_IN_SECONDS)/8.0)

Led::Led(int spiChannel, int numLeds) :
        _spiChannel(spiChannel), _numLeds(numLeds) {
    _spi.open(_spiChannel, SPI_SPEED, 0);
    // allocate RGB data
    _rgbData.reserve(numLeds);
    for (int i = 0; i < numLeds; i++) {
        _rgbData.emplace_back(RGB());
    }
    // allocate SPI buffer 12 bytes (24 bits PWM) per LED + RESET signal size in bytes
    size_t sz = (size_t) (12 * _numLeds + RESET_SIGNAL_IN_SPI_BYTES);
    _spiData.reserve(sz);
}

void Led::show() {
    for (auto const &srcRGBValue: _rgbData) {
        unsigned char grb[] = {srcRGBValue._g, srcRGBValue._r, srcRGBValue._b};
        for (auto const &srcValue:grb) {
            unsigned char current = 0;
            for (size_t srcBitIndex = 0; srcBitIndex < 8; srcBitIndex++) {
                size_t srcBitValue = (srcValue << srcBitIndex) & 0x080U;
                unsigned char pattern = (srcBitValue != 0 ? PWM_ONE : PWM_ZERO);
                if (srcBitIndex % 2 == 0) {
                    current = pattern << 4;
                } else {
                    current |=  pattern;
                    _spiData.push_back(current);
                }

            }
        }
    }
    for (size_t i = 0; i < RESET_SIGNAL_IN_SPI_BYTES; i++) {
        _spiData.push_back(0);
    }
    _spi.write(_spiData);
    _spiData.clear();
}

#endif

// common part
Led::~Led() {
    _spi.close();
}

void Led::clear() {
    for (size_t i = 0; i < _rgbData.size(); i++) {
        _rgbData[i].clear();
    }
}

void Led::setColor(const WS2812B::RGB &color, int ledIndex) {
    _checkLedIndexRange(ledIndex);
    _rgbData[ledIndex] = color;
}

WS2812B::RGB &Led::getColor(int ledIndex) {
    _checkLedIndexRange(ledIndex);
    return _rgbData[ledIndex];
}

void Led::_checkLedIndexRange(int ledIndex) {
    if (ledIndex < 0) throw std::invalid_argument("ledIndex should be >= 0");
    if (ledIndex > _numLeds) throw std::invalid_argument("ledIndex should be < numLeds passed to the constructor");
}

