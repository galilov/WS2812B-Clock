//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com

#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <wiringPiSPI.h>
#include "Led.h"

using namespace WS2812B;

// Set SCLK signal to 20 MHz. This will be our bit rate, Precision is 50 nS for WS2812B
#define SPI_SPEED           20000000

//https://cdn.sparkfun.com/datasheets/BreakoutBoards/WS2812B.pdf
#define LONG_SIGNAL_uS    0.9
#define SHORT_SIGNAL_uS   0.35
#define RESET_SIGNAL_uS   70.0
#define uS_PER_SECOND     1000000.0
#define LONG_SIGNAL_IN_SPI_BITS     (LONG_SIGNAL_uS*SPI_SPEED/uS_PER_SECOND)
#define SHORT_SIGNAL_IN_SPI_BITS    (SHORT_SIGNAL_uS*SPI_SPEED/uS_PER_SECOND)
#define RESET_SIGNAL_IN_SPI_BITS    (RESET_SIGNAL_uS*SPI_SPEED/uS_PER_SECOND)


Led::Led(int spiChannel, int numLeds) :
        _spiChannel(spiChannel), _numLeds(numLeds), _hardwareFd(-1) {
    // allocate RGB data
    _rgbData.reserve(numLeds);
    for (int i = 0; i < numLeds; i++) {
        _rgbData.emplace_back(RGB());
    }
    // allocate SPI buffer
    _spiData.reserve((size_t)
                             ((LONG_SIGNAL_IN_SPI_BITS + SHORT_SIGNAL_IN_SPI_BITS) * 3 * _numLeds +
                              RESET_SIGNAL_IN_SPI_BITS));
    _hardwareInit();
}

Led::~Led() {
    _hardwareCleanup();
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

void Led::show() {
    unsigned char current = 0;
    for (auto const &srcRGBValue: _rgbData) {
        unsigned char grb[] = {srcRGBValue._g, srcRGBValue._r, srcRGBValue._b};
        for (auto const &srcValue:grb) {
            for (size_t srcBitIndex = 0; srcBitIndex < 8; srcBitIndex++) {
                size_t srcBitValue = (srcValue << srcBitIndex) & 0x080U;
                int firstSemiWave = (int) (srcBitValue != 0 ? LONG_SIGNAL_IN_SPI_BITS : SHORT_SIGNAL_IN_SPI_BITS);
                for (size_t i = 0; i < LONG_SIGNAL_IN_SPI_BITS + SHORT_SIGNAL_IN_SPI_BITS; i++) {
                    unsigned char targetBit = (firstSemiWave-- > 0);
                    size_t bitIndex = 7 - (i % 8);
                    if (targetBit) {
                        current |= (1U << bitIndex);
                    }
                    if (bitIndex == 0) {
                        _spiData.push_back(current);
                        current = 0;
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < RESET_SIGNAL_IN_SPI_BITS; i++) {
        _spiData.push_back(0);
    }
    _hardwareWriteData();
    _spiData.clear();
}

void Led::_hardwareInit() {
    _hardwareFd = wiringPiSPISetup(_spiChannel, SPI_SPEED);
}

void Led::_hardwareWriteData() {
    if (_hardwareFd == -1) {
        throw std::ios_base::failure("Use _hardwareInit() to open SPI channel");
    }
    size_t nBytesWritten = ::write(_hardwareFd, &_spiData[0], _spiData.size());
    if (nBytesWritten != _spiData.size()) {
        throw std::ios_base::failure("SPI data write error");
    }
}

void Led::_hardwareCleanup() {
    if (_hardwareFd != -1) {
        ::close(_hardwareFd);
        _hardwareFd = -1;
    }
}

void Led::_checkLedIndexRange(int ledIndex) {
    if (ledIndex < 0) throw std::invalid_argument("ledIndex should be >= 0");
    if (ledIndex > _numLeds) throw std::invalid_argument("ledIndex should be < numLeds passed to the constructor");
}
