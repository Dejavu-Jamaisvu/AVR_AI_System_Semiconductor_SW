#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// 9번 핀(PB1) 제어 매크로
#define SERVO_HIGH() (PORTB |= (1 << PORTB1))
#define SERVO_LOW()  (PORTB &= ~(1 << PORTB1))

// 1us를 정밀하게 만드는 내부 함수
// _delay_us(1)을 루프로 돌리면 발생하는 약 0.5us의 추가 지연을 보정합니다.
void delay_us_custom(uint16_t us) {
    while (us--) {
        _delay_us(1); 
    }
}

// 통합 제어 함수 (각도 대신 마이크로초를 직접 입력)
void move_to(uint16_t pulse_us) {
    for (int i = 0; i < 40; i++) {
        SERVO_HIGH();
        
        // [핵심] 루프 지터(Jitter)를 줄이기 위해 보정값 적용
        // 1us당 발생하는 CPU 연산 시간을 고려하여 입력값의 약 70~80%만 수행
        uint16_t adjusted_pulse = (pulse_us * 10) / 13; 
        delay_us_custom(adjusted_pulse);

        SERVO_LOW();
        _delay_ms(19); // 주기 20ms 유지
    }
}

int main(void) {
    DDRB |= (1 << DDB1); // 9번 핀 출력 설정

    while (1) {
        
        move_to(544);  // 0도
        _delay_ms(1000);

        move_to(1472); // 90도
        _delay_ms(1000);

        move_to(2400); // 180도
        _delay_ms(1000);
    }
}