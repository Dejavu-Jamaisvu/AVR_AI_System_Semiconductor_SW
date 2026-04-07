//include/my_servo.h
#ifndef MY_SERVO_H_
#define MY_SERVO_H_

#include <avr/io.h>

// --- [ 서보 모터 각도별 타이머 틱 값 ] ---
// 계산식: (Target_us * 2) -> 0.5us 단위 기준
#define ANGLE_0            1088    // 544us
#define ANGLE_45           2016    // 1008us
#define ANGLE_90           2944    // 1472us
#define ANGLE_135          3872    // 1936us
#define ANGLE_180          4800    // 2400us

#define SERVO_TOP          39999   // 20ms 주기 (16MHz, 8분주)
#define SERVO_MOVE_DURATION 25     // 0.5초 신호 유지 (50Hz 기준)

// --- [ 함수 선언 ] ---
void Servo_Init(void);
void Servo_Set_Raw(uint16_t ticks);
void Servo_Detach(void);

// 특정 각도로 이동 (예: Servo_Write(45);)
void Servo_Write(uint16_t angle);

#endif