//src/srclib/my_servo.c
#include "my_servo.h"

void Servo_Init(void) {
    DDRB |= (1 << DDB1); // PB1 (D9) 출력 설정
    
    // Mode 14: Fast PWM, TOP = ICR1
    TCCR1A = (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // 8분주
    ICR1 = SERVO_TOP;

    // [추가] 타이머 1 오버플로 인터럽트 활성화
    // 이 줄이 없으면 메인의 ISR(TIMER1_OVF_vect)이 실행되지 않아 
    // timer_ticks가 0에 멈춰있게 됩니다.
    TIMSK1 |= (1 << TOIE1); 
}

void Servo_Set_Raw(uint16_t ticks) {
    TCCR1A |= (1 << COM1A1); // PWM 출력 연결
    OCR1A = ticks;
}

void Servo_Detach(void) {
    TCCR1A &= ~(1 << COM1A1); // PWM 출력 해제
    PORTB &= ~(1 << PORTB1);   // LOW 상태 유지
}

/**
 * @brief 0, 45, 90, 135, 180도 중 선택하여 이동
 */
void Servo_Write(uint16_t angle) {
    switch(angle) {
        case 0:   Servo_Set_Raw(ANGLE_0);   break;
        case 45:  Servo_Set_Raw(ANGLE_45);  break;
        case 90:  Servo_Set_Raw(ANGLE_90);  break;
        case 135: Servo_Set_Raw(ANGLE_135); break;
        case 180: Servo_Set_Raw(ANGLE_180); break;
        default:  break; // 정의되지 않은 각도는 무시
    }
}