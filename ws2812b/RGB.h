//
// Created by Alexander Galilov on 26.05.2019.
// E-Mail: alexander.galilov@gmail.com 

#ifndef RGB_H
#define RGB_H

namespace WS2812B {

    struct RGB {
        unsigned char _r, _g, _b;

        RGB(unsigned char r, unsigned char g, unsigned char b) :
                _r(r), _g(g), _b(b) {
            // empty
        }

        RGB() : _r(0), _g(0), _b(0) {
            // empty
        }

        void clear() {
            _r = 0;
            _g = 0;
            _b = 0;
        }

        RGB &set(unsigned char r, unsigned char g, unsigned char b) {
            _r = r;
            _g = g;
            _b = b;
            return *this;
        }

        RGB &setR(unsigned char r) {
            _r = r;
            return *this;
        }

        RGB &setG(unsigned char g) {
            _g = g;
            return *this;
        }

        RGB &setB(unsigned char b) {
            _b = b;
            return *this;
        }
    };
}

#endif //RGB_H
