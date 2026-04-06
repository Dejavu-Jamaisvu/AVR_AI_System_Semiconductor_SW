//UART출력있고 동작됨
#include <Arduino.h>
#include <stdio.h>

// 1. UART 초기화 (9600 보레이트 설정)
void UART_init(unsigned int baud) {
    unsigned int baud_rate = F_CPU / 16 / baud - 1;
    UBRR0H = (unsigned char)(baud_rate >> 8);
    UBRR0L = (unsigned char)baud_rate;
    UCSR0B = (1 << TXEN0);              // 송신(TX) 활성화
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8비트 데이터 포맷
}

// 2. 한 문자 전송 함수
void UART_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));   // 송신 버퍼가 비어있을 때까지 대기
    UDR0 = data;
}

// 3. 문자열 전송 함수 (시리얼 모니터 확인용)
void UART_print(const char* str) {
    while (*str) {
        UART_transmit(*str++);
    }
}

// 4. ADC 초기화
void ADC_init(unsigned char channel) {
    ADMUX = (1 << REFS0); 
    ADCSRA = (1 << ADEN) | 0x07; 
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);          
}

// 5. ADC 읽기
int read_ADC(void) {
    while (ADCSRA & (1 << ADSC));   
    return ADC; 
}

int main(void) {
    // 11(PB3), 12(PB4), 13(PB5)번 핀 출력 설정
    DDRB |= (1 << DDB3) | (1 << DDB4) | (1 << DDB5);

    UART_init(9600); // 시리얼 9600 시작
    ADC_init(0);     // A0 핀 사용
    
    UART_print("--- ADC Level Meter System Start ---\r\n");

    char buffer[50]; // 문자열 저장을 위한 버퍼

    while (1) {
        int value = read_ADC(); 
        ADCSRA |= (1 << ADSC);  

        // 모든 LED 끄기
        PORTB &= ~((1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5));

        // 레벨 계산 및 UART 출력을 위한 변수
        int level = 0;
        if (value > 100) { PORTB |= (1 << PORTB3); level = 1; }
        if (value > 450) { PORTB |= (1 << PORTB4); level = 2; }
        if (value > 850) { PORTB |= (1 << PORTB5); level = 3; }

        // UART로 현재 값과 레벨 전송
        sprintf(buffer, "ADC: %d | Level: %d\r\n", value, level);
        UART_print(buffer);

        _delay_ms(200); // 너무 빠르면 모니터링이 힘드므로 0.2초 간격 출력
    }
    return 0;
}



//UART출력은없고 동작됨
// #include <Arduino.h>

// // ADC 초기화 (이미지의 125KHz 설정 반영)
// void ADC_init(unsigned char channel) {
//     ADMUX = (1 << REFS0);           // AVCC 기준 전압
//     ADCSRA = (1 << ADEN) | 0x07;    // ADC 활성화 및 128 분주
//     ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
//     ADCSRA |= (1 << ADSC);          
// }

// // ADC 읽기
// int read_ADC(void) {
//     while (ADCSRA & (1 << ADSC));   
//     return ADC; 
// }

// int main(void) {
//     // 11(PB3), 12(PB4), 13(PB5)번 핀을 출력으로 설정
//     DDRB |= (1 << DDB3) | (1 << DDB4) | (1 << DDB5);

//     ADC_init(0); // 가변저항이 꽂힌 A0(PC0) 사용

//     while (1) {
//         int value = read_ADC(); // 가변저항 값 읽기 (0~1023)
//         ADCSRA |= (1 << ADSC);  // 다음 읽기 시작

//         // 1. 모든 LED 일단 끄기 (참고 코드의 PORTA = 0x00 역할)
//         PORTB &= ~((1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5));

//         // 2. 레벨 미터 로직 (보내주신 알고리즘 적용)
//         // LED가 3개이므로 1024 / 3 = 약 341단위로 나눕니다.
        
//         if (value > 100)  PORTB |= (1 << PORTB3); // 1단계: 11번 켜기
//         if (value > 450)  PORTB |= (1 << PORTB4); // 2단계: 12번 켜기
//         if (value > 850)  PORTB |= (1 << PORTB5); // 3단계: 13번 켜기

//         _delay_ms(10); // 부드러운 표시를 위한 짧은 대기
//     }
//     return 0;
// }