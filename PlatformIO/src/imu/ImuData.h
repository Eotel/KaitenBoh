#pragma once
#include <inttypes.h>
#include <Arduino.h>

namespace imu
{

    static const int ImuXyz = 3;
    static const int ImuWxyz = 4;
    static const int ImuDataLen = 44; // 4 + (4*3) + (4*3) + (4*4), float/uint_32t: 4byte

    struct ImuData
    {
    public:
        uint32_t timestamp;
        float acc[ImuXyz];
        float gyro[ImuXyz];
        float quat[ImuWxyz];

        explicit ImuData() : timestamp(0)
        {
            memset(this, 0, ImuDataLen);
            quat[0] = 1.0F;
        }
    };

} // imu
