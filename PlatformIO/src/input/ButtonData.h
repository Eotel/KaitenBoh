#pragma once
#include <inttypes.h>

namespace input
{

    static const int ButtonDataLen = 5;

    struct ButtonData
    {
    public:
        uint32_t timestamp;
        uint8_t btnBits;

        explicit ButtonData() : timestamp(0), btnBits(0) {}
    };

} // input
