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

// BLYNK_WRITE(V11)
// {
//   int tombol_blynk = param.asInt();
//   RST = tombol_blynk;
//   // if(RST) waktuReset = millis();
// }

void seting_ultrasonic(byte pintrig, byte pinecho)
{
  pinMode(pintrig, OUTPUT);
  pinMode(pinecho, INPUT);
}

void initialize()
{
  BakMandi.trig = 12;    // PIN
  BakMandi.echo = 13;    // PIN
  BakUtama.trig = 27;    // PIN
  BakUtama.echo = 14;    // PIN
  BakCadangan.trig = 25; // PIN
  BakCadangan.echo = 26; // PIN
  pinMode(Rainsensor, INPUT);
  pinMode(Selenoid_1, OUTPUT);
  digitalWrite(Selenoid_1, HIGH);
  seting_ultrasonic(BakMandi.trig, BakMandi.echo);
  seting_ultrasonic(BakUtama.trig, BakUtama.echo);
  seting_ultrasonic(BakCadangan.trig, BakCadangan.echo);
}

void konversi()
{
  BakMandi.minimal = 60;  // dalam centimeter
  BakMandi.maksimal = 15; // dalam centimeter
  BakMandi.persenMinimal = 10;
  BakMandi.persenMaksimal = 80;
  BakMandi.levelBak = map(BakMandi.penghitung(BakMandi.trig, BakMandi.echo), BakMandi.minimal, BakMandi.maksimal, 0, 100); // konversi dari nilai minimal - nilai maksimal ke 0 - 100 memungkinkan untuk mengubah ke persentase
  // BakMandi.levelBak = BakMandi.penghitung(BakMandi.trig, BakMandi.echo); // konversi dari nilai minimal - nilai maksimal ke 0 - 100 memungkinkan untuk mengubah ke persentase

  BakUtama.minimal = 60;  // dalam centimeter
  BakUtama.maksimal = 15; // dalam centimeter
  BakUtama.persenMinimal = 10;
  BakUtama.persenMaksimal = 80;
  BakUtama.levelBak = map(BakUtama.penghitung(BakUtama.trig, BakUtama.echo), BakUtama.minimal, BakUtama.maksimal, 0, 100); // konversi dari nilai minimal - nilai maksimal ke 0 - 100 memungkinkan untuk mengubah ke persentase
  // BakUtama.levelBak = BakUtama.penghitung(BakUtama.trig, BakUtama.echo); // konversi dari nilai minimal - nilai maksimal ke 0 - 100 memungkinkan untuk mengubah ke persentase

  BakCadangan.minimal = 60;  // dalam centimeter
  BakCadangan.maksimal = 15; // dalam centimeter
  BakCadangan.persenMinimal = 10;
  BakCadangan.persenMaksimal = 80;
  BakCadangan.levelBak = map(BakCadangan.penghitung(BakCadangan.trig, BakCadangan.echo), BakCadangan.minimal, BakCadangan.maksimal, 0, 100); // konversi dari nilai minimal - nilai maksimal ke 0 - 100 memungkinkan untuk mengubah ke persentase
  // BakCadangan.levelBak = BakCadangan.penghitung(BakCadangan.trig, BakCadangan.echo); // konversi dari nilai minimal - nilai maksimal ke 0 - 100 memungkinkan untuk mengubah ke persentase
}

