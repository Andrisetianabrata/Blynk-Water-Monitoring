#include <Arduino.h>
#define BLYNK_MAX_SEND_BYTE 256
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <ArduinoJson.h>
#define path_data "/data.json"
#include "OneButton.h"

const byte Rainsensor = 4;
const byte ButtonSelenoid = 23;
const byte TemperatureSensor = 32;
const byte Selenoid_1 = 16;
byte flowsensor = 2; // Pin sensor Flow

BlynkTimer timer;
WidgetLED SelenoidLED(V5);
WidgetLED sensorHujan(V10);
WidgetRTC rtc;
WidgetTable table;
BLYNK_ATTACH_WIDGET(table, V9);
int idIndex = 0;

TaskHandle_t Task1;
TaskHandle_t Task2;

long duration;
float distance;

struct hari
{
  long minggu, senin, selasa, rabu, kamis, jumat, sabtu, total;
} pembaca, satuminggu;

byte sensorInt = 0;
float konstanta = 7.5; // konstanta flow meter
volatile byte pulseCount;
float debit;
unsigned int flowmlt;
unsigned long oldTime;

char auth[] = "E7VmvS18KPZnvlPlIBRAIOqCeh_QoOD2";
char ssid[] = "andri";
char pass[] = "";
char server[] = "iot.serangkota.go.id";

bool rainTriger = false;

bool selenoid;
bool selenoid1;
bool selenoid2;
bool BlynkSelenoidState;
bool emergencyStop;

char Hari[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

OneWire oneWire(TemperatureSensor);
DallasTemperature sensorTemp(&oneWire);

OneButton button(ButtonSelenoid, true, true);

File file;

bool mulaiJam;
unsigned long tampilanMillis = 0;

struct levelAir
{
public:
  int minimal;
  int maksimal;
  int levelBak;
  int persenMinimal;
  int persenMaksimal;
  byte echo;
  byte trig;

  uint32_t penghitung(byte triger, byte echo_)
  {
    digitalWrite(triger, LOW);
    delayMicroseconds(2);
    digitalWrite(triger, HIGH);
    delayMicroseconds(10);
    digitalWrite(triger, LOW);

    duration = pulseIn(echo_, HIGH);
    distance = (duration * 0.0343) / 2;
    return distance;
  }
} BakMandi, BakUtama, BakCadangan;

LiquidCrystal_I2C lcd(0x27,16,2);
uint8_t clickable = true;
static unsigned long myWaktu = 0;
float suhu;