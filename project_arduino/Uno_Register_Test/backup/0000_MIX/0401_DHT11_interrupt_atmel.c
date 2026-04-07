#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h> // 인터럽트 사용 추가

#define DHT_PIN  PD2       // 아두이노 디지털 2번 핀은 ATmega328P의 PORTD 2번 비트임
#define DHT_DDR  DDRD      // 데이터 방향 레지스터 (입력/출력 설정)
#define DHT_PORT PORTD     // 출력 레지스터 (HIGH/LOW 출력)
#define DHT_PIN_REG PIND   // 입력 레지스터 (현재 핀의 전압 상태 읽기)


uint8_t data[5];


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

int read_dht11() {
    uint16_t timeout;
    
    // 초기화
    for(int i=0; i<5; i++) data[i] = 0;

    DHT_DDR |= (1 << DHT_PIN); // 출력모드
    DHT_PORT &= ~(1 << DHT_PIN); // LOW(0V)
    _delay_ms(18);
    
    DHT_PORT |= (1 << DHT_PIN);  // HIGH(5V)
    _delay_us(40); // 20~40us
    DHT_DDR &= ~(1 << DHT_PIN);   // 입력모드

    // 센서는 응답으로 LOW(80us) -> HIGH(80us) 신호를 차례로 보냄
    timeout = 0;
    while (DHT_PIN_REG & (1 << DHT_PIN)) // LOW가 올 때까지 대기
        if (++timeout > 1000) return 0;  // 오래 걸리면 센서 연결 불량

    timeout = 0;
    while (!(DHT_PIN_REG & (1 << DHT_PIN))) // LOW(80us)
        if (++timeout > 1000) return 0;

    timeout = 0;
    while (DHT_PIN_REG & (1 << DHT_PIN)) // HIGH(80us)
        if (++timeout > 1000) return 0;

    for (int i = 0; i < 40; i++) {
        // LOW로 시작 //LOW 구간이 끝날 때까지 대기
        while (!(DHT_PIN_REG & (1 << DHT_PIN))); 
        
        // 0인 경우: 26~28us 유지 / 1인 경우: 약 70us 유지
        _delay_us(35); // 중간 지점
        
        if (DHT_PIN_REG & (1 << DHT_PIN)) {
            // 35us 뒤에 HIGH면 1
            // i/8은 현재 몇 번째 바이트인지(0~4), (7-i%8)은 바이트 내의 비트 위치를 의미
            data[i / 8] |= (1 << (7 - (i % 8)));
            
            // LOW로 떨어질 때까지 기다려야 다음 비트
            while (DHT_PIN_REG & (1 << DHT_PIN)); 
        }
        // 35us 뒤에 이미 LOW면 배열 0이 그대로 유지
    }
    
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        return 1;
    } else {
        return 0;
    }
}




volatile uint16_t timer_ms = 0; // 1ms마다
volatile uint8_t read_flag = 0;  // 2초마다

// 타이머0번 비교 일치(COMPA) 인터럽트 서비스 루틴 (1ms마다 자동 호출)
ISR(TIMER0_COMPA_vect) { 
    timer_ms++;
    if (timer_ms >= 2000) { // 2000ms = 2초
        read_flag = 1; 
        timer_ms = 0;
    }
}

void timer0_init() {

    TCCR0A = (1 << WGM01); // CTC 모드 설정 //OCR0A에 설정한 목표 숫자까지만 세고 0
    TCCR0B = (1 << CS01) | (1 << CS00); //CS01, CS00=1: 64분주

    // 64/16,000,000 = 0.000004초 = 4us
    // 목표시간 1ms(1,000us)
    // 1,000us/4us = 250번 (0~249)
    OCR0A = 249;

    TIMSK0 |= (1 << OCIE0A); //OCIE0A 비트 1: 목표(249) 도달 시 인터럽트 알람벨
}





int main(void) {
    UART_init();
    timer0_init(); // 타이머 설정 시작!
    sei();         // "이제 알람 소리 들어도 돼!" (전역 인터럽트 허용)
    char buffer[64];
    
    UART_print("--- DHT11 Timer Interrupt Mode ---\r\n");

    while (1) {
        // read_flag가 1일 때만 센서를 읽어 (2초마다 한 번씩)
        if (read_flag == 1) {
            read_flag = 0;

            if (read_dht11()) {
                UART_print("BIT STREAM: ");
                for (int i = 0; i < 5; i++) {
                    UART_print_binary(data[i]);
                    UART_transmit(' ');
                }
                UART_print("\r\n");

                sprintf(buffer, "[Hum: %d%%, Temp: %dC, Checksum: %02X]\r\n", 
                        data[0], data[2], data[4]);
                UART_print(buffer);
                UART_print("\r\n");
            } else {
                UART_print("ERROR: Sensor Read Failed!\r\n");
            }
        }
        //_delay_ms(2000);
    }
    
    return 0;
}