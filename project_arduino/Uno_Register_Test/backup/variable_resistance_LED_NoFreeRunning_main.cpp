#include <Arduino.h>
#include <stdio.h>
#include <util/delay.h>

// 1. UART 초기화 (이미지의 9600 반영)
void UART_init(unsigned int baud) {
    unsigned int baud_rate = F_CPU / 16 / baud - 1;
    UBRR0H = (unsigned char)(baud_rate >> 8);
    UBRR0L = (unsigned char)baud_rate;
    UCSR0B = (1 << TXEN0);              
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); 
}

void UART_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_print(const char* str) {
    while (*str) UART_transmit(*str++);
}

// 2. ADC 초기화 (Free Running 모드 사용 안 함)
void ADC_init(unsigned char channel) {
    ADMUX = (1 << REFS0);           // AVCC(5V) 기준 전압
    
    // [설명] ADATE 비트를 설정하지 않음 -> Free Running 모드 비활성화
    // 오직 ADC 활성화(ADEN)와 128 분주(125KHz)만 설정
    ADCSRA = (1 << ADEN) | 0x07;    
    
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // ADC0(PC0) 채널 선택
}

// 3. ADC 싱글 변환 읽기 함수
int read_ADC_Single() {
    // [설명] 프리러닝이 아니므로, 매번 ADSC를 1로 만들어 변환을 시작해야 함
    ADCSRA |= (1 << ADSC);          
    
    // 변환이 끝날 때까지 대기
    while (ADCSRA & (1 << ADSC));   
    return ADC; // 0~1023 결과 반환
}

int main(void) {
    // 11(PB3), 12(PB4), 13(PB5)번 핀 출력 설정
    DDRB |= (1 << DDB3) | (1 << DDB4) | (1 << DDB5);

    UART_init(9600);
    ADC_init(0); // A0 초기화
    
    // 시리얼 모니터에 모드 명시
    UART_print("--- ADC Single Conversion Mode (NOT Free Running) ---\r\n");

    char buffer[50];

    while (1) {
        // [설명] 프리러닝이 아니므로 100ms마다 한 번씩 '직접' 호출해서 읽음
        int value = read_ADC_Single(); 

        // LED 레벨 미터 (11, 12, 13번)
        PORTB &= ~((1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5));
        if (value > 100) PORTB |= (1 << PORTB3);
        if (value > 450) PORTB |= (1 << PORTB4);
        if (value > 850) PORTB |= (1 << PORTB5);

        // UART 데이터 전송 (100ms 간격)
        sprintf(buffer, "ADC Value: %d (Single Mode)\r\n", value);
        UART_print(buffer);

        // 100ms 대기
        _delay_ms(100); 
    }
    return 0;
}