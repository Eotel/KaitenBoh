#pragma once

#include <Preferences.h>

namespace prefs
{

    static const char *PrefNameSpaceKey = "osc_pos_sender"; // max 15 chars
    static const char *PrefDataKey_gyroOffsetX = "gyro_offset_x";
    static const char *PrefDataKey_gyroOffsetY = "gyro_offset_y";
    static const char *PrefDataKey_gyroOffsetZ = "gyro_offset_z";
    static const char *PrefDataKey_uniqueId = "unique_id";

    class Settings
    {
    public:
        explicit Settings();
        ~Settings();
        void begin();
        void clear();
        void finish();
        void writeGyroOffset(const float *gyroOffset);
        bool readGyroOffset(float *gyroOffset);
        void writeUniqueId(const String &uniqueId);
        bool readUniqueId(String &uniqueId);

    private:
        Preferences preferences;
    }; // ButtonCheck

} // prefs
