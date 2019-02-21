#include <GxEPD.h>
#include <GxGDEW042T2/GxGDEW042T2.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Keep your project specific credentials in a non-version controlled
// `credentials.h` file and set the `NO_CREDENTIALS_HEADER` to false.
#define NO_CREDENTIALS_HEADER false
#if NO_CREDENTIALS_HEADER == true
const auto kInternetSSID = "your-ssid";
const auto kWifiPassword = "your-wifi-password";
const auto kServerUrl = "http://url.com/for/your/server";
#else
#include "credentials.h"
#endif

const int kScreenWidth = 400;
const int kScreenHeight = 300;
const int kFont24ptInPixelsHeight = 32;
const int kFont18ptInPixelsHeight = 24;
const int kFont12ptInPixelsHeight = 16;
const int kFont9ptInPixelsHeight = 12;
const int kHeightCursorOffset = 30;

const int kTopTimeLengthInPixels = 142;
const int kTopRectangleWidth = kScreenWidth;
const int kTopRectangleHeight = kScreenHeight / 6;
const int kTopTimeCursorX = (kScreenWidth - kTopTimeLengthInPixels) / 2;
const int kTopTimeCursorY = kHeightCursorOffset + (kTopRectangleHeight - kFont24ptInPixelsHeight) / 2;

const int kMiddleDividerWidth = 5;
const int kMiddleDividerCursorY = kTopRectangleHeight;
const int kMiddleDividerCursorX = (kScreenWidth - kMiddleDividerWidth) / 2;
const int kMiddleAreaHeight = 3.5 * kTopRectangleHeight;

const int kVarbergsgatanWidth = 182;
const int kFredasgatanWidth = 154;
const int kVarbergsgatanPadding = (kMiddleDividerCursorX - kVarbergsgatanWidth) / 2;
const int kFredasgatanPadding = (kMiddleDividerCursorX - kFredasgatanWidth) / 2;
const int kVarbergsgatanCursorX = kVarbergsgatanPadding;
const int kFredasgatanCursorX = kMiddleDividerCursorX + kMiddleDividerWidth + kFredasgatanPadding;
const int kStationNameCursorY = kTopRectangleHeight + kFont12ptInPixelsHeight * 1.3;
const int kStationNameDividerCursorY = kStationNameCursorY + kFont12ptInPixelsHeight * 0.7;

const int kNextDepartureLabelCursorY = kStationNameDividerCursorY + kFont9ptInPixelsHeight * 1.3;
const int kNextDepartureMinutesCursorY = kNextDepartureLabelCursorY + kFont24ptInPixelsHeight * 3;
const int kNextDepartureVarbergsgatanCursorX = kVarbergsgatanCursorX - 10;
const int kNextDepartureFredasgatanCursorX = kFredasgatanCursorX - 10;
const int kLaterDepartureMinutesCursorY = kNextDepartureMinutesCursorY + kFont12ptInPixelsHeight * 1.6;
const int kLaterDeparturesRightPadding = 30;
const int kLaterVarbergsgatanDeparturesCursorX = kMiddleDividerCursorX - kLaterDeparturesRightPadding;
const int kLaterFredasgatanDeparturesCursorX = kScreenWidth - kLaterDeparturesRightPadding;

const int kBottomRectangleHeight = kScreenHeight - kMiddleAreaHeight - kTopRectangleHeight;
const int kBottomDividerCursorY = kTopRectangleHeight + kMiddleAreaHeight;
const int kBottomDividerHeight = kMiddleDividerWidth;
const int kBottomBoxCursorY = kBottomDividerCursorY + kBottomDividerHeight;
const int kBottomBoxWidth = kScreenWidth / 4;

const int kTemperatureCursorY = kBottomDividerCursorY + kMiddleDividerWidth + kFont12ptInPixelsHeight * 1.2;
const int kDegreesCursorY = kTemperatureCursorY - kFont12ptInPixelsHeight * 0.5;
const char* kDegreesSymbol = "o";

const int kWeatherCursorY = kTemperatureCursorY + kFont12ptInPixelsHeight * 1.4;
const int kWeatherTimeCursorY = kWeatherCursorY + kFont12ptInPixelsHeight * 1.4;
const int kWeatherPadding = 1;

const unsigned long kRequestTimeout = 15000UL; // In milliseconds
const unsigned long kNormalDeepSleepDuration = 60e6; // In microseconds
const unsigned long kErrorDeepSleepDuration = kNormalDeepSleepDuration / 4; // In microseconds
const unsigned long kLongDeepSleepDuration = kNormalDeepSleepDuration * 60; // In microseconds

