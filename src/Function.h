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
  BakMandi.trig = 12;
  BakMandi.echo = 13;
  BakUtama.trig = 27;
  BakUtama.echo = 14;
  BakCadangan.trig = 25;
  BakCadangan.echo = 26;
  pinMode(Rainsensor, INPUT);
  pinMode(Selenoid_1, OUTPUT);
  seting_ultrasonic(BakMandi.trig, BakMandi.echo);
  seting_ultrasonic(BakUtama.trig, BakUtama.echo);
  seting_ultrasonic(BakCadangan.trig, BakCadangan.echo);
}

int penghitung(byte triger, byte echo)
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
  BakMandi.minimal = 70;   // dalam persen
  BakMandi.maksimal = 10;  // dalam persen
  BakMandi.levelBak = map(BakMandi.penghitung(BakMandi.trig, BakMandi.echo), BakMandi.minimal, BakMandi.maksimal, 0, 100); // konversi dari nilai minimal - nilai maksimal ke 0 - 100

  BakUtama.minimal = 70;   // dalam persen
  BakUtama.maksimal = 10;  // dalam persen
  BakUtama.levelBak = map(BakUtama.penghitung(BakUtama.trig, BakUtama.echo), BakUtama.minimal, BakUtama.maksimal, 0, 100); // konversi dari nilai minimal - nilai maksimal ke 0 - 100

  BakCadangan.minimal = 70;   // dalam persen
  BakCadangan.maksimal = 10;  // dalam persen
  BakCadangan.levelBak = map(BakCadangan.penghitung(BakCadangan.trig, BakCadangan.echo), BakCadangan.minimal, BakCadangan.maksimal, 0, 100); // konversi dari nilai minimal - nilai maksimal ke 0 - 100

  rainTriger = digitalRead(Rainsensor);
  
  if (BakMandi.levelBak > 0 && BakMandi.levelBak < (BakMandi.maksimal + 2) && selenoid)
  {
    Blynk.notify("Bak Mandi Telah Terisi PENUH!"); // nyalakan notifikasi
  }

  if (!emergencyStop)
  {
    if (BakUtama.levelBak > BakUtama.maksimal && (BakMandi.levelBak > BakMandi.minimal && BakMandi.levelBak < BakMandi.maksimal) && !rainTriger)
    {
      selenoid1 = true;
    }
    else if (BakMandi.levelBak < BakMandi.minimal)
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

  if (rainTriger)
  {
    sensorHujan.on();
  }
  else
  {
    sensorHujan.off();
  }
  
  Blynk.virtualWrite(V0, BakMandi.levelBak);
  Blynk.virtualWrite(V1, BakUtama.levelBak);
  Blynk.virtualWrite(V2, BakCadangan.levelBak);
  Blynk.virtualWrite(V3, pembaca.total / 1000.0);
  Blynk.virtualWrite(V7, debit);
  Blynk.virtualWrite(V8, 23);
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
    idIndex = 1;
  }
  else if (weekday() == 2 && mulaiJam)
  {
    pembaca.senin = pembaca.total;
    // pembaca.total = 0
    idIndex = 2;
  }
  else if (weekday() == 3 && mulaiJam)
  {
    pembaca.selasa = pembaca.total;
    // pembaca.total = 0
    idIndex = 3;
  }
  else if (weekday() == 4 && mulaiJam)
  {
    pembaca.rabu = pembaca.total;
    // pembaca.total = 0
    idIndex = 4;
  }
  else if (weekday() == 5 && mulaiJam)
  {
    pembaca.kamis = pembaca.total;
    // pembaca.total = 0
    idIndex = 5;
  }
  else if (weekday() == 6 && mulaiJam)
  {
    pembaca.jumat = pembaca.total;
    // pembaca.total = 0
    idIndex = 6;
  }
  else if (weekday() == 7 && mulaiJam)
  {
    pembaca.sabtu = pembaca.total;
    // pembaca.total = 0
    idIndex = 7;
  }
  
  if(millis() - tampilanMillis >= 1000)
  {
    tampilanMillis = millis();
    Serial.printf("Level Bak: %d\nSelenoid: %d\nEmergency: %d\nSuhu: %d\nDebit: %d\nVolume: %d\n", BakMandi.levelBak, mulaiJam, emergencyStop, 24, flowmlt, pembaca.total);
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

  if (hour() == 23 && minute() == 59 && second() == 59)
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
