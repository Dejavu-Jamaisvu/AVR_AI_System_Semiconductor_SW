//src/srclib/my_pulse_sensor.c

#include "my_pulse_sensor.h"

void ADC_Init() {
    // REFS0: AVCC(5V)를 기준 전압으로 사용
    ADMUX = (1 << REFS0);
    // ADEN: ADC 활성화, ADPS2:0: 분주비를 128로 설정 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_Read(uint8_t channel) {
    // 채널 선택 (0~7번)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // 변환 시작
    ADCSRA |= (1 << ADSC);
    // 변환 완료될 때까지 대기
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void PulseSensor_Init(PulseSensor* sensor, uint8_t channel, uint16_t threshold) {
    sensor->pin = channel;
    sensor->threshold = threshold;
    sensor->lastState = false;
    ADC_Init();
}

bool PulseSensor_IsBeatDetected(PulseSensor* sensor) {
    uint16_t rawValue = ADC_Read(sensor->pin);

    if (rawValue > sensor->threshold) {
        if (!sensor->lastState) {
            sensor->lastState = true;
            return true; // 비트 발생!
        }
    } else {
        sensor->lastState = false;
    }
    return false;
}