//
// Created by Alexander Galilov on 02.06.2019.
// E-Mail: alexander.galilov@gmail.com 

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <vector>

class SPI {
public:
    SPI();
    bool isOpened();
    void open(int channel, int speed, int mode);
    void close();
    void write(const std::vector<unsigned char>& data);
    virtual ~SPI();

private:
    static const char *_spiDevTemplate;
    static const uint8_t _spiBPW;
    //static const uint16_t _spiDelay;
    int _fd;
};


#endif //SPI_H
