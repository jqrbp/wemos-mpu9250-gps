#include "wifiUtils.h"
#include "webUtils.h"
#include "fileUtils.h"
#include "imuUtils.h"
#include "gpsUtilsWrapper.h"
#include "serialUtils.h"

static const int ledPin = 2;
unsigned long blinkTime = 0;

void setup()
{
  pinMode(ledPin, OUTPUT);
  serial_begin(115200);

  imu_setup();

  gpsUtilsWrapper_begin();
  
  fs_setup();
  wifi_setup();
  web_setup();
}

void loop()
{
  if (millis() - blinkTime > 500) {
    blinkTime = millis();
    digitalWrite(ledPin, !digitalRead(ledPin));
  }

  imu_loop();
  serial_loop();
  
  gpsUtilsWrapper_loop();
  wifi_loop();
  web_loop();
}
