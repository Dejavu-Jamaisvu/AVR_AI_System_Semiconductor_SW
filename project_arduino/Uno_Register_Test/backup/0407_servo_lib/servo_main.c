#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "my_servo.h"

// 틱 변수는 라이브러리와 타입을 맞추는 것이 안전합니다.
volatile uint16_t timer_ticks = 0;

ISR(TIMER1_OVF_vect) {
    timer_ticks++;
}

int main(void) {
    // 1. 하드웨어 초기화 (D9핀 설정 등)
    Servo_Init();
    
    // 2. 인터럽트 활성화
    sei();

    uint16_t test_angles[] = {ANGLE_0, ANGLE_45, ANGLE_90, ANGLE_135, ANGLE_180};
    uint8_t index = 0;
    
    // [수정] 초기값을 현재 틱에서 충분히 뺀 값으로 설정하여 즉시 시작하게 함
    uint16_t last_move_tick = -75; 
    uint8_t is_moving = 0;

    while (1) {
        // 인터럽트 변수 안전하게 읽기
        uint16_t now;
        cli();
        now = timer_ticks;
        sei();

        // 1. 다음 각도로 이동 (1.5초 간격)
        if (!is_moving && (uint16_t)(now - last_move_tick) >= 75) {
            Servo_Set_Raw(test_angles[index]);
            
            last_move_tick = now;
            is_moving = 1;
            index = (index + 1) % 5;
        }

        // 2. 지터 방지 (0.5초 후 신호 차단)
        if (is_moving && (uint16_t)(now - last_move_tick) >= 25) {
            Servo_Detach();
            is_moving = 0;
        }
    }
}