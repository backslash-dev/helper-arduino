#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Wi-Fi and server configuration
#define WIFI_SSID "이재민의 iPhone"
#define WIFI_PASS "1234qwer"
#define SERVER_IP "43.201.83.172:3000"

// Pins
const int ledPin = 2;
const int peizoPin = 16;
const int emergencyButtonPin = 14;
const int pulseSensorPurplePin = A0;

// 위급 상황 감지 변수
unsigned long lastHeartbeatTime = 0;
unsigned long lastEmergencyButtonPressTime = 0;

int Signal;


// Fall detection threshold
const int Threshold = 550;
int heartRate = 15;
int heartDownCount = 0;
int buttonWatingCount = 0;
bool imageCapture = false;
String response;

const char* deviceId = "arduino";

// Function prototypes
void setupWiFi();
void syncTime();
void sendHttpPostRequest(const char* path, const char* requestBody, String& responseMessage);
void registerDevice(const char* deviceId, String& responseMessage);
void detectFall(const char* deviceId, int heartRate, bool imageCapture, String& responseMessage);

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(peizoPin, OUTPUT);
  pinMode(emergencyButtonPin, INPUT_PULLUP);

  Serial.begin(115200);

  setupWiFi();
  syncTime();

  // Set up client for insecure connections (not recommended for production)
  WiFiClientSecure client;
  client.setInsecure();

  // // Register Device
  // registerDevice(deviceId, response);
  // Serial.println("Device Registration Response: " + response);
}

void setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void syncTime() {
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
}

void sendHttpPostRequest(const char* path, const char* requestBody, String& responseMessage) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  WiFiClient client;
  HTTPClient http;

  String url = "http://" + String(SERVER_IP) + String(path);
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

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
  unsigned long currentTime = millis();  // 현재 시간을 밀리초 단위로 가져옴

  // 심박수 감지
  Signal = analogRead(pulseSensorPurplePin);

  if (Signal > Threshold) {
    lastHeartbeatTime = currentTime;  // 심박수가 감지될 때마다 시간 갱신
  } else {
    // 1분 동안 심박수가 감지되지 않을 때 
    if (currentTime - lastHeartbeatTime >= 60000) {

      // 부저 울림
      if (currentTime % 2000 < 1000) {
        digitalWrite(peizoPin, HIGH);
      } else {
        digitalWrite(peizoPin, LOW);
      }
        
      // 버튼 체크
      int buttonState = digitalRead(emergencyButtonPin);

      if (buttonState == LOW) {
        lastEmergencyButtonPressTime = currentTime; // 버튼이 눌린 시간 기록
      }

      // 버튼 안누른 지 20초가 지나면
      if (buttonState == HIGH && currentTime - lastEmergencyButtonPressTime >= 20000) {
        // 서버 전송
        detectFall(deviceId, heartRate, imageCapture, response);
        Serial.println("Fall Detection Response: " + response);        
      }
    }
  }

  delay(10);
}
