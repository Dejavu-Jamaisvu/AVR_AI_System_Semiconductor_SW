#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h> // 인터럽트 추가

#define DHT_PIN   PD2       // 아두이노 디지털 2번 핀은 ATmega328P의 PORTD 2번 비트임
#define DHT_DDR   DDRD      // 데이터 방향 레지스터 (입력/출력 설정)
#define DHT_PORT  PORTD     // 출력 레지스터 (HIGH/LOW 출력)
#define DHT_PIN_REG PIND   // 입력 레지스터 (현재 핀의 전압 상태 읽기)

uint8_t data[5];
volatile int8_t bit_idx = -1;      // 현재 읽고 있는 비트 위치 (0~39)
volatile uint8_t data_ready = 0;   // 데이터 수집 완료 플래그
volatile uint16_t ms_count = 0;    // ms 단위 시간 측정용
volatile uint8_t request_flag = 0; // 2초 주기 요청 플래그

typedef enum { IDLE, START_LOW, START_HIGH, RESPONSE, READ_DATA } dht_state_t;
volatile dht_state_t state = IDLE;

void UART_init() {
    // Baud Rate 설정: 16MHz 클럭 기준, 9600bps 속도를 위해 103 대입
    // 16,000,000/9600*16 = 104.166.. //0 ~ 103
    UBRR0H = (unsigned char)(103 >> 8); // 103을 밀어서 상위 비트 추출 (결과는 0)
    UBRR0L = (unsigned char)103;  //하위 8비트
    
    // 송신부(TX) 활성화
    UCSR0B = (1 << TXEN0); //0~7번 비트 중 3번(TXEN0) 활성화
    
    //데이터 비트 수 설정
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  //8-bit (표준) //정지 비트 기본값 0은 1개
}

void UART_transmit(char c) {
    // 송신 버퍼가 비워질 때까지 무한 대기 (UDRE0 비트가 1이 되면 비워진 것)
    while (!(UCSR0A & (1 << UDRE0))); //UCSR0A의 UDRE0(5번) 1이면 비워진 것
    
    // 버퍼에 문자 기록 -> 실제 전송 시작
    UDR0 = c; //데이터가 드나드는 통로이자 버퍼 
    //UDR0에서 Shift Register로 데이터가 넘어가 비게 되면 UDRE0(5번 비트) 1
}

void UART_print(const char* str) {
    while (*str) UART_transmit(*str++);
}

void UART_print_binary(uint8_t b) {
    for (int i = 7; i >= 0; i--) {
        UART_transmit((b & (1 << i)) ? '1' : '0');
    }
}

void init_peripherals() {
    // Timer0: 1ms 시스템 틱 생성 (CTC 모드)
    TCCR0A = (1 << WGM01); // CTC 모드 설정 //OCR0A에 설정한 목표 숫자까지만 세고 0
    TCCR0B = (1 << CS01) | (1 << CS00); //CS01, CS00=1: 64분주

    // 64/16,000,000 = 0.000004초 = 4us
    // 목표시간 1ms(1,000us)
    // 1,000us/4us = 250번 (0~249)
    OCR0A = 249;
    TIMSK0 |= (1 << OCIE0A); // Timer0 비교 일치 인터럽트 활성화

    // Timer1: 0.5us 단위 펄스 측정 (Normal 모드)
    TCCR1B |= (1 << CS11); // 8분주

    // PCINT2: PD2 핀 변화 감지 설정
    PCICR |= (1 << PCIE2); //PCIE2: 2조 (Port D: 디지털 0~7번)
    PCMSK2 |= (1 << PCINT18); // PD2(디지털 2번) 핀만 감시
}

// Non-blocking 시작 신호 함수
void request_dht11() {
    if (state != IDLE) return; 
    for(int i=0; i<5; i++) data[i] = 0;
    bit_idx = -1;

    DHT_DDR |= (1 << DHT_PIN); // 출력모드
    DHT_PORT &= ~(1 << DHT_PIN); // LOW(0V)
    state = START_LOW;
    ms_count = 0; // Timer0 ISR에서 18ms를 세기 시작함
}

// Timer0: ms 단위 시간 관리
ISR(TIMER0_COMPA_vect) {
    static uint16_t poll_timer = 0;
    
    // 2초마다 측정 요청
    if (++poll_timer >= 2000) {
        poll_timer = 0;
        request_flag = 1;
    }

    // 시작 신호 18ms 유지
    if (state == START_LOW) {
        if (++ms_count >= 18) {
            DHT_PORT |= (1 << DHT_PIN); // HIGH(5V)
            DHT_DDR &= ~(1 << DHT_PIN); // 입력모드 전환
            state = START_HIGH;
            TCNT1 = 0; // Timer1 리셋
        }
    }
}

// PCINT2: 신호 엣지 감지 및 데이터 읽기
ISR(PCINT2_vect) {
    uint16_t pulse_time = TCNT1; //전 인터럽트부터 지금까지 흐른시간 
    TCNT1 = 0; // 다음 엣지 측정을 위해 리셋

    uint8_t pin_state = DHT_PIN_REG & (1 << DHT_PIN);//지금 핀이 HIGH(1)인지 LOW(0)인지

    if (state == START_HIGH) {
        if (!pin_state) state = RESPONSE; //LOW되면
    } 
    else if (state == RESPONSE) {
        if (!pin_state && pulse_time > 100) {
            state = READ_DATA;
            bit_idx = 0;
        }
    } 
    else if (state == READ_DATA) {
        if (!pin_state) {
            // 8분주 기준 1us = 2 count. 기준점 50us = 100 count.
            // 0인 경우: 26~28us 유지 / 1인 경우: 약 70us 유지
            if (pulse_time > 100) { 
                data[bit_idx / 8] |= (1 << (7 - (bit_idx % 8)));
            }
            bit_idx++;

            if (bit_idx >= 40) {
                state = IDLE;
                data_ready = 1;
            }
        }
    }
}



int main(void) {
    UART_init();
    init_peripherals();
    sei(); // 전역 인터럽트 활성화
    char buffer[64];
    
    UART_print("--- DHT11 PCINT2/Timer Mode ---\r\n");

    while (1) {
        if (request_flag) {
            request_flag = 0;
            request_dht11(); // 데이터 요청 시작
        }

        if (data_ready) {
            data_ready = 0;

            UART_print("BIT STREAM: ");
            for (int i = 0; i < 5; i++) {
                UART_print_binary(data[i]);
                UART_transmit(' ');
            }
            UART_print("\r\n");

            // 체크섬 검사
            if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
                sprintf(buffer, "[Hum: %d%%, Temp: %dC, Checksum: %02X]\r\n", 
                        data[0], data[2], data[4]);
                UART_print(buffer);
            } else {
                UART_print("ERROR: Checksum Mismatch!\r\n");
            }
            UART_print("\r\n");
        }
    }
    return 0;
}