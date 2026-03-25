#include <Arduino.h> // PlatformIO에서는 이 헤더가 필수입니다.

void setup() {
  // DDRB 레지스터 주소 (0x24) 직접 접근 -> 5번 비트 출력 설정
  //*((volatile unsigned char *)0x24) |= 0x20;

  
  *((volatile unsigned char *)0x24) |= 0x30;
}

void loop() {

  *((volatile unsigned char *)0x25) |= 0x10;  // 12번 ON
  *((volatile unsigned char *)0x25) |= 0x20;  // 13번 ON
  delay(500);

  *((volatile unsigned char *)0x25) &= ~0x10; // 12번 OFF
  
  delay(500);

  *((volatile unsigned char *)0x25) |= 0x10;  // 12번 ON
  *((volatile unsigned char *)0x25) &= ~0x20; // 13번 OFF
  delay(500);

  *((volatile unsigned char *)0x25) &= ~0x10; // 12번 OFF
  
  delay(500);

  // // PORTB 레지스터 (0x25) -> LED ON
  // *((volatile unsigned char *)0x25) |= 0x20;
  
  // delay(1000);

  // // PORTB 레지스터 (0x25) -> LED OFF
  // *((volatile unsigned char *)0x25) &= ~0x20;
  
  
  // delay(1000);
}


// #include <Arduino.h>

// // put function declarations here:
// int myFunction(int, int);
// void main(){}
// void setup() {
//   // put your setup code here, to run once:
//   int result = myFunction(2, 3);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }

