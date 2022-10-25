#include "Settings.h"

namespace prefs
{

    Settings::Settings() {}
    Settings::~Settings() {}

    /**
     * @brief Open Preferences with my-app namespace
     */
    void Settings::begin()
    {
        preferences.begin(PrefNameSpaceKey, false);
    }

    /**
     * @brief Remove all preferences under opened namespace
     */
    void Settings::clear()
    {
        preferences.clear();
    }

    /**
     * @brief Close the Preferences
     */
    void Settings::finish()
    {
        preferences.end();
    }

    /**
     * @brief ジャイロのオフセット値を書き込む
     *
     * @param gyroOffset 書き込むジャイロのオフセット値配列のアドレス
     */
    void Settings::writeGyroOffset(const float *gyroOffset)
    {
        preferences.putFloat(PrefDataKey_gyroOffsetX, gyroOffset[0]);
        preferences.putFloat(PrefDataKey_gyroOffsetY, gyroOffset[1]);
        preferences.putFloat(PrefDataKey_gyroOffsetZ, gyroOffset[2]);
    }

    /**
     * @brief ジャイロのオフセット値を読み込む
     *
     * @param gyroOffset 読み込んだジャイロのオフセット値を格納する配列のアドレス
     * @return true 正常終了： nvs領域からオフセット値の取得に成功
     * @return false 異常終了: オフセット値が取得できずデフォルト値を返却した
     */
    bool Settings::readGyroOffset(float *gyroOffset)
    {
        float x = preferences.getFloat(PrefDataKey_gyroOffsetX, 0.0F);
        float y = preferences.getFloat(PrefDataKey_gyroOffsetY, 0.0F);
        float z = preferences.getFloat(PrefDataKey_gyroOffsetZ, 0.0F);
        gyroOffset[0] = x;
        gyroOffset[1] = y;
        gyroOffset[2] = z;
        return x != 0.0F || y != 0.0F || z != 0.0F;
    }

    void Settings::writeUniqueId(const String &uniqueId)
    {
        preferences.putString(PrefDataKey_uniqueId, uniqueId);
    }

    bool Settings::readUniqueId(String &uniqueId)
    {
        uniqueId = preferences.getString(PrefDataKey_uniqueId, "default");
        return uniqueId != "default";
    }

    void Settings::writeHostIp(const String &hostIp)
    {
        preferences.putString(PrefDataKey_hostIp, hostIp);
    }

    bool Settings::readHostIp(String &hostIp)
    {
        hostIp = preferences.getString(PrefDataKey_hostIp, "192.168.20.50");
        return hostIp != "192.168.20.50";
    }

} // prefs
