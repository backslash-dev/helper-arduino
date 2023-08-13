
int pulseSensorPurplePin = A0; // 심박센서 핀
int signal; // 실제 감지되는 심박수 값
int ledPin = 2; // 보드 LED 핀
 
int Threshold = 550; // 심박수 임계값
 
 
void setup() {
  pinMode(ledPin, OUTPUT);  
  Serial.begin(9600);    
 
}
 
void loop() {
  signal = analogRead(pulseSensorPurplePin);
  Serial.println(signal);
 
  if(signal > Threshold){                  
     digitalWrite(ledPin, HIGH);
  } else {
     digitalWrite(ledPin, LOW);               
  }

  delay(10);
}