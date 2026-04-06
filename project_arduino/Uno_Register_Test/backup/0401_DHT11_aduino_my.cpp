#include <Arduino.h>

#define DHTPIN 2
uint8_t data[5];

// 함수 선언 (PlatformIO 에러 방지)
bool readDHT11();
void printBinary(uint8_t b);

void setup() {
  Serial.begin(9600);
  Serial.println(F("--DHT11 Raw Bit Stream Test--"));
}

void loop() {
  if (readDHT11()) {
    Serial.print(F("Raw Bits: "));
    
    // 5바이트를 각각 비트로 출력
    for (int i = 0; i < 5; i++) {
      printBinary(data[i]);
      Serial.print(" "); 
    }
    Serial.println();

    // 해석된 값 출력
    Serial.print(F("Humidity: ")); Serial.print(data[0]); Serial.print("."); Serial.print(data[1]);
    Serial.print(F("%  Temp: ")); Serial.print(data[2]); Serial.print("."); Serial.print(data[3]);
    Serial.print(F("C  Checksum: ")); Serial.println(data[4], HEX);
  } else {
    Serial.println(F("Read Error - Check Wiring"));
  }

  delay(2000);
}

// 8비트 데이터를 '00000000' 형태로 출력하는 보조 함수
void printBinary(uint8_t b) {
  for (int i = 7; i >= 0; i--) {
    Serial.print(bitRead(b, i));
  }
}

bool readDHT11() {
  for (int i = 0; i < 5; i++) data[i] = 0;

  // 1. 시작 신호
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHTPIN, INPUT);

  // 2. 센서 응답 확인
  unsigned long timeout = micros();
  while(digitalRead(DHTPIN) == HIGH) if(micros() - timeout > 100) return false;
  while(digitalRead(DHTPIN) == LOW)  if(micros() - timeout > 200) return false;
  while(digitalRead(DHTPIN) == HIGH) if(micros() - timeout > 300) return false;

  // 3. 40비트 데이터 수집
  for (int i = 0; i < 40; i++) {
    while(digitalRead(DHTPIN) == LOW); 
    
    unsigned long startTime = micros();
    while(digitalRead(DHTPIN) == HIGH);
    
    // 40us보다 길면 1, 짧으면 0
    if ((micros() - startTime) > 40) {
      data[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  // 4. 체크섬 검증
  return (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF));
}