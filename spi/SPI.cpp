//
// Created by Alexander Galilov on 02.06.2019.
// E-Mail: alexander.galilov@gmail.com
//
// some code below is based on https://git.drogon.net/?p=wiringPi, (c) by Gordon Henderson
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdexcept>
#include <bits/ios_base.h>
#include <cstring>
#include "SPI.h"

const char *SPI::_spiDevTemplate = "/dev/spidev0.%d";
const uint8_t SPI::_spiBPW = 8;
//const uint16_t SPI::_spiDelay = 0;

SPI::SPI() : _fd(-1) {

}

bool SPI::isOpened() {
    return _fd != -1;
};

void SPI::open(int channel, int speed, int mode) {
    if (_fd != -1)
        throw std::ios_base::failure("SPI is already opened");
    if (mode < 0 || mode > 3)
        throw std::invalid_argument("Wrong 'mode' parameter to open SPI channel");
    char spiDev[32];
    ::snprintf(spiDev, sizeof(spiDev), _spiDevTemplate, channel);
    if ((_fd = ::open(spiDev, O_RDWR)) < 0)
        throw std::ios_base::failure(::strerror(errno));
    if (::ioctl(_fd, SPI_IOC_WR_MODE, &mode) < 0)
        throw std::ios_base::failure(::strerror(errno));

    if (::ioctl(_fd, SPI_IOC_WR_BITS_PER_WORD, &_spiBPW) < 0)
        throw std::ios_base::failure(::strerror(errno));

    if (::ioctl(_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
        throw std::ios_base::failure(::strerror(errno));

}

void SPI::close() {
    if (_fd != -1) {
        ::close(_fd);
        _fd = -1;
    }
}

void SPI::write(const std::vector<unsigned char>& data) {
    ::write(_fd, &data[0], data.size());
}

SPI::~SPI() {
    close();
}