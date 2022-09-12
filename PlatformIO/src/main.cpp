/**
 * @file main.cpp
 * @author Miura Daiki
 * @brief 姿勢クォータニオンをOSCで送信する
 * @version 0.1
 * @date 2022-07-13
 *
 * @copyright Copyright (c) 2022 Miura Daiki
 *
 * 改変元: "https://github.com/naninunenoy/AxisOrange.git"
 * 改変箇所: コメント追加 / BTSPP -> OSC に変更 / 他
 * Copyright (c) 2019 Nakano Yosuke
 * MIT (https://opensource.org/licenses/mit-license.php)
 */

#include <M5Unified.h>
#include <ArduinoOSCWiFi.h>
#include "imu/ImuReader.h"
#include "imu/AverageCalc.h"
#include "prefs/Settings.h"

#define TASK_DEFAULT_CORE_ID 1
#define TASK_STACK_DEPTH 4096UL
#define TASK_NAME_IMU "IMUTask"
#define TASK_NAME_SEND_OSC "SendOscTask"
#define TASK_NAME_RECEIVE_OSC "ReceiveOscTask"
#define TASK_NAME_NOTIFY "ReceiveOscTask"
#define TASK_SLEEP_IMU 5           // = 1000[ms] / 200[Hz]
#define TASK_SLEEP_SEND_OSC 40     // = 1000[ms] / 20[Hz]
#define TASK_SLEEP_RECEIVE_OSC 100 // = 1000[ms] / 10[Hz]
#define TASK_SLEEP_NOTIFY 100      // = 1000[ms] / 10[Hz]
#define MUTEX_DEFAULT_WAIT 1000UL  // 1000ms ESP32のFreeRTOSでは 1TICK=1ms

static void ImuLoop(void *arg);
static void SendOscLoop(void *arg);
static void ReceiveOscLoop(void *arg);
static void NotifyLoop(void *arg);

TaskHandle_t taskHandle;

imu::ImuReader *imuReader;
imu::ImuData imuData;
static SemaphoreHandle_t imuDataMutex = NULL;

bool gyroOffsetInstalled = true;
imu::AverageCalcXYZ gyroAve;
prefs::Settings settingPref;

String host_ip = "10.0.0.116";
const int bind_port = 22222;
const int send_port = 33333;

String uniqueId = "default";
String quat_addr = "/quat";

/**
 * @brief Lcdの描画を更新する
 */
void UpdateLcd()
{
  M5.Lcd.setCursor(0, 0);
  if (gyroOffsetInstalled)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.println("Kaiten-Boh\n");
    M5.Lcd.print("IP : ");
    M5.Lcd.println(WiFi.localIP());
    M5.Lcd.print("UniqueID : ");
    M5.Lcd.println(uniqueId);
  }
  else
  {
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.println("GyroOffset");
  }
}

/**
 * @brief WiFiに接続する．
 * @brief WiFiの接続に成功するまで試行する． 30回接続に失敗した場合は本体を再起動する．
 */
void ConnectWiFi()
{

#ifdef ESP_PLATFORM
  WiFi.disconnect(true, true); // disable wifi, erase ap info
  delay(1000);
  WiFi.mode(WIFI_STA);
#endif
  M5.Lcd.fillScreen(RED);
  M5.Lcd.println("Wifi Connecting");

  // 前回接続時情報で接続する
  Serial.println("WiFi begin");
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED)
  {
    M5.Lcd.print(".");
    delay(500);
    // 10秒以上接続できなかったら抜ける
    if (10000 < millis())
      break;
  }
  M5.Lcd.fillScreen(PURPLE);
  M5.Lcd.setCursor(0, 0);

  // 未接続の場合にはSmartConfig待受
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig(   // start smart config
        SC_TYPE_ESPTOUCH_V2, // smart config type
        "1234567890123456"); // AES Key (128 bit)

    M5.Lcd.println("Waiting for SmartConfig");
    while (!WiFi.smartConfigDone())
    {
      delay(500);
      M5.Lcd.print("#");
      // 30秒以上接続できなかったら抜ける
      if (30000 < millis())
      {
        M5.Lcd.println("");
        M5.Lcd.println("Reset");
        ESP.restart();
      }
    }
    M5.Lcd.fillScreen(ORANGE);
    M5.Lcd.setCursor(0, 0);

    // Wi-fi接続
    M5.Lcd.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      M5.Lcd.print(".");
      // 60秒以上接続できなかったら抜ける
      if (60000 < millis())
      {
        Serial.println("");
        M5.Lcd.println("");
        Serial.println("Reset");
        ESP.restart();
      }
    }
    Serial.println("");
    Serial.println("WiFi Connected.");
  }

  UpdateLcd();
}

