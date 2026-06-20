#include <Arduino.h>
#ifdef M5STACK
#include <M5Unified.h>
#endif

#include "app/BleCliApp.h"

static BleCliApp g_app;

void setup() {
  delay(2000);
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
    delay(10);
  }

#ifdef M5STACK
  M5.begin();
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("BLEcli", M5.Display.width() / 2, M5.Display.height() / 2);
#else
  Serial.println("BLEcli starting...");
#endif

  g_app.begin();
}

void loop() {
#ifdef M5STACK  
  M5.update();
#endif  
  g_app.loop();
}
