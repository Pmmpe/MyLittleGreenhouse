#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <ESP8266HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "";
const char* password = "";

// ThingSpeak settings
const char* apiKey = "182KUICAY3DQXWLA";
const char* server = "http://api.thingspeak.com/update";

// Deep sleep time (in microseconds)
const uint64_t sleepDuration = 5 * 60 * 1e6;  // 5 minutes

// Pins
#define FLASH_BUTTON_PIN 0     // GPIO0
#define LED_PIN 2              // GPIO2 (onboard LED)

Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nBooting...");

  pinMode(LED_PIN, OUTPUT);
  pinMode(FLASH_BUTTON_PIN, INPUT_PULLUP);

  // If cold boot, wait for FLASH button press
  if (ESP.getResetInfoPtr()->reason == REASON_DEFAULT_RST) {
    Serial.println("Cold boot detected. Waiting for FLASH button press...");

    digitalWrite(LED_PIN, LOW);  // Turn LED ON
    while (digitalRead(FLASH_BUTTON_PIN) == HIGH) {
      delay(100);
    }
    digitalWrite(LED_PIN, HIGH); // Turn LED OFF
    Serial.println("FLASH button pressed. Proceeding...");
  } else {
    Serial.println("Wake from deep sleep. Continuing automatically...");
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi Failed. Sleeping...");
    goToSleep();
  }

  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  // BME280 sensor init
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found!");
    delay(3000);
    goToSleep();
  }

  // Read data
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  Serial.printf("Temp: %.2f°C  Humidity: %.2f%%  Pressure: %.2f hPa\n",
                temperature, humidity, pressure);

  // Send to ThingSpeak
  WiFiClient client;
  HTTPClient http;
  String url = String(server) + "?api_key=" + apiKey +
               "&field1=" + String(temperature) +
               "&field2=" + String(humidity) +
               "&field3=" + String(pressure);

  http.begin(client, url);
  int httpCode = http.GET();
  Serial.print("ThingSpeak response: ");
  Serial.println(httpCode);
  http.end();

  delay(200);
  goToSleep();
}

void loop() {
  // Not used
}

void goToSleep() {
  Serial.println("Going to deep sleep...");
  delay(100);
  ESP.deepSleep(sleepDuration);
}
