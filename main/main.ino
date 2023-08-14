#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h> // Include the HTTPClient library

#ifndef STASSID
#define STASSID "이재민의 iPhone"
#define STAPSK "1234qwer"

#define SERVER_IP "127.0.0.1:3000"
#endif

WiFiClientSecure client;

const char* ssid = STASSID;
const char* password = STAPSK;

int pulseSensorPurplePin = A0;
int signal;
int ledPin = 2;
int peizoPin = 16;
int emergencyButtonPin = 14;

int Threshold = 550;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(peizoPin, OUTPUT);
  pinMode(emergencyButtonPin, INPUT_PULLUP);

  Serial.begin(115200);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);

  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  client.setInsecure();
}

void sendHttpPostRequest(const char* path, const char* requestBody, String& responseMessage) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    String url = "http://" + String(SERVER_IP) + String(path);
    http.begin(client, url);

    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    int httpCode = http.POST(requestBody);

    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        responseMessage = payload;
        Serial.println("Received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void registerDevice(const char* deviceId, String& responseMessage) {
  const char* path = "/register/";
  String requestBody = "{\"deviceId\": \"" + String(deviceId) + "\"}";
  sendHttpPostRequest(path, requestBody.c_str(), responseMessage);
}

void detectFall(const char* deviceId, int heartRate, bool imageCapture, String& responseMessage) {
  const char* path = "/fall-detection/";
  String requestBody = "{\"deviceId\": \"" + String(deviceId) + "\","
                      "\"heartRate\": " + String(heartRate) + ","
                      "\"imageCapture\": " + String(imageCapture ? "true" : "false") + "}";
  sendHttpPostRequest(path, requestBody.c_str(), responseMessage);
}



void loop() {
  // Example usage
  String response;

  // Register Device
  const char* deviceId = "your_device_id";
  registerDevice(deviceId, response);
  Serial.println("Device Registration Response: " + response);

  // Detect Fall
  int heartRate = 15;
  bool imageCapture = false;
  detectFall(deviceId, heartRate, imageCapture, response);
  Serial.println("Fall Detection Response: " + response);

  // Other loop code
  delay(10000);
}

