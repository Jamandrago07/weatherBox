#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_EPD.h"
#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "time.h"

SPIClass *myspi = NULL;

//Define WebServer Port
//WiFiServer server(80);
AsyncWebServer server(80);
String header;
const char* PARAM_MESSAGE = "message";


#ifdef ARDUINO_ADAFRUIT_FEATHER_RP2040_THINKINK // detects if compiling for
                                                // Feather RP2040 ThinkInk
#define EPD_DC PIN_EPD_DC       // ThinkInk 24-pin connector DC
#define EPD_CS PIN_EPD_CS       // ThinkInk 24-pin connector CS
#define EPD_BUSY PIN_EPD_BUSY   // ThinkInk 24-pin connector Busy
#define SRAM_CS -1              // use onboard RAM
#define EPD_RESET PIN_EPD_RESET // ThinkInk 24-pin connector Reset
#define EPD_SPI &SPI1           // secondary SPI for ThinkInk
#else
#define EPD_DC 21
#define EPD_CS 47
#define EPD_BUSY 37 // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS -1
#define EPD_RESET 38  // can set to -1 and share with microcontroller Reset!
#define EPD_SPI &SPI // primary SPI
#endif

#define EPD_SCK 35
#define EPD_MISO 45
#define EPD_MOSI 36

#define BME_SCK 6
#define BME_MISO 5
#define BME_MOSI 4
#define BME_CS 7
#define SEALEVELPRESSURE_HPA (1013.25)

const float p = 3.1415926;

float Tem=0;
float Pre=0;
float Hum=0;
float Alt=0;

const char* ssid     = "KnockForFreeHugs";
const char* password = "FestiveWallaby13CC";
const char* ntpServer = "time.nist.gov";
const long  gmtOffset_sec = 46800;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

// DEFINITIONS
 Adafruit_SSD1680 display(296, 128, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS,EPD_BUSY, EPD_SPI);
 Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

 //####################################################################
 //Functions

 void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void ReadBME(){
  Tem=0.0f;
  Pre=0.0f;
  Hum=0.0f;
  Alt=0.0f;
  Tem=bme.readTemperature();
  Pre=bme.readPressure()/ 100.0F;
  Hum=bme.readHumidity();
  Alt=abs(bme.readAltitude(SEALEVELPRESSURE_HPA));
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
}

void printValues(){
  String t,p,h,a;
  char ct[15],cp[15],ch[15],ca[15];
  t=String(Tem) + String("*C");
  p=String(Pre) + String(" hPa");
  h=String(Hum) + String("%");
  a=String(Alt) + String(" m");
  t.toCharArray(ct,15);
  p.toCharArray(cp,15);
  h.toCharArray(ch,15);
  a.toCharArray(ca,15);
  display.clearBuffer();
  display.setCursor(5, 5);
  display.fillScreen(EPD_WHITE);
  display.setTextColor(EPD_BLACK);
  display.setTextSize(2);
  display.println(ct);
  display.setCursor(130, 5);
  display.println(cp);
  display.setCursor(5,25);
  display.println(ch);
  display.setCursor(130,25);
  display.println(ca);
  display.setTextSize(3);
  display.setCursor(5,60);
  display.setTextColor(EPD_RED);
  display.print(&timeinfo , "%H:%M");
  display.setCursor(5,90);
  display.setTextSize(2);
  display.println(&timeinfo,"%d/%m/%Y");
  display.setCursor(5,110);
  display.setTextSize(1);
  display.println(WiFi.localIP());
  display.display();
}
void ConnectWiFi()
{
  String td;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  while (WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  //IP_Address=WiFi.localIP();
  Serial.println(WiFi.localIP());
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
  delay(500);
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });

    server.onNotFound(notFound);

    server.begin();
}

 //####################################################################
void setup(void) {
  bool status;
  Serial.begin(115200);
  myspi = new SPIClass(FSPI);
  myspi->begin(EPD_SCK, EPD_MISO, EPD_MOSI, EPD_CS);
  status = bme.begin();
    if (!status)
    {
      Serial.println("BME 280 not found. Check wiring");
      while(1);
    }

  display.begin();
  Serial.println("Initialized and COnnected to WiFi");


}

void loop() { 

  ConnectWiFi();
  ReadBME();
  delay(1000);
  printValues();
  delay(56000);

  }