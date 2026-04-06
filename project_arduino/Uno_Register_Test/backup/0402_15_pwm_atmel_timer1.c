#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// --- [ 서보 모터 설정값 ] ---
#define SERVO_TOP          39999   // 20ms 주기 (8분주 기준)
#define TICKS_PER_SEC      50      // 20ms * 50 = 1000ms (1초)
#define MOVE_DURATION      25      // 0.5초 동안 신호 유지 (지터 방지)

// SG90 실측 최적값 (0.5us 단위 변환 완료)
#define ANGLE_0            1087    // 544us
#define ANGLE_90           2943    // 1472us
#define ANGLE_180          4799    // 2400us

// --- [ 전역 상태 변수 ] ---
volatile uint16_t timer_ticks = 0;

ISR(TIMER1_OVF_vect) {
    timer_ticks++;
}

// --- [ 하드웨어 제어 함수 ] ---
void Servo_Init() {
    DDRB |= (1 << DDB1); // PB1 (Digital 9) 출력

    // Mode 14: Fast PWM, TOP = ICR1, 8분주
    TCCR1A = (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = SERVO_TOP;
    
    TIMSK1 |= (1 << TOIE1); // 오버플로 인터럽트 활성화
    sei();
}

void Servo_Set_Raw(uint16_t ticks) {
    TCCR1A |= (1 << COM1A1); // Attach
    OCR1A = ticks;
}

void Servo_Detach() {
    TCCR1A &= ~(1 << COM1A1); // Detach
    PORTB &= ~(1 << PORTB1);
}

int main(void) {
    Servo_Init();

    uint16_t last_tick = 0;
    uint16_t angles[] = {ANGLE_0, ANGLE_90, ANGLE_180};
    uint8_t step = 0;
    uint8_t is_moving = 0;

    while (1) {
        uint16_t current_tick = timer_ticks;

        // 1. 상태 변화 (1초 간격 체크)
        if (!is_moving && (current_tick - last_tick >= TICKS_PER_SEC)) {
            Servo_Set_Raw(angles[step]);
            
            step = (step + 1) % 3;
            last_tick = current_tick;
            is_moving = 1;
        }

        // 2. 지터 방지 (0.5초 후 신호 차단)
        if (is_moving && (current_tick - last_tick >= MOVE_DURATION)) {
            Servo_Detach();
            is_moving = 0;
        }
        
        // 3. 메인 루프가 완전히 비어있음 -> 여기서 센서 데이터 읽기 등을 수행
    }
}