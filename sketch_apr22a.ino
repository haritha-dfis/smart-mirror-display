#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <time.h>

// === TFT Setup ===
#define TFT_CS     5
#define TFT_RST    22
#define TFT_DC     21
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// === Config ===
const char* ssid = "Minni's iphone";
const char* password = "minni1993!";
const char* weatherApiKey = "e9735fb5da6ec9c52acb1e5e18c7e3ad";
const char* weatherCity = "Bengaluru";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 5.5 * 3600;
const int daylightOffset_sec = 0;

// === Timers ===
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherInterval = 10 * 60 * 1000; // 10 minutes
unsigned long lastTimeUpdate = 0;
const unsigned long timeInterval = 1000; // 1 second

void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  WiFi.begin(ssid, password);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println("Connecting WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);

  displayCalendar();  // Static calendar
  displayWeather();   // Initial weather load
}

void loop() {
  unsigned long now = millis();

  if (now - lastTimeUpdate >= timeInterval) {
    displayTimeAndDate();
    lastTimeUpdate = now;
  }

  if (now - lastWeatherUpdate >= weatherInterval) {
    displayWeather();
    lastWeatherUpdate = now;
  }
}

// === TIME & DATE ===
void displayTimeAndDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time");
    return;
  }

  char timeStr[10], dateStr[20];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  strftime(dateStr, sizeof(dateStr), "%a %d %b", &timeinfo);

  tft.fillRect(0, 0, 160, 30, ST77XX_BLACK);
  tft.setCursor(10, 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.println(timeStr);

  tft.setCursor(10, 25);
  tft.setTextSize(1);
  tft.println(dateStr);
}

// === WEATHER ===
void displayWeather() {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(weatherCity) + "&appid=" + String(weatherApiKey) + "&units=metric";

  http.begin(url);
  int code = http.GET();

  tft.fillRect(0, 40, 160, 60, ST77XX_BLACK);

  if (code > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    if (doc["cod"] == 200) {
      String desc = doc["weather"][0]["description"].as<String>();
      float temp = doc["main"]["temp"];
      int humidity = doc["main"]["humidity"];

      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(1);
      tft.setCursor(10, 45); tft.print("Weather: "); tft.println(desc);
      tft.setCursor(10, 60); tft.print("Temp: "); tft.print(temp); tft.println(" C");
      tft.setCursor(10, 75); tft.print("Humidity: "); tft.print(humidity); tft.println(" %");
    }
  } else {
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 50);
    tft.println("Weather fetch failed");
  }
  http.end();
}

// === CALENDAR (Static) ===
void displayCalendar() {
  tft.fillRect(0, 105, 160, 60, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(10, 105);
  tft.setTextSize(1);
  tft.println("Today's Events:");
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 120); tft.println("10AM: Code Review");
}