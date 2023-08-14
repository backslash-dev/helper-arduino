#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h> // Include the HTTPClient library

#ifndef STASSID
#define STASSID "이재민의 iPhone" // Replace with your Wi-Fi SSID
#define STAPSK "1234qwer"        // Replace with your Wi-Fi password

#define SERVER_IP "127.0.0.1:3000" // Replace with the IP address and port of your server
#endif

WiFiClientSecure client;

const char* ssid = STASSID;      // Wi-Fi SSID
const char* password = STAPSK;   // Wi-Fi password

int pulseSensorPurplePin = A0;   // Analog input pin for the pulse sensor
int signal;                       // Variable to store the sensor signal
int ledPin = 2;                   // Pin for an LED indicator
int peizoPin = 16;                // Pin for a piezo buzzer
int emergencyButtonPin = 14;      // Pin for an emergency button

int Threshold = 550;              // Threshold value for fall detection

void setup() {
  pinMode(ledPin, OUTPUT);        // Set LED pin as output
  pinMode(peizoPin, OUTPUT);      // Set piezo buzzer pin as output
  pinMode(emergencyButtonPin, INPUT_PULLUP); // Set emergency button pin as input with pull-up resistor

  Serial.begin(115200);           // Initialize the serial communication

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

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // Configure NTP time synchronization
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

  client.setInsecure(); // Allow insecure connections (not recommended for production use)
}

void sendHttpPostRequest(const char* path, const char* requestBody, String& responseMessage) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    String url = "http://" + String(SERVER_IP) + String(path);
    http.begin(client, url); // Begin the HTTP connection

    http.addHeader("Content-Type", "application/json"); // Set the HTTP header for JSON content

    Serial.print("[HTTP] POST...\n");
    int httpCode = http.POST(requestBody); // Send the POST request with the given JSON data

    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString(); // Get the response payload
        responseMessage = payload;
        Serial.println("Received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Close the HTTP connection
  }
}

void registerDevice(const char* deviceId, String& responseMessage) {
  const char* path = "/register/";
  String requestBody = "{\"deviceId\": \"" + String(deviceId) + "\"}";
  sendHttpPostRequest(path, requestBody.c_str(), responseMessage); // Send a POST request to register the device
}

void detectFall(const char* deviceId, int heartRate, bool imageCapture, String& responseMessage) {
  const char* path = "/fall-detection/";
  String requestBody = "{\"deviceId\": \"" + String(deviceId) + "\","
                      "\"heartRate\": " + String(heartRate) + ","
                      "\"imageCapture\": " + String(imageCapture ? "true" : "false") + "}";
  sendHttpPostRequest(path, requestBody.c_str(), responseMessage); // Send a POST request for fall detection
}

void loop() {
  // Example usage
  String response;

  // Register Device
  const char* deviceId = "your_device_id"; // Replace with your device ID
  registerDevice(deviceId, response);
  Serial.println("Device Registration Response: " + response);

  // Detect Fall
  int heartRate = 15;        // Example heart rate value
  bool imageCapture = false; // Example image capture status
  detectFall(deviceId, heartRate, imageCapture, response);
  Serial.println("Fall Detection Response: " + response);

  // Other loop code
  delay(10000); // Wait for a period before repeating the loop
}
