
int pulseSensorPurplePin = A0; // 심박센서 핀
int signal; // 실제 감지되는 심박수 값
int ledPin = 2; // 보드 LED 핀
int peizoPin = 16; //페이조 부저 + 핀
int emergencyButtonPin = 14; //위급상황 버튼 핀
 
int Threshold = 550; // 심박수 임계값
 
 
void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(peizoPin, OUTPUT); 
  pinMode(emergencyButtonPin, INPUT_PULLUP);

  Serial.begin(9600);    
 
}
 
void loop() {
  // signal = analogRead(pulseSensorPurplePin);
  // Serial.println(signal);
 
  // if(signal > Threshold){                  
  //    digitalWrite(ledPin, HIGH);
  // } else {
  //    digitalWrite(ledPin, LOW);               
  // }

  if (digitalRead(emergencyButtonPin) == LOW) {
    digitalWrite(ledPin, LOW);
    digitalWrite(peizoPin, HIGH);
  } else {
    digitalWrite(ledPin, HIGH);
    digitalWrite(peizoPin, LOW);
    
  }
  // delay(10);


}