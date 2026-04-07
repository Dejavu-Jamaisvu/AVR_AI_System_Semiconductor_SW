#include <Arduino.h>

// LED 상태를 관리하는 클래스 정의
class Flasher {
  private:
    uint8_t ledPin;               // 제어할 핀 번호
    unsigned long OnTime;         // 켜져 있을 시간 (ms)
    unsigned long OffTime;        // 꺼져 있을 시간 (ms)
    
    int ledState;                 // 현재 LED 상태 (HIGH/LOW)
    unsigned long previousMillis; // 상태가 마지막으로 바뀐 시간

  public:
    // 생성자: 핀 번호 및 주기 설정
    Flasher(int pin, unsigned long on, unsigned long off) {
        ledPin = pin;
        OnTime = on;
        OffTime = off;
        
        ledState = LOW;
        previousMillis = 0;
        
        pinMode(ledPin, OUTPUT);
    }

    // 상태 업데이트 함수 (loop에서 지속 호출)
    void update() {
        // 현재 시간 확인
        unsigned long currentMillis = millis(); 

        // LED가 켜져 있고, 설정한 켜짐 시간(OnTime)이 지났을 때
        if ((ledState == HIGH) && (currentMillis - previousMillis >= OnTime)) {
            ledState = LOW;                  // 상태를 LOW로 변경
            previousMillis = currentMillis;  // 현재 시간을 과거 시간으로 저장
            digitalWrite(ledPin, ledState);  // 실제 핀 제어
        }
        // LED가 꺼져 있고, 설정한 꺼짐 시간(OffTime)이 지났을 때
        else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime)) {
            ledState = HIGH;                 // 상태를 HIGH로 변경
            previousMillis = currentMillis;  // 현재 시간을 과거 시간으로 저장
            digitalWrite(ledPin, ledState);  // 실제 핀 제어
        }
    }
};

// 독립적인 주기를 가진 객체 생성
// led1: 13번 핀, 100ms 켜지고 400ms 꺼짐 (빠르게 깜빡임)
Flasher led1(13, 100, 400);

// led2: 12번 핀, 500ms 켜지고 500ms 꺼짐 (느리게 깜빡임)
Flasher led2(12, 500, 500);

// led3: 11번 핀, 50ms 켜지고 950ms 꺼짐 (1초에 한 번 짧게 깜빡임)
Flasher led3(11, 50, 950);

void setup() {
    // 핀 초기화는 객체 생성 시 수행되므로 생략 가능
}

void loop() {
    // 1. 각 LED의 상태 업데이트
    // 내부적으로 millis()를 검사하여 시간이 된 객체만 상태를 반전시킴
    led1.update();
    led2.update();
    led3.update();
    
    // 2. 다른 작업 추가
    // delay()가 없으므로 센서 읽기, 통신 등의 작업을 자유롭게 배치 가능
}