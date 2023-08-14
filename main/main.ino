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

void loop() {
  sendHttpPostRequest(); // Call the function to send HTTP POST request
}

void sendHttpPostRequest() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://" SERVER_IP "/register/");  // HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST(" {\"deviceId\": \"your_device_id\"}");

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);
}
