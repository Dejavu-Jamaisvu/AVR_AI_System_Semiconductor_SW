#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

// --- [ 커스텀 정밀 지연 매크로 ] ---
// CPU 사이클을 직접 소모하여 마이크로초 단위 타이밍 생성
#define DELAY_US(us) __builtin_avr_delay_cycles((F_CPU / 1000000UL) * (us))
#define DELAY_MS(ms) __builtin_avr_delay_cycles((F_CPU / 1000UL) * (ms))

// --- [ 설정 및 핀 정의 ] ---
#define BAUDRATE 9600
#define MY_UBRR  (F_CPU/16/BAUDRATE - 1)

#define DHT11_PIN     PD2
#define SERVO_PIN     PB1 // OC1A 핀 (Timer1 PWM 출력)
#define TICKS_2S      100 // 20ms * 100 = 2초

#define ANGLE_0       1087  // 닫힘 (0.544ms)
#define ANGLE_90      2943  // 열림 (1.472ms)

// --- [ 전역 변수 ] ---
volatile uint32_t timer1_ticks = 0;
char uart_buffer[64];

// --- [ UART 통신 초기화 및 송신 ] ---
void UART_Init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0); // 송신 활성화
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8비트 데이터 포맷
}

void UART_PrintString(const char* str) {
    while (*str) {
        while (!(UCSR0A & (1 << UDRE0))); // 버퍼가 비었는지 확인
        UDR0 = *str++;
    }
}

// --- [ DHT11 정밀 읽기 및 체크섬 검증 ] ---
uint8_t DHT11_Read_Data(uint8_t *rel_h) {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint8_t i, j;

    DDRD |= (1 << DHT11_PIN);
    PORTD &= ~(1 << DHT11_PIN);
    DELAY_MS(18); 
    PORTD |= (1 << DHT11_PIN);
    DELAY_US(30);
    DDRD &= ~(1 << DHT11_PIN); // 입력 모드로 전환

    DELAY_US(40);
    if (!(PIND & (1 << DHT11_PIN))) {
        while (!(PIND & (1 << DHT11_PIN))); // 80us LOW 대기
        while (PIND & (1 << DHT11_PIN));    // 80us HIGH 대기

        for (i = 0; i < 5; i++) {
            for (j = 0; j < 8; j++) {
                while (!(PIND & (1 << DHT11_PIN))); // 비트 시작 대기
                DELAY_US(35); // 0과 1을 구분하는 임계 시간
                if (PIND & (1 << DHT11_PIN)) {
                    data[i] |= (1 << (7 - j));
                    while (PIND & (1 << DHT11_PIN)); // 비트 종료 대기
                }
            }
        }

        if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) == data[4]) {
            *rel_h = data[0]; 
            return 1; // 성공
        }
    }
    return 0; // 실패
}

// --- [ Timer1 오버플로 인터럽트: 20ms 주기 ] ---
ISR(TIMER1_OVF_vect) {
    timer1_ticks++;
}

// --- [ 시스템 통합 초기화 ] ---
void Init_Systems() {
    UART_Init(MY_UBRR);
    
    // 9번 핀(PB1) 출력 설정
    DDRB |= (1 << SERVO_PIN);
    
    // Timer1 설정: Fast PWM 모드 14 (ICR1을 TOP으로 사용)
    // 비반전 모드(COM1A1), 8분주(CS11)
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = 39999;    // 20ms 주기 (16MHz / 8분주 / 50Hz - 1)
    OCR1A = ANGLE_0; // 초기 위치: 창문 닫힘
    
    TIMSK1 |= (1 << TOIE1); // 타이머 오버플로 인터럽트 활성화
    sei(); // 전역 인터럽트 허용
    
    UART_PrintString("\r\n[SYSTEM] Smart Window Active (No-Delay Mode)\r\n");
}

int main(void) {
    Init_Systems();
    
    uint32_t last_check_time = 0;
    uint8_t current_humidity = 0;
    uint8_t window_is_open = 0;

    while (1) {
        // 논블로킹 타이머 2초가 지났는지 확인
        if (timer1_ticks - last_check_time >= TICKS_2S) {
            last_check_time = timer1_ticks;

            if (DHT11_Read_Data(&current_humidity)) {
                // 자동 제어 로직 (히스테리시스 적용)
                if (current_humidity >= 70 && !window_is_open) {
                    OCR1A = ANGLE_90; // 하드웨어가 즉시 펄스 폭 변경
                    window_is_open = 1;
                    UART_PrintString(">> Event: High Humidity. Opening window!\r\n");
                } 
                else if (current_humidity <= 50 && window_is_open) {
                    OCR1A = ANGLE_0;
                    window_is_open = 0;
                    UART_PrintString(">> Event: Normal Humidity. Closing window!\r\n");
                }

                // 실시간 모니터링 출력
                sprintf(uart_buffer, "Sensor -> Humidity: %d%% | Status: %s\r\n", 
                        current_humidity, window_is_open ? "OPEN" : "CLOSED");
                UART_PrintString(uart_buffer);
            } else {
                UART_PrintString("[ERROR] Checksum Mismatch or Sensor Timeout.\r\n");
            }
        }
        
    }
}