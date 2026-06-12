#include <Arduino.h>
#include <M5Unified.h>

#include "app/BleCliApp.h"

static BleCliApp g_app;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
    delay(10);
  }

  M5.begin();
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("BLECLI", M5.Display.width() / 2, M5.Display.height() / 2);

  g_app.begin();
}

void loop() {
  M5.update();
  g_app.loop();
}
