//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com

#include <unistd.h>
#include <time.h>
#include <stdexcept>
#include <iostream>
#include <wiringPiSPI.h>
#include "Led.h"

using namespace WS2812B;

// Set bitrate to *about* 5 MHz.
#define SPI_SPEED           5000000

// see _hardwareInit() and https://github.com/raspberrypi/linux/issues/2094
double g_realSpiSpeed = 0;

//https://cdn.sparkfun.com/datasheets/BreakoutBoards/WS2812B.pdf
#define LONG_SIGNAL_uS    0.9
#define SHORT_SIGNAL_uS   0.35
#define RESET_SIGNAL_uS   70.0
#define uS_PER_SECOND     1000000.0
#define LONG_SIGNAL_IN_SPI_BITS     ((int)(LONG_SIGNAL_uS*g_realSpiSpeed/uS_PER_SECOND))
#define SHORT_SIGNAL_IN_SPI_BITS    ((int)(SHORT_SIGNAL_uS*g_realSpiSpeed/uS_PER_SECOND))
#define RESET_SIGNAL_IN_SPI_BITS    ((int)(RESET_SIGNAL_uS*g_realSpiSpeed/uS_PER_SECOND))


Led::Led(int spiChannel, int numLeds) :
        _spiChannel(spiChannel), _numLeds(numLeds), _hardwareFd(-1) {
    _hardwareInit();
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
    _hardwareWriteData();
    _spiData.clear();
}

void Led::_hardwareInit() {
    _hardwareFd = wiringPiSPISetup(_spiChannel, SPI_SPEED);
    // Some magic to fix https://github.com/raspberrypi/linux/issues/2094
    if (g_realSpiSpeed == 0) {
        const auto bufSize = 16000;
        const double blockTransferApproxTime = bufSize * 8.0 / SPI_SPEED;
        const auto nIterations = (size_t)(1.0 / blockTransferApproxTime);
        auto *data = (unsigned char *) calloc(1, bufSize);
        ::timespec t0, t1;
        ::clock_gettime(CLOCK_REALTIME, &t0); //CLOCK_PROCESS_CPUTIME_ID does not work :(
        for (int i = 0; i < nIterations; i++) {
            ::wiringPiSPIDataRW(_spiChannel, data, bufSize);
        }
        ::clock_gettime(CLOCK_REALTIME, &t1);
        ::free(data);
        long long nSecT0 = t0.tv_sec * 1000000000LL + t0.tv_nsec;
        long long nSecT1 = t1.tv_sec * 1000000000LL + t1.tv_nsec;
        g_realSpiSpeed = nIterations * 8.0 * bufSize * 1000000000L / (nSecT1 - nSecT0);
        //printf("g_realSpeed: %f bits per second\n", g_realSpiSpeed);
    }
}

void Led::_hardwareWriteData() {
    if (_hardwareFd == -1) {
        throw std::ios_base::failure("Use _hardwareInit() to open SPI channel");
    }
    wiringPiSPIDataRW(_spiChannel, &_spiData[0], _spiData.size());
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
