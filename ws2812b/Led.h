//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com 

#ifndef LED_H
#define LED_H

#include <stddef.h>
#include <vector>
#include "RGB.h"

namespace WS2812B {
    class Led {
    public:
        // Constructor
        // spiChannel -- can be used to select target device
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

        // Init SPI parameters
        void _hardwareInit();

        // Write data to SPI controller
        void _hardwareWriteData();

        // Close & cleanup SPI
        void _hardwareCleanup();
        void _checkLedIndexRange(int ledIndex);
    private:
        const int _spiChannel;
        const int _numLeds;
        int _hardwareFd;
        std::vector<RGB> _rgbData;
        std::vector<unsigned char> _spiData;
    };

}
#endif //W2812BSPI_H
