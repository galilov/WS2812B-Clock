//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com 
//
// To be sure it will work properly please do some changes:
// In /boot/cmdline.txt add spidev.bufsiz=65536. In my case the result is:
// dwc_otg.lpm_enable=0 console=serial0,115200 console=tty1 root=PARTUUID=da6e0af0-02 rootfstype=ext4 elevator=deadline fsck.repair=yes spidev.bufsiz=65536 rootwait
//
// In /boot/config.txt add the following line:
// core_freq=250
// It will fix the issue https://github.com/raspberrypi/linux/issues/2094
// I implemented workaround (for USE_SIMPLE_IMPLEMENTATION == false) and I hope this step is not required but...
#ifndef LED_H
#define LED_H

#include <stddef.h>
#include <vector>
#include "RGB.h"
#include "../spi/SPI.h"
// Set it to true when the line
// core_freq=250
// is added to /boot/config.txt
#define USE_SIMPLE_IMPLEMENTATION true

namespace WS2812B {
    class Led {
    public:
        // Constructor
        // spiChannel -- can be used to select target device. It is not used by LEDs but can be supported by
        //               external hardware.
        // numLeds -- the number of leds in chain/stripe/matrix/ring...
        Led(int spiChannel, int numLeds);

        // Destructor
        virtual ~Led();

        void clear();

        // Set the color of particular leds
        void setColor(const WS2812B::RGB &color, int ledIndex);

        WS2812B::RGB &getColor(int ledIndex);

        // Show/update lights
        void show();

    private:
        // disable copy constructor
        Led(const Led &);
        void _checkLedIndexRange(int ledIndex);
    private:
        const int _spiChannel;
        const int _numLeds;
        std::vector<RGB> _rgbData;
        std::vector<unsigned char> _spiData;
        SPI _spi;
    };

}
#endif //W2812BSPI_H
