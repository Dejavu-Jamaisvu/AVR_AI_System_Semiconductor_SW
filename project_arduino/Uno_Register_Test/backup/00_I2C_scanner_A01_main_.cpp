#include <Arduino.h>
#include <avr/io.h>

void setup() {
    Serial.begin(9600);

    DDRB |= 0x38;  // set LED output (11, 12, 13)
    DDRC |= 0x00;
    ACSR = 0x00;  // ACD=0 (켜기), ACBG=0 (AIN0 핀 사용), 나머지 기본값
    ADCSRA &= ~(1 << ADEN);
    ADCSRB |= (1 << ACME);
    ADMUX &= ~0x07; // AIN1 만 변경 가능
}

void loop() {
                        // bit(5)
    PORTB = (ACSR & (1 << ACO)) ? 0x38 : 0x00;
    Serial.println((ACSR & (1 << ACO)));
    delay(500);
}