> # Water Flow Meter
### [referensi](https://tutorial.cytron.io/2019/10/02/interface-water-flow-sensor-using-esp32-board/)

```cpp
/*
  Application:
  - Interface water flow sensor with ESP32 board.
  
  Board:
  - ESP32 Dev Module
    https://my.cytron.io/p-node32-lite-wifi-and-bluetooth-development-kit

  Sensor:
  - G 1/2 Water Flow Sensor
    https://my.cytron.io/p-g-1-2-water-flow-sensor
 */
#include <Arduino.h>
#define LED_BUILTIN 2
#define SENSOR  4

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
  }
}
```
### Flow v2
### [referensi](https://how2electronics.com/arduino-water-flow-sensor-measure-flow-rate-volume/)
```cpp
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
int X;
int Y;
float TIME = 0;
float FREQUENCY = 0;
float WATER = 0;
float TOTAL = 0;
float LS = 0;
const int input = A0;
void setup()
{
Serial.begin(9600);
lcd.begin(16, 2);
lcd.clear();
lcd.setCursor(0,0);
lcd.print(“Water Flow Meter”);
lcd.setCursor(0,1);
lcd.print(“****************”);
delay(2000);
pinMode(input,INPUT);
}
void loop()
{
X = pulseIn(input, HIGH);
Y = pulseIn(input, LOW);
TIME = X + Y;
FREQUENCY = 1000000/TIME;
WATER = FREQUENCY/7.5;
LS = WATER/60;
if(FREQUENCY >= 0)
{
if(isinf(FREQUENCY))
{
lcd.clear();
lcd.setCursor(0,0);
lcd.print(“VOL. :0.00”);
lcd.setCursor(0,1);
lcd.print(“TOTAL:”);
lcd.print( TOTAL);
lcd.print(” L”);
}
else
{
TOTAL = TOTAL + LS;
Serial.println(FREQUENCY);
lcd.clear();
lcd.setCursor(0,0);
lcd.print(“VOL.: “);
lcd.print(WATER);
lcd.print(” L/M”);
lcd.setCursor(0,1);
lcd.print(“TOTAL:”);
lcd.print( TOTAL);
lcd.print(” L”);
}
}
delay(1000);
}
```

> # Sensor ultra sonic
### [referensi](https://www.anakkendali.com/tutorial-esp32-mengakses-sensor-ultrasonik-sebagai-sensor-level-air/)

```cpp
#define pintriger 14
#define pinecho 26

long durasi;
float cm, inch;

void setup()
{
  Serial.begin(9600);
  pinMode (pintriger, OUTPUT);
  pinMode (pinecho, INPUT);
}

void loop()
{
  digitalWrite (pintriger, 0);
  delayMicroseconds(2);
  digitalWrite (pintriger, 1);
  delayMicroseconds(10);
  digitalWrite (pintriger, 0);
  delayMicroseconds(2);
  
  durasi = pulseIn(pinecho, HIGH);
  cm = (durasi * 0.0343)/2;
  inch = cm / 2.54;
  Serial.print(cm);
  Serial.print(" Cm");
  Serial.print("\t");
  Serial.print(inch);
  Serial.println(" Inch");
  delay(50);
}
```
> # Sensor Suhu LM35
### [referensi](https://www.mahirelektro.com/2020/11/mengenal-sensor-suhu-lm35-dan-cara-kerjanya.html)

