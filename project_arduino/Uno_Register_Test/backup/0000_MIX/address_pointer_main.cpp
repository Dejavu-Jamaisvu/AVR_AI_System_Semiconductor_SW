

// led 2개
#include <Arduino.h> // PlatformIO에서는 이 헤더가 필수입니다.

void setup() {
  // DDRB 레지스터 주소 (0x24) 직접 접근 -> 5번 비트 출력 설정
  //*((volatile unsigned char *)0x24) |= 0x20;

  
  *((volatile unsigned char *)0x24) |= 0x30;
}

unsigned long prev12 = 0; // 12번 LED의 마지막 변경 시간
unsigned long prev13 = 0; // 13번 LED의 마지막 변경 시간

const long interval12 = 500;  // 12번은 0.5초마다 반전
const long interval13 = 1000; // 13번은 1초마다 반전 (전체 주기 2초)

void loop() {

  //delay 사용하지 않고 millis() 사용======================
  unsigned long currentMillis = millis();

  // 12번 LED 제어 (500ms 마다 토글)
  if (currentMillis - prev12 >= interval12) {
    prev12 = currentMillis;
    *((volatile unsigned char *)0x25) ^= 0x10; 
  }

  // 13번 LED 제어 (1000ms 마다 토글)
  if (currentMillis - prev13 >= interval13) {
    prev13 = currentMillis;
    *((volatile unsigned char *)0x25) ^= 0x20;
  }

  //과제 - 500ms LED추가=====================================
  // *((volatile unsigned char *)0x25) |= 0x10;  // 12번 ON
  // *((volatile unsigned char *)0x25) |= 0x20;  // 13번 ON
  // delay(500);

  // *((volatile unsigned char *)0x25) &= ~0x10; // 12번 OFF
  
  // delay(500);

  // *((volatile unsigned char *)0x25) |= 0x10;  // 12번 ON
  // *((volatile unsigned char *)0x25) &= ~0x20; // 13번 OFF
  // delay(500);

  // *((volatile unsigned char *)0x25) &= ~0x10; // 12번 OFF
  
  // delay(500);


  //수업=====================================================
  // // PORTB 레지스터 (0x25) -> LED ON
  // *((volatile unsigned char *)0x25) |= 0x20;
  
  // delay(1000);

  // // PORTB 레지스터 (0x25) -> LED OFF
  // *((volatile unsigned char *)0x25) &= ~0x20;
  
  
  // delay(1000);
}

