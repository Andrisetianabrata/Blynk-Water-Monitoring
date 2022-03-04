#include <Arduino.h>
#include "variable.h"

void filesystem_begin()
{
  if (!LITTLEFS.begin(true))
  {
    Serial.println("LITTLEFS Mount Failed");
    return;
  }
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V6);
  rtc.begin();
}

BLYNK_WRITE(V4)
{
  int tombol_blynk = param.asInt();
  BlynkSelenoidState = tombol_blynk;
}

BLYNK_WRITE(V6)
{
  int tombol_blynk = param.asInt();
  emergencyStop = tombol_blynk;
}

void seting_ultrasonic(byte pintrig, byte pinecho)
{
  pinMode(pintrig, OUTPUT);
  pinMode(pinecho, INPUT);
}

void initialize()
{
  ultrasonic1.trig = 12;
  ultrasonic1.echo = 13;
  ultrasonic2.trig = 27;
  ultrasonic2.echo = 14;
  ultrasonic3.trig = 25;
  ultrasonic3.echo = 26;
  pinMode(Rainsensor, INPUT);
  pinMode(Selenoid_1, OUTPUT);
  seting_ultrasonic(ultrasonic1.trig, ultrasonic1.echo);
  seting_ultrasonic(ultrasonic2.trig, ultrasonic2.echo);
  seting_ultrasonic(ultrasonic3.trig, ultrasonic3.echo);
}

float tulisUltrasonic(byte triger, byte echo)
{
  digitalWrite(triger, LOW);
  delayMicroseconds(2);
  digitalWrite(triger, HIGH);
  delayMicroseconds(10);
  digitalWrite(triger, LOW);

  duration = pulseIn(echo, HIGH);
  distance = (duration * 0.0343) / 2;
  return distance;
}

void eventKamarMandi()
{
  // ButtonState = digitalRead(ButtonSelenoid);
  levelPnmpngn = 70;
  levelBakMandi = tulisUltrasonic(ultrasonic1.trig, ultrasonic1.echo);

  if (levelBakMandi > 3 && levelBakMandi < 20 && selenoid)
  {
    Blynk.notify("Bak Mandi Telah Terisi PENUH!");
  }

  if (!emergencyStop)
  {
    if (levelPnmpngn < 80 && (levelBakMandi > 80 && levelBakMandi < 500) && !rainTriger)
    {
      selenoid1 = true;
    }
    else if (levelBakMandi < 20)
    {
      selenoid1 = false;
      if (BlynkSelenoidState)
      {
        Blynk.virtualWrite(V4, 0);
        BlynkSelenoidState = 0;
      }
    }

    if (selenoid1)
    {
      selenoid2 = true;
    }
    else if (!selenoid1)
    {
      selenoid2 = !true;
    }

    if (selenoid2 || BlynkSelenoidState)
    {
      selenoid = true;
    }
    else if (!selenoid2)
    {
      selenoid = !true;
    }
  }
  else
  {
    selenoid = !true;
    Blynk.virtualWrite(V4, 0);
    BlynkSelenoidState = 0;
  }
}

void BlynkFunction()
{
  sensorTemp.requestTemperatures();
  float suhu = sensorTemp.getTempCByIndex(0);
  eventKamarMandi();

  if (selenoid)
  {
    SelenoidLED.on();
  }
  else
  {
    SelenoidLED.off();
  }
  Blynk.virtualWrite(V0, tulisUltrasonic(ultrasonic1.trig, ultrasonic1.echo));
  Blynk.virtualWrite(V1, tulisUltrasonic(ultrasonic2.trig, ultrasonic2.echo));
  Blynk.virtualWrite(V2, tulisUltrasonic(ultrasonic3.trig, ultrasonic3.echo));
  Blynk.virtualWrite(V3, pembaca.total / 1000.0);
  Blynk.virtualWrite(V7, debit);
  Blynk.virtualWrite(V8, 23);
  // Serial.printf("Level Bak: %d\nSelenoid: %d\nEmergency: %d\nSuhu: %d\nDebit: %d\nVolume: %d\n", levelBakMandi, selenoid, emergencyStop, suhu, flowmlt, pembaca.total);
  // Serial.printf("Blynk Selenoid: %d\n", BlynkSelenoidState);
  // Serial.printf("jam: %2d:%2d:%2d Tanggal: %d/%d/%d Hari: %s\n\n", hour(), minute(), second(), day(), month(), year(), Hari[weekday() - 1]);
  // Serial.print("Debit air: ");
  // Serial.print(int(debit));
  // Serial.print("L/min");
  // Serial.print("\t");

  // Serial.print("Volume: ");
  // Serial.print(pembaca.total);
  // Serial.println("mL");

  // Serial.print("1 minggu: ");
  // Serial.print(satuminggu.total);
  // Serial.println("mL");
}

void longClick()
{
  emergencyStop = !emergencyStop;
  Blynk.virtualWrite(V6, emergencyStop);
}