void setup()
{
  // Initialize
  M5.begin();
  Serial.begin(115200);

  // read settings
  float gyroOffset[3] = {0.0F};
  settingPref.begin();
  // settingPref.clear();
  settingPref.readGyroOffset(gyroOffset);
  settingPref.readUniqueId(uniqueId);
  settingPref.finish();

  // lcd
  M5.Lcd.setRotation(3);
  M5.Lcd.setFont(&fonts::Font2); // Font Size (8, 16) px
  M5.Lcd.setTextSize(1);         // Font size *1
  M5.Lcd.setTextColor(TFT_WHITE);

  // wifiに接続する
  ConnectWiFi();

  // imu
  imuReader = new imu::ImuReader(M5.Imu);
  imuReader->initialize();
  if (gyroOffsetInstalled)
    imuReader->writeGyroOffset(gyroOffset[0], gyroOffset[1], gyroOffset[2]);

  OscWiFi.subscribe(bind_port, "/set/hostip",
                    [&](String &s)
                    {
                      xTaskNotify(taskHandle, 0, eNoAction);
                      host_ip = s;
                      UpdateLcd();
                    });

  OscWiFi.subscribe(bind_port, "/set/offset",
                    []()
                    {
                      xTaskNotify(taskHandle, 0, eNoAction);
                      gyroOffsetInstalled = false;
                      // imuReader->writeGyroOffset(0.0F, 0.0F, 0.0F);
                      UpdateLcd();
                    });

  OscWiFi.subscribe(bind_port, "/set/uniqueid",
                    [&](String &s)
                    {
                      xTaskNotify(taskHandle, 0, eNoAction);
                      uniqueId = s;
                      settingPref.begin();
                      settingPref.writeUniqueId(uniqueId);
                      settingPref.finish();
                      UpdateLcd();
                    });

  OscWiFi.subscribe(bind_port, "/reboot",
                    []()
                    {
                      xTaskNotify(taskHandle, 0, eNoAction);
                      ESP.restart();
                    });

  // task
  imuDataMutex = xSemaphoreCreateMutex();
  //! 指定したCPUコアでタスクを起動する
  xTaskCreatePinnedToCore(ImuLoop, TASK_NAME_IMU, TASK_STACK_DEPTH,
                          NULL, 2, NULL, TASK_DEFAULT_CORE_ID);
  xTaskCreatePinnedToCore(SendOscLoop, TASK_NAME_SEND_OSC, TASK_STACK_DEPTH,
                          NULL, 1, NULL, TASK_DEFAULT_CORE_ID);
  xTaskCreatePinnedToCore(ReceiveOscLoop, TASK_NAME_RECEIVE_OSC, TASK_STACK_DEPTH,
                          NULL, 1, NULL, TASK_DEFAULT_CORE_ID);
  xTaskCreatePinnedToCore(NotifyLoop, TASK_NAME_NOTIFY, TASK_STACK_DEPTH,
                          NULL, 1, &taskHandle, 0);
}

void loop() {}

static void ImuLoop(void *arg)
{
  while (1)
  {
    uint32_t entryTime = millis();
    if (xSemaphoreTake(imuDataMutex, MUTEX_DEFAULT_WAIT) == pdTRUE)
    {
      imuReader->update();
      imuReader->read(imuData);
      if (!gyroOffsetInstalled)
      {
        if (!gyroAve.push(imuData.gyro[0], imuData.gyro[1], imuData.gyro[2]))
        {
          float x = gyroAve.averageX();
          float y = gyroAve.averageY();
          float z = gyroAve.averageZ();
          // set offset
          imuReader->writeGyroOffset(x, y, z);
          // save offset
          float offset[] = {x, y, z};
          settingPref.begin();
          settingPref.writeGyroOffset(offset);
          settingPref.finish();
          gyroOffsetInstalled = true;
          gyroAve.reset();
          UpdateLcd();
        }
      }
    }
    xSemaphoreGive(imuDataMutex);
    // idle
    int32_t sleep = TASK_SLEEP_IMU - (millis() - entryTime);
    vTaskDelay((sleep > 0) ? sleep : 0);
  }
}

static void SendOscLoop(void *arg)
{
  while (1)
  {
    uint32_t entryTime = millis();
    if (gyroOffsetInstalled)
    {
      if (xSemaphoreTake(imuDataMutex, MUTEX_DEFAULT_WAIT) == pdTRUE)
      {
        String addr = "/" + uniqueId + quat_addr;
        OscWiFi.send(host_ip, send_port, addr,
                     imuData.quat[0],
                     imuData.quat[1],
                     imuData.quat[2],
                     imuData.quat[3]);
      }
      xSemaphoreGive(imuDataMutex);
    }

    // idle
    int32_t sleep = TASK_SLEEP_SEND_OSC - (millis() - entryTime);
    vTaskDelay((sleep > 0) ? sleep : 0);
  }
}

static void ReceiveOscLoop(void *arg)
{
  while (1)
  {
    uint32_t entryTime = millis();

    // osc 受け取る
    // ジャイロオフセットのアドレスだったらオフセットする
    // ユニークID変更のアドレスだったら変更する
    OscWiFi.update();

    // idle
    int32_t sleep = TASK_SLEEP_RECEIVE_OSC - (millis() - entryTime);
    vTaskDelay((sleep > 0) ? sleep : 0);
  }
}

static void NotifyLoop(void *arg)
{
  while (1)
  {
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

    digitalWrite(GPIO_NUM_10, LOW);
    vTaskDelay(10);
    digitalWrite(GPIO_NUM_10, HIGH);

    vTaskDelay(1);
  }
}