GxIO_Class io(SPI, /*CS=D8*/ 4, /*DC=D6*/ 12, /*RST=D4*/ 2);
GxEPD_Class display(io, /*RST=D4*/ 2, /*BUSY=D2*/ 5);

unsigned long timeOfTheDayToSleepInterval(const char* timeOfDay) {
  // Verify we actually received an input in the format HH:mm
  if (timeOfDay == nullptr || timeOfDay[0] == '\0' || timeOfDay[1] == '\0') {
    return kNormalDeepSleepDuration;
  }
  const char timeBuffer[] = {timeOfDay[0], timeOfDay[1]};
  const auto currentHour = atoi(timeBuffer);

  // If it's night or too early in the morning, there's no need for updates
  if (currentHour > 22 || currentHour < 7) {
    return kLongDeepSleepDuration;
  }

  return kNormalDeepSleepDuration;
}

void draw(const char* updateTime,
          const char* nextVarbergsgatanDeparture, const char* laterVarbergsgatanDeparture,
          const char* nextFredasgatanDeparture, const char* laterFredasgatanDeparture,
          const char* recentPredictionTime, const char* recentWeather, int recentTemperature,
          const char* shortRangePredictionTime, const char* shortRangePredictionWeather, int shortRangePredictionTemperature,
          const char* midRangePredictionTime, const char* midRangePredictionWeather, int midRangePredictionTemperature,
          const char* longRangePredictionTime, const char* longRangePredictionWeather, int longRangePredictionTemperature) {

  display.init();
  pinMode(12, OUTPUT); // Pin 12 on ESP8266 is initialized as input by SPI so we must set it as OUTPUT again
  display.fillScreen(GxEPD_WHITE);


  display.fillRect(0, 0, kTopRectangleWidth, kTopRectangleHeight, GxEPD_BLACK);
  display.setFont(&FreeMonoBold24pt7b);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(kTopTimeCursorX, kTopTimeCursorY); // w: (400 - 140) / 2 -- h: 30 + (60 - 32) / 2
  display.print(updateTime);

  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(kVarbergsgatanCursorX, kStationNameCursorY);
  display.print("Varbergsgatan");
  display.setCursor(kFredasgatanCursorX, kStationNameCursorY);
  display.print("Fredasgatan");

  display.drawFastHLine(0, kStationNameDividerCursorY, kScreenWidth, GxEPD_BLACK);

  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(kVarbergsgatanCursorX, kNextDepartureLabelCursorY);
  display.print("Next trams:");
  display.setCursor(kFredasgatanCursorX, kNextDepartureLabelCursorY);
  display.print("Next buses:");

  display.setFont(&FreeMonoBold24pt7b);
  display.setTextSize(3);
  display.setCursor(kNextDepartureVarbergsgatanCursorX, kNextDepartureMinutesCursorY);
  display.print(nextVarbergsgatanDeparture);
  display.setCursor(kNextDepartureFredasgatanCursorX, kNextDepartureMinutesCursorY);
  display.print(nextFredasgatanDeparture);

  display.setFont(&FreeMonoBold12pt7b);
  display.setTextSize(1);
  display.setCursor(kLaterVarbergsgatanDeparturesCursorX, kLaterDepartureMinutesCursorY);
  display.print(laterVarbergsgatanDeparture);
  display.setCursor(kLaterFredasgatanDeparturesCursorX, kLaterDepartureMinutesCursorY);
  display.print(laterFredasgatanDeparture);

  display.fillRect(kMiddleDividerCursorX, kMiddleDividerCursorY, kMiddleDividerWidth, kMiddleAreaHeight, GxEPD_BLACK);
  display.fillRect(0, kBottomDividerCursorY, kScreenWidth, kBottomDividerHeight, GxEPD_BLACK);

  display.fillRect(0, kBottomBoxCursorY, kBottomBoxWidth, kBottomRectangleHeight, GxEPD_BLACK);
  display.fillRect(kScreenWidth / 2, kBottomBoxCursorY, kBottomBoxWidth, kBottomRectangleHeight, GxEPD_BLACK);

  // Most recent weather data
  display.setCursor(0, kTemperatureCursorY);
  display.setTextColor(GxEPD_WHITE);
  display.print(recentTemperature);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(display.getCursorX(), kDegreesCursorY);
  display.print(kDegreesSymbol);

  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(kWeatherPadding, kWeatherCursorY);
  display.print(recentWeather);
  display.setCursor(0, kWeatherTimeCursorY);
  display.print(recentPredictionTime);

  // Next prediction weather data
  display.setCursor(kWeatherPadding + kBottomBoxWidth, kTemperatureCursorY);
  display.setTextColor(GxEPD_BLACK);
  display.print(shortRangePredictionTemperature);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(display.getCursorX(), kDegreesCursorY);
  display.print(kDegreesSymbol);

  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(kWeatherPadding + kBottomBoxWidth, kWeatherCursorY);
  display.print(shortRangePredictionWeather);

  display.setCursor(kWeatherPadding + kBottomBoxWidth, kWeatherTimeCursorY);
  display.print(shortRangePredictionTime);

  // Mid prediction weather data
  display.setCursor(kWeatherPadding + 2 * kBottomBoxWidth, kTemperatureCursorY);
  display.setTextColor(GxEPD_WHITE);
  display.print(midRangePredictionTemperature);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(display.getCursorX(), kDegreesCursorY);
  display.print(kDegreesSymbol);

  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(kWeatherPadding + 2 * kBottomBoxWidth, kWeatherCursorY);
  display.print(midRangePredictionWeather);

  display.setCursor(kWeatherPadding + 2 * kBottomBoxWidth, kWeatherTimeCursorY);
  display.print(midRangePredictionTime);

  // Far prediction weather data
  display.setCursor(kWeatherPadding + 3 * kBottomBoxWidth, kTemperatureCursorY);
  display.setTextColor(GxEPD_BLACK);
  display.print(longRangePredictionTemperature);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(display.getCursorX(), kDegreesCursorY);
  display.print(kDegreesSymbol);

  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(kWeatherPadding + 3 * kBottomBoxWidth, kWeatherCursorY);
  display.print(longRangePredictionWeather);

  display.setCursor(kWeatherPadding + 3 * kBottomBoxWidth, kWeatherTimeCursorY);
  display.print(longRangePredictionTime);

  display.update();
}

/**
   (Re)connects the module to WiFi

   @return true if connection was succesful otherwise false
*/
bool connectToWifi() {
  // Set WiFi to station mode & disconnect from an AP if previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Try to connect to the internet
  WiFi.begin(kInternetSSID, kWifiPassword);
  auto attemptsLeft = 1000;
  while ((WiFi.status() != WL_CONNECTED) && (--attemptsLeft > 0)) {
    delay(10); // Wait a bit before retrying
  }
  return attemptsLeft > 0;
}

void setup()
{
  if (!connectToWifi()) {
    ESP.deepSleep(kErrorDeepSleepDuration);
    return;
  }

  HTTPClient http;
  http.setTimeout(kRequestTimeout);
  http.begin(kServerUrl);
  auto httpCode = http.GET();

  if (httpCode < 0 || httpCode != HTTP_CODE_OK) {
    http.end();
    ESP.deepSleep(kErrorDeepSleepDuration);
    return;
  }

  DynamicJsonDocument doc(600);
  deserializeJson(doc, http.getStream());
  JsonObject root = doc.as<JsonObject>();

  JsonArray predictions = root["pred"];
  JsonArray recentPredictions = predictions[0];
  JsonArray shortRangePredictions = predictions[1];
  JsonArray midRangePredictions = predictions[2];
  JsonArray longRangePredictions = predictions[3];

  draw(root["time"].as<const char*>(),
       root["depsA"][0].as<const char*>(), root["depsA"][1].as<const char*>(),
       root["depsB"][0].as<const char*>(), root["depsB"][1].as<const char*>(),
       recentPredictions[0].as<const char*>(), recentPredictions[1].as<const char*>(), recentPredictions[2].as<int>(),
       shortRangePredictions[0].as<const char*>(), shortRangePredictions[1].as<const char*>(), shortRangePredictions[2].as<int>(),
       midRangePredictions[0].as<const char*>(), midRangePredictions[1].as<const char*>(), midRangePredictions[2].as<int>(),
       longRangePredictions[0].as<const char*>(), longRangePredictions[1].as<const char*>(), longRangePredictions[2].as<int>());

  ESP.deepSleep(timeOfTheDayToSleepInterval(root["time"].as<const char*>()));
}

void loop()
{
  // We should never be here, but just in case this ever happens, go to sleep
  ESP.deepSleep(kErrorDeepSleepDuration);
}
