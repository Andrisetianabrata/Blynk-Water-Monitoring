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

// int levelPnmpngn;
// int levelBakMandi;
bool selenoid;
bool selenoid1;
bool selenoid2;
bool BlynkSelenoidState;
bool emergencyStop;

char Hari[7][12] = {"Miggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

OneWire oneWire(TemperatureSensor);
DallasTemperature sensorTemp(&oneWire);

OneButton button(ButtonSelenoid, true, true);

File file;

bool mulaiJam;
unsigned long tampilanMillis = 0;

struct levelAir
{
  int minimal;
  int maksimal;
  int levelBak;
  byte echo;
  byte trig;
  int penghitung(byte triger, byte echo);
}BakMandi, BakUtama, BakCadangan;
