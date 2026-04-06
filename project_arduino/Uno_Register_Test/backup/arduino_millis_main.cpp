#include <Arduino.h>

// 1. LED 상태를 보관할 구조체 정의
typedef struct {
    uint8_t pin;               // 핀 번호
    unsigned long onTime;      // 켜져 있을 시간 (ms)
    unsigned long offTime;     // 꺼져 있을 시간 (ms)
    uint8_t state;             // 현재 상태 (HIGH/LOW)
    unsigned long prevMillis;  // 마지막으로 상태가 바뀐 시간
} Flasher;

// 2. 구조체 초기화 함수 (C++의 생성자 역할)
void Flasher_Init(Flasher* f, uint8_t pin, unsigned long on, unsigned long off) {
    f->pin = pin;
    f->onTime = on;
    f->offTime = off;
    f->state = LOW;
    f->prevMillis = 0;
    
    pinMode(f->pin, OUTPUT);
}

// 3. 상태 업데이트 함수 (구조체 포인터를 받아 상태 변경)
void Flasher_Update(Flasher* f, unsigned long currentMillis) {
    // 켜져 있고, 끄는 시간이 되었을 때
    if ((f->state == HIGH) && (currentMillis - f->prevMillis >= f->onTime)) {
        f->state = LOW;
        f->prevMillis = currentMillis;
        digitalWrite(f->pin, f->state);
    }
    // 꺼져 있고, 켜는 시간이 되었을 때
    else if ((f->state == LOW) && (currentMillis - f->prevMillis >= f->offTime)) {
        f->state = HIGH;
        f->prevMillis = currentMillis;
        digitalWrite(f->pin, f->state);
    }
}

// 구조체 변수(인스턴스) 선언
Flasher led1;
Flasher led2;
Flasher led3;

void setup() {
    // 각 LED 초기화 (주소 전달 연산자 '&' 사용)
    Flasher_Init(&led1, 13, 100, 400);   // 13번 핀
    Flasher_Init(&led2, 12, 500, 500);   // 12번 핀
    Flasher_Init(&led3, 11, 50, 950);    // 11번 핀
}

void loop() {
    // 성능 최적화: loop 한 번당 millis()는 한 번만 호출하여 전달
    unsigned long currentMillis = millis(); 
    
    // 각 LED 상태 업데이트
    Flasher_Update(&led1, currentMillis);
    Flasher_Update(&led2, currentMillis);
    Flasher_Update(&led3, currentMillis);
    
    // 이 위치에 지연 없이 다른 작업 추가 가능
}