void singgelClick()
{
  BlynkSelenoidState = !BlynkSelenoidState;
  Blynk.virtualWrite(V4, BlynkSelenoidState);
}

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void cekFile_dataAkumulasi()
{
  file = LITTLEFS.open("/akumulasi.json", "r");
  if (!file || file.isDirectory())
  {
    file.close();
    Serial.println("- failed to open file for reading");
    String output;
    StaticJsonDocument<1000> doc;
    doc["minggu"] = 0;
    doc["senin"] = 0;
    doc["selasa"] = 0;
    doc["rabu"] = 0;
    doc["kamis"] = 0;
    doc["jumat"] = 0;
    doc["sabtu"] = 0;
    serializeJson(doc, output);
    file = LITTLEFS.open("/akumulasi.json", FILE_WRITE);
    file.println(output);
    file.close();
    ESP.restart();
  }
  else
  {
    while (file.available())
    {
      String i = file.readString();
      Serial.println("Filenya: " + i);
      StaticJsonDocument<1000> doc;
      DeserializationError error = deserializeJson(doc, i);
      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      pembaca.minggu = doc["minggu"];
      pembaca.senin = doc["senin"];
      pembaca.selasa = doc["selasa"];
      pembaca.rabu = doc["rabu"];
      pembaca.kamis = doc["kamis"];
      pembaca.jumat = doc["jumat"];
      pembaca.sabtu = doc["sabtu"];
    }
    file.close();
  }
}

void cekFile_dataTotal()
{
  file = LITTLEFS.open("/total.json", "r");
  if (!file || file.isDirectory())
  {
    file.close();
    Serial.println("- failed to open file for reading");
    String output;
    StaticJsonDocument<1000> doc;
    doc["total"] = 0;
    doc["SatuMinggu"] = 0;
    serializeJson(doc, output);
    file = LITTLEFS.open("/total.json", FILE_WRITE);
    file.println(output);
    file.close();
    ESP.restart();
  }
  else
  {
    while (file.available())
    {
      String i = file.readString();
      Serial.println("Filenya: " + i);
      StaticJsonDocument<1000> doc;
      DeserializationError error = deserializeJson(doc, i);
      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      pembaca.total = doc["total"];
      satuminggu.total = doc["SatuMinggu"];
    }
    file.close();
    Serial.println(pembaca.total);
  }
}

void mulai_record()
{
  if (weekday() == 1 && mulaiJam)
  {
    pembaca.minggu = pembaca.total;
    // pembaca.total = 0
  }
  else if (weekday() == 2 && mulaiJam)
  {
    pembaca.senin = pembaca.total;
    // pembaca.total = 0
  }
  else if (weekday() == 3 && mulaiJam)
  {
    pembaca.selasa = pembaca.total;
    // pembaca.total = 0
  }
  else if (weekday() == 4 && mulaiJam)
  {
    pembaca.rabu = pembaca.total;
    // pembaca.total = 0
  }
  else if (weekday() == 5 && mulaiJam)
  {
    pembaca.kamis = pembaca.total;
    // pembaca.total = 0
  }
  else if (weekday() == 6 && mulaiJam)
  {
    pembaca.jumat = pembaca.total;
    // pembaca.total = 0
  }
  else if (weekday() == 7 && mulaiJam)
  {
    pembaca.sabtu = pembaca.total;
    // pembaca.total = 0
  }
  
  if(millis() - tampilanMillis >= 1000)
  {
    tampilanMillis = millis();
    Serial.printf("Level Bak: %d\nSelenoid: %d\nEmergency: %d\nSuhu: %d\nDebit: %d\nVolume: %d\n", levelBakMandi, mulaiJam, emergencyStop, 24, flowmlt, pembaca.total);
    Serial.printf("Blynk Selenoid: %d\n", BlynkSelenoidState);
    Serial.printf("jam: %2d:%2d:%2d Tanggal: %d/%d/%d Hari: %s\n\n", hour(), minute(), second(), day(), month(), year(), Hari[weekday() - 1]);
    Serial.print("Debit air: ");
    Serial.print(int(debit));
    Serial.print("L/min");
    Serial.print("\t");

    Serial.print("Volume: ");
    Serial.print(pembaca.total);
    Serial.println("mL");

    Serial.print("1 minggu: ");
    Serial.print(satuminggu.total);
    Serial.println("mL");
  }

  if (hour() == 13 && minute() == 27 && second() == 59)
  {
    mulaiJam = 1;
  }
  else
  {
    mulaiJam = 0;
  }

  if (mulaiJam)
  {
    String output;
    StaticJsonDocument<120> doc;
    doc["minggu"] = pembaca.minggu;
    doc["senin"] = pembaca.senin;
    doc["selasa"] = pembaca.selasa;
    doc["rabu"] = pembaca.rabu;
    doc["kamis"] = pembaca.kamis;
    doc["jumat"] = pembaca.jumat;
    doc["sabtu"] = pembaca.sabtu;
    serializeJson(doc, output);
    file = LITTLEFS.open("/akumulasi.json", FILE_WRITE);
    file.print(output);
    Serial.println(output);
    file.close();
    pembaca.total = 0;
  }
}
