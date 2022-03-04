#include <Arduino.h>
#include "Function.h"

void Task_One(void *pvParameters)
{
  Serial.println("");
  sensorTemp.begin();
  cekFile_dataAkumulasi();
  cekFile_dataTotal();
  initialize();
  Blynk.begin(auth, ssid, pass, server, 8080);
  setSyncInterval(10 * 60);
  timer.setInterval(1000, BlynkFunction);
  Serial.println("");
  for (;;)
  {
    Blynk.run();
    timer.run();
  }
}

void Task_Two(void *pvParameters)
{
  static const char *path = "/total.json";
  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH);
  pulseCount = 0;
  debit = 0.0;
  flowmlt = 0;
  oldTime = 0;

  attachInterrupt(digitalPinToInterrupt(flowsensor), pulseCounter, FALLING);

  button.attachClick(singgelClick);
  button.setPressTicks(1000);
  button.attachLongPressStart(longClick);
  button.attachMultiClick(multiClick);
  lcd.init();
  lcd.backlight();
  for (;;)
  {
    if ((millis() - oldTime) > 1000)
    {
      detachInterrupt(digitalPinToInterrupt(flowsensor));
      String output;
      StaticJsonDocument<120> doc;
      debit = ((1000.0 / (millis() - oldTime)) * pulseCount) / konstanta;
      oldTime = millis();
      flowmlt = (debit / 60) * 1000.0;
      pembaca.total += flowmlt;
      satuminggu.total += flowmlt;
      doc["total"] = pembaca.total;
      doc["SatuMinggu"] = satuminggu.total;
      pulseCount = 0;

      serializeJson(doc, output);
      file = LITTLEFS.open(path, FILE_WRITE);
      file.print(output);
      file.close();

      attachInterrupt(digitalPinToInterrupt(flowsensor), pulseCounter, FALLING);
    }

    if(millis() - myWaktu >= 10000)
    {
      myWaktu = millis();
      clickable ++;
      if(clickable > 2) clickable = 0;
    }

    if(clickable == 0)
    {
      printLCD_info();
    }else if(clickable == 1){
      printLCD_waktu();
    }else if(clickable == 2){
      printDebit();
    }

    button.tick();
    mulai_record();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  filesystem_begin();

  xTaskCreatePinnedToCore(
      Task_One,
      "TaskBlynk",
      7500,
      NULL,
      3,
      &Task1,
      1);

  xTaskCreatePinnedToCore(
      Task_Two,
      "Flow Sensor",
      7500,
      NULL,
      2,
      &Task2,
      0);

  Serial.println();
}

void loop() {}