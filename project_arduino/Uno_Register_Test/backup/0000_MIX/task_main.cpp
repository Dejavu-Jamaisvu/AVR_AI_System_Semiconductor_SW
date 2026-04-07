

#include <Arduino.h>
#include <TaskScheduler.h> //platformio.ini에 작성필요

Scheduler runner;

// 태스크 콜백 함수 선언
void blinkLED1();
void blinkLED2();

// 태스크 객체 생성 (주기ms, 반복횟수, 콜백함수, 스케줄러포인터, 자동시작)
Task task1(1000, TASK_FOREVER, &blinkLED1, &runner, true); // 1초 주기
Task task2(300, TASK_FOREVER, &blinkLED2, &runner, true);  // 0.3초 주기

void blinkLED1() {
    digitalWrite(13, !digitalRead(13)); // 내장 LED 토글
}

void blinkLED2() {
    digitalWrite(12, !digitalRead(12)); // 외부 LED 토글
}

void setup() {
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    runner.startNow(); // 스케줄러 시작
}

void loop() {
    runner.execute(); // 스케줄러 실행 루프 (delay 사용 금지)
}