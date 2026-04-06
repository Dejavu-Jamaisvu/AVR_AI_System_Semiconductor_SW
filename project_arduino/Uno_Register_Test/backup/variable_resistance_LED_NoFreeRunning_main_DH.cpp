#include <Arduino.h>

void adc_init(unsigned char);
int read_adc(void);

void setup() {
    Serial.begin(9600);
    adc_init(0);

    PORTB |= 0x38;  // set LED output (11, 12, 13)
}

void loop() {
    int adc_val = read_adc();
    Serial.println(adc_val);

    if (adc_val >= 768) {
        DDRB = 0x38;
    }
    else if (adc_val >= 512) {
        DDRB = 0x30;
    }
    else if (adc_val >= 256) {
        DDRB = 0x20;
    }
    else {
        DDRB = 0x00;
    }

    delay(100);
}

void adc_init(unsigned char channel) {
    ADMUX = (1 << REFS0); // AVCC 기준 전압
    ADCSRA = 0x07;        // 128 분주
    ADCSRA |= (1 << ADEN); // ADC 활성화
    ADMUX |= ((ADMUX & 0xE0) | channel); // 채널 선택 (PF0 = 0)
    ADCSRA |= (1 << ADSC); // 변환 시작
    //ADCSRA |= (1 << ADATE); // FREE RUNNING
}

int read_adc(void) {
    ADCSRA |= (1 << ADSC);                          // 변환 시작
    while(!(ADCSRA & (1<< ADIF))); // 변환 완료 대기
    return ADC; // 결과 반환 (0~1023)
}