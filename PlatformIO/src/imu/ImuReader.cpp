#include "ImuReader.h"

namespace imu
{
    /**
     * @brief Construct a new Imu Reader:: Imu Reader object
     *
     * @param m5 IMU＿Classのインスタンス
     */
    ImuReader::ImuReader(m5::IMU_Class &m5) : m5Imu(m5), ahrs(), imuData()
    {
        memset(gyroOffsets, 0, sizeof(float) * ImuXyz);
    }

    /**
     * @brief IMUインスタンスを初期化する
     *
     * @return true 正常終了
     * @return false 異常終了
     */
    bool ImuReader::initialize()
    {
        return m5Imu.begin();
    }

    /**
     * @brief ジャイロのオフセット値をメンバ変数に保存する
     *
     * @param x
     * @param y
     * @param z
     * @return true 正常終了
     * @return false 異常終了
     */
    bool ImuReader::writeGyroOffset(float x, float y, float z)
    {
        gyroOffsets[0] = x;
        gyroOffsets[1] = y;
        gyroOffsets[2] = z;
        return true;
    }

    /**
     * @brief 最新のIMUのデータを取得する
     *
     * @return true 正常終了
     * @return false 異常終了
     */
    bool ImuReader::update()
    {
        float &ax = imuData.acc[0];
        float &ay = imuData.acc[1];
        float &az = imuData.acc[2];
        float &gx = imuData.gyro[0];
        float &gy = imuData.gyro[1];
        float &gz = imuData.gyro[2];
        float &qw = imuData.quat[0];
        float &qx = imuData.quat[1];
        float &qy = imuData.quat[2];
        float &qz = imuData.quat[3];

        m5Imu.getAccel(&ax, &ay, &az);
        m5Imu.getGyro(&gx, &gy, &gz);

        gx -= gyroOffsets[0];
        gy -= gyroOffsets[1];
        gz -= gyroOffsets[2];

        /**
         * @brief AHRSアルゴリズムで姿勢のクォータニオンを取得する
         *
         */
        ahrs.UpdateQuaternion(
            gx * DEG_TO_RAD, gy * DEG_TO_RAD, gz * DEG_TO_RAD,
            ax, ay, az,
            qw, qx, qy, qz);
        imuData.timestamp = millis();
        lastUpdated = imuData.timestamp;
        return true;
    }

    /**
     * @brief
     *
     * @param outImuData 読み込んだIMUデータを保存する変数のアドレス
     * @return true 正常終了
     * @return false 異常終了 IMUデータの更新がない
     */
    bool ImuReader::read(ImuData &outImuData) const
    {
        if (lastUpdated == outImuData.timestamp)
        {
            return false; // not updated
        }
        memcpy(&outImuData, &imuData, ImuDataLen);
        return true;
    }

} // imu
