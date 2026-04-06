#include <Arduino.h>
#include <SoftwareSerial.h>

// SoftwareSerial 객체 생성 (RX핀, TX핀)
// RX: 2번 핀 (상대방의 TX가 연결됨)
// TX: 3번 핀 (상대방의 RX가 연결됨)
SoftwareSerial mySerial(2, 3); 

void setup() {
    // 1. 하드웨어 시리얼 (PC와 연결)
    Serial.begin(9600);
    
    // 2. 소프트웨어 시리얼 (외부 모듈과 연결)
    // 주의: SoftwareSerial은 9600, 19200, 38400 정도까지만 안정적입니다.
    // 115200은 데이터가 깨질 확률이 매우 높습니다.
    mySerial.begin(9600); 
    
    Serial.println("SoftwareSerial Test Started");
}

void loop() {
    // 외부 모듈 -> 아두이노 -> PC
    if (mySerial.available()) {
        char c = mySerial.read();
        Serial.write(c);
    }

    // PC -> 아두이노 -> 외부 모듈
    if (Serial.available()) {
        char c = Serial.read();
        mySerial.write(c);
    }
}