void eventKamarMandi()
{
  konversi();
  rainTriger = digitalRead(Rainsensor);

  if (BakMandi.levelBak > 0 && BakMandi.levelBak < BakMandi.persenMaksimal && selenoid)
  {
    Blynk.notify("Bak Mandi Telah Terisi PENUH!"); // nyalakan notifikasi
  }

  if (!emergencyStop)
  {
    if (BakUtama.levelBak > 80 && BakMandi.levelBak < 20 && !rainTriger)
    {
      selenoid1 = true;
    }
    else if (BakMandi.levelBak > 80)
    {
      if (BlynkSelenoidState)
      {
        BlynkSelenoidState = false;
        Blynk.virtualWrite(V4, 0);
      }
      selenoid1 = false;
    }

    if (selenoid1 || BlynkSelenoidState)
    {
      selenoid = true;
    }
    else
    {
      selenoid = false;
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
  suhu = sensorTemp.getTempCByIndex(0);
  eventKamarMandi();

  if (selenoid)
  {
    SelenoidLED.on();
  }
  else
  {
    SelenoidLED.off();
  }

  if (!rainTriger)
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
  Blynk.virtualWrite(V8, suhu);
  digitalWrite(Selenoid_1, !selenoid);
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

void multiClick()
{
  if (button.getNumberClicks() == 3)
  {
    clickable++;
    if (clickable > 2)
      clickable = 0;
    myWaktu = millis();
  }
  else if (button.getNumberClicks() == 10)
  {
    RST = true;
  }
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
      idIndex = doc["index"];
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
    satuminggu.total = 0;
    // idIndex = 1;
    forTable = Hari[weekday() - 1];
    forTable += " ";
    forTable += String(day());
    forTable += " ";
    forTable += String(month());
    forTable += " ";
    forTable += String(year());
    forTable += "=";
    table_value = pembaca.minggu;
  }
  else if (weekday() == 2 && mulaiJam)
  {
    pembaca.senin = pembaca.total;
    // pembaca.total = 0
    // idIndex = 2;
    forTable = Hari[weekday() - 1];
    forTable += " ";
    forTable += String(day());
    forTable += " ";
    forTable += String(month());
    forTable += " ";
    forTable += String(year());
    forTable += "=";
    table_value = pembaca.senin;
  }
  else if (weekday() == 3 && mulaiJam)
  {
    pembaca.selasa = pembaca.total;
    // pembaca.total = 0
    // idIndex = 3;
    forTable = Hari[weekday() - 1];
    forTable += " ";
    forTable += String(day());
    forTable += " ";
    forTable += String(month());
    forTable += " ";
    forTable += String(year());
    forTable += "=";
    table_value = pembaca.selasa;
  }
  else if (weekday() == 4 && mulaiJam)
  {
    pembaca.rabu = pembaca.total;
    // pembaca.total = 0
    // idIndex = 4;
    forTable = Hari[weekday() - 1];
    forTable += " ";
    forTable += String(day());
    forTable += " ";
    forTable += String(month());
    forTable += " ";
    forTable += String(year());
    forTable += "=";
    table_value = pembaca.rabu;
  }
  else if (weekday() == 5 && mulaiJam)
  {
    pembaca.kamis = pembaca.total;
    // pembaca.total = 0
    // idIndex = 5;
    forTable = Hari[weekday() - 1];
    forTable += " ";
    forTable += String(day());
    forTable += " ";
    forTable += String(month());
    forTable += " ";
    forTable += String(year());
    forTable += "=";
    table_value = pembaca.kamis;
  }
  else if (weekday() == 6 && mulaiJam)
  {
    pembaca.jumat = pembaca.total;
    // pembaca.total = 0
    // idIndex = 6;
    forTable = Hari[weekday() - 1];
    forTable += " ";
    forTable += String(day());
    forTable += " ";
    forTable += String(month());
    forTable += " ";
    forTable += String(year());
    forTable += "=";
    table_value = pembaca.jumat;
  }
  else if (weekday() == 7 && mulaiJam)
  {
    pembaca.sabtu = pembaca.total;
    // pembaca.total = 0
    // idIndex = 7;
    forTable = Hari[weekday() - 1];
    forTable += " ";
    forTable += String(day());
    forTable += " ";
    forTable += String(month());
    forTable += " ";
    forTable += String(year());
    forTable += "=";
    table_value = pembaca.sabtu;
  }

  if (millis() - tampilanMillis >= 1000)
  {
    tampilanMillis = millis();
    Serial.printf("Level Bak      : %d%\nBak Cadangan   : %d%\nBak Utama      : %d%\nSelenoid       : %d\nEmergency      : %d\nSuhu           : %d\nDebit          : %d\nVolume         : %dmL\nRaintriger     : %d\n", BakMandi.levelBak, BakCadangan.levelBak, BakUtama.levelBak, selenoid, emergencyStop, 24, flowmlt, pembaca.total, rainTriger);
    Serial.printf("Blynk Selenoid : %d\n", BlynkSelenoidState);
    Serial.print("Debit air      : ");
    Serial.print(int(debit));
    Serial.print("L/min");
    Serial.println("");

    Serial.print("Volume         : ");
    Serial.print(pembaca.total);
    Serial.println("mL");

    Serial.print("1 minggu       : ");
    Serial.print(satuminggu.total);
    Serial.println("mL");
    Serial.printf("jam: %02d:%02d:%02d Tanggal: %02d/%02d/%02d Hari: %s\n", hour(), minute(), second(), day(), month(), year(), Hari[weekday() - 1]);
    Serial.println("");
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
    idIndex++;
    table.addRow(idIndex, forTable, table_value);
  }
}

void printLCD_info()
{
  static unsigned long waktu__ = 0;
  if (millis() - waktu__ >= 1000)
  {
    waktu__ = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Bak1 |Bak2 |Bak3");
    lcd.setCursor(0, 1);
    lcd.print(String(BakMandi.levelBak) + "%");
    lcd.setCursor(4, 1);
    lcd.print(" |");
    lcd.print(String(BakUtama.levelBak) + "%");
    lcd.setCursor(10, 1);
    lcd.print(" |");
    lcd.print(String(BakCadangan.levelBak) + "%");
  }
}

void printLCD_waktu()
{
  static unsigned long waktu_ = 0;
  static bool ticker = true;
  if (millis() - waktu_ >= 1000)
  {
    lcd.setCursor(0, 0);
    lcd.clear();
    if (ticker)
    {
      lcd.printf("%d:%d", hour(), minute());
    }
    else
    {
      lcd.printf("%d %d", hour(), minute());
    }
    lcd.printf("  Suhu: %dc", int(suhu));
    lcd.setCursor(0, 1);
    lcd.printf("%s %d/%d/%d", String(Hari[weekday() - 1]), day(), month(), year());
    ticker = !ticker;
    waktu_ = millis();
  }
}

void printDebit()
{
  static unsigned long waktu___ = 0;
  if (millis() - waktu___ >= 1000)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Debit: %dL/m", int(debit));
    lcd.setCursor(0, 1);
    lcd.printf("Total: %dL", int(pembaca.total / 1000));
    waktu___ = millis();
  }
}