```cpp
const int sensor = A1;
int tempc;
float tempf;
float vout;
float adc;

void setup() {
  Serial.begin(9600);
  Serial.println("https://mahirelektro.com");
  Serial.println("Tutorial Arduino: Mengakses sensor suhu LM35");
  Serial.println(" ");
  pinMode(sensor, INPUT);
  delay(800);
}

void loop() {
  adc = analogRead(sensor);
  vout = adc / 1023 * 5;
  tempc = vout * 100;
  tempf = (tempc * 1.8) + 32;

  Serial.print("Suhu (Celcius)= ");
  Serial.print(tempc);
  Serial.println(" °C");

  Serial.print("Suhu (Fahrenheit)= ");
  Serial.print(tempf);
  Serial.println(" °F");
  Serial.println(" ");
  delay(500);
}
```
# Main code
```cpp
#define BLYNK_PRINT Serial
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define BlynkWaterFlow_liter V0
#define BlynkUltraSonic_cm V1

#define SensorFlow 2

struct dataStructure
{
public:
  float cm;
  float inch;
  byte pintriger;
  byte pinecho;
  long currentMillis = 0;
  long previousMillis = 0;
};
dataStructure ultr1;
dataStructure ultr2;
dataStructure ultr3;

float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
float totalMilliLitres;

char auth[] = "vIMDGajmKf2p51TYRJpnjY3jdM7GqKDQ";
char ssid[] = "admin";
char pass[] = "";

long durasi;

void IRAM_ATTR pulseCounter();
void WaterFlow_Begin();
void UltraSonic_begin();
float twoDecimalPoints(float parammeter);

void setup()
{
  ultr1.pintriger = 12;
  ultr1.pinecho = 13;
  pinMode(SensorFlow, INPUT_PULLUP);
  pinMode(ultr1.pinecho, INPUT);
  pinMode(ultr1.pintriger, OUTPUT);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;

  attachInterrupt(digitalPinToInterrupt(SensorFlow), pulseCounter, FALLING);

  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, "iot.serangkota.go.id", 8080);
}

void loop()
{
  WaterFlow_Begin();
  // UltraSonic_begin();
  Blynk.run();
}

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void WaterFlow_Begin()
{
  dataStructure flowMillis;
  if (millis() - flowMillis.previousMillis > 1000)
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - flowMillis.previousMillis)) * pulse1Sec) / calibrationFactor;
    flowMillis.previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;

    totalMilliLitres += flowMilliLitres;

    Serial.print("Flow rate: ");
    Serial.print(int(flowRate)); 
    Serial.print("L/min");
    Serial.print("\t");

    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");

    // Write to blynk on V0
    Blynk.virtualWrite(BlynkWaterFlow_liter, twoDecimalPoints(totalMilliLitres/1000.0));
  }
}

void UltraSonic_begin()
{
  digitalWrite (ultr1.pintriger, 0);
  delayMicroseconds(2);
  digitalWrite (ultr1.pintriger, 1);
  delayMicroseconds(10);
  digitalWrite (ultr1.pintriger, 0);
  delayMicroseconds(2);
  durasi = pulseIn(ultr1.pinecho, HIGH);
  ultr1.cm = (durasi * 0.0343)/2;
  ultr1.inch = ultr1.cm / 2.54;
  dataStructure ultrasonicMillis;
  if(millis() - ultrasonicMillis.previousMillis > 1000)
  {
    ultrasonicMillis.previousMillis = millis();
    Serial.println(ultr1.cm);
    // Write to Blynk on V1
    Blynk.virtualWrite(BlynkUltraSonic_cm, ultr1.cm);
  }
}

float twoDecimalPoints(float parammeter)
{
  int newparram = ((parammeter + 0.05) * 10);
  parammeter = (newparram / 10.0);
  return parammeter;
}
```
# FreeRTOS
referensi by me
```cpp
// #if CONFIG_FREERTOS_UNICORE
// #define ARDUINO_RUNNING_CORE 0
// #else
// #define ARDUINO_RUNNING_CORE 1
// #endif


/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "HTTPClient.h"
#include "RTClib.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "VDRT_pdiGbc7NGDhshlEIwwC_R9o1hZl";
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "andri";
char pass[] = "";

RTC_DS3231 rtc;

TaskHandle_t Task1;
TaskHandle_t Task2;

WiFiClient client;
const char *host = "iambic-concerns.000webhostapp.com";
const char *host_openWeather = "api.openweathermap.org";
String kota = "Bandung";
String api = "2eb5d5fd2d079473a2c0897d2bd41dd2";


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned char myBitmapbattery_16x8[] PROGMEM = {
    0xff, 0xfc, 0x80, 0x04, 0x80, 0x06, 0x80, 0x06, 0x80, 0x06, 0x80, 0x06, 0x80, 0x04, 0xff, 0xfc};
// 'wifi1', 11x8px
const unsigned char myBitmapwifi1[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00};
// 'wifi2', 11x8px
const unsigned char myBitmapwifi2[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x11, 0x00, 0x04, 0x00};
// 'wifi3', 11x8px
const unsigned char myBitmapwifi3[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x20, 0x80, 0x4e, 0x40, 0x11, 0x00, 0x04, 0x00};
// 'wifi4', 11x8px
const unsigned char myBitmapwifi4[] PROGMEM = {
    0x00, 0x00, 0x3f, 0x80, 0x40, 0x40, 0x9f, 0x20, 0x20, 0x80, 0x4e, 0x40, 0x11, 0x00, 0x04, 0x00};
// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 128)
const unsigned char *WiFi_Array[4] = {
    myBitmapwifi1,
    myBitmapwifi2,
    myBitmapwifi3,
    myBitmapwifi4};

int p(int n)
{
  return (n / 10) % 10;
}

int s(int n)
{
  return n % 10;
}

void kirim()
{
  int i = random(2165);
  int suhu = random(i);
  int kelembaban = random(i);
  int co2 = random(i);
  int o2 = random(i);
  int debu = random(i);

  if (!client.connect(host_openWeather, 80))
  {
    Serial.println("Connection failed..!");
    return;
  }
  String link = "";
  HTTPClient http;

  link = "http://" + String(host) + "/php/Terima.php?suhu=" + String(suhu) + "&kelembaban=" + String(kelembaban) + "&co2=" + String(co2) + "&o2=" + String(o2) + "&pm2=" + String(debu);
  http.begin(link);
  http.GET();
  String respon = http.getString();
  Serial.println(respon);
  http.end();
}

void Minta_openWeather()
{
  if (!client.connect(host, 80))
  {
    Serial.println("Connection failed..!");
    return;
  }
  String link = "";
  HTTPClient http;
  // http://api.openweathermap.org/data/2.5/weather?q={City}&appid={API}&units=metric
  link = "http://" + String(host_openWeather) + "/data/2.5/weather?q=" + kota + "&appid=" + api + "&units=metric";
  http.begin(link);
  http.GET();
  String respon = http.getString();
  // Serial.println(respon);
  http.end();

  // String input;
  StaticJsonDocument<1024> doc;

  DeserializationError error = deserializeJson(doc, respon/*, DeserializationOption::Filter(filter)*/);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  const char* weather_0_main = doc["weather"][0]["main"]; // "Clouds"
  const char* weather_0_description = doc["weather"][0]["description"]; // "overcast clouds"

  JsonObject main = doc["main"];
  float main_temp = main["temp"]; // 295.25
  int main_pressure = main["pressure"]; // 1012
  int main_humidity = main["humidity"]; // 91

  // Serial.println(respon);
  Serial.printf("Temp: %2.1f\nHumi: %i\nPresure: %i\n", main_temp, main_humidity, main_pressure);
}


/*---------------------- Tasks ---------------------*/
void Task_One(void *pvParameters) // This is a task.
{
  Blynk.begin(auth, ssid, pass);
  for (;;) // A Task shall never return or exit.
  {
    // Task 1 LOOP WILL BE HERE
    Blynk.run();
    if (!Blynk.connected())
    {
      Blynk.begin(auth, ssid, pass);
    }
  }
}

void Task_Two(void *pvParameters) // This is a task.
{
  //  Serial.begin(115200);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
    {
      // Don't proceed, loop forever
    }
  }
  // WiFi.begin(ssid, pass);
  rtc.begin();
  long interval_myweb = 5*60*1000;
  long interval_weather = 1*60*1000;
  long Data_Kirim = 0;
  long Data_Kirim_ = 0;
  for (;;)
  {
    //Task 2 LOOP WILL BE HERE
    DateTime now = rtc.now();
    long rssi = WiFi.RSSI();
    // Serial.print("RSSI:");
    // Serial.println(rssi);
    int jam = now.hour();
    int mnt = now.minute();
    display.clearDisplay();
    display.drawBitmap(112, 0, myBitmapbattery_16x8, 16, 8, WHITE);
    display.fillRect(112, 0, 10, 8, WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.printf("%d%d:%d%d", p(jam), s(jam), p(mnt), s(mnt));

    if (WiFi.status() == WL_CONNECTED)
    {
      // display.setCursor(35, 0);
      // display.print("WiFi");
      display.drawBitmap(98, 0, WiFi_Array[map(rssi, -110, -20, 0, 4)], 11, 8, WHITE);
    }
    else
    {
      display.setCursor(35, 0);
      display.print("");
      rssi = -110;
    }
    display.display();
    if(millis() - Data_Kirim >= interval_myweb)
    {
      Data_Kirim = millis();
      kirim(); // Here will be LAG on TASK TWO.
    }
    if(millis() - Data_Kirim_ >= interval_weather)
    {
      Data_Kirim_ = millis();
      Minta_openWeather(); // Here will be LAG on TASK TWO.
    }
  }
}

// the setup function runs once when you press reset or power the board
void setup()
{

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
      Task_One
      ,"TaskBlynk" // A name just for humans
      ,5000 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,NULL
      ,2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,&Task1, 
      1); // Selected Core

  xTaskCreatePinnedToCore(
      Task_Two
      ,"UI"
      ,5000 // Stack size
      ,NULL
      ,0 // Priority
      ,&Task2
      ,0); // Selected Core
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Dont put anything in here.
}
```

# Ultra sonic 2

```cpp
/* --- www.nyebarilmu.com --- */
 
/*
Sensor jarak HC-SR04
pin yang perlu dihubungkan
VCC to arduino 5v
GND to arduino GND
Echo to Arduino pin 7
Trig to Arduino pin 8
*/
 
#define echoPin 12 //Echo Pin
#define trigPin 11 //Trigger Pin
#define LEDPin 13 //Led default dari Arduino uno
 
int maximumRange = 200; //kebutuhan akan maksimal range
int minimumRange = 00; //kebutuhan akan minimal range
long duration, distance; //waktu untuk kalkulasi jarak
 
void setup() {
Serial.begin (9600); //inisialiasasi komunikasi serial
//deklarasi pin
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);
pinMode(LEDPin, OUTPUT);
}
 
void loop() {
/* Berikut siklus trigPin atau echo pin yang digunakan
untuk menentukan jarak objek terdekat dengan memantulkan
gelombang suara dari itu. */
digitalWrite(trigPin, LOW);delayMicroseconds(2);
digitalWrite(trigPin, HIGH);delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
 
//perhitungan untuk dijadikan jarak
distance = duration/58.2;
 
/* Kirim angka negatif ke komputer dan Turn LED ON 
untuk menunjukkan "di luar jangkauan" */
 
if (distance >= maximumRange || distance <= minimumRange)
{
Serial.println("-1");digitalWrite(LEDPin, HIGH);
}
else {
 
/*Kirim jarak ke komputer menggunakan Serial protokol, dan
menghidupkan LED OFF untuk menunjukkan membaca sukses. */
Serial.println(distance);
digitalWrite(LEDPin, LOW);
 
//waktu tunda 50mS
delay(50);
}
}
```