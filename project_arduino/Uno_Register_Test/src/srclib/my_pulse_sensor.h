//src/srclib/my_pulse_sensor.h

#ifndef MY_PULSE_SENSOR_H
#define MY_PULSE_SENSOR_H

#include <avr/io.h>
#include <stdbool.h>

// ADC 초기화 함수 (ADC0 채널 사용)
void ADC_Init();

// ADC 값 읽기 함수
uint16_t ADC_Read(uint8_t channel);

// 심박 감지 로직을 위한 구조체
typedef struct {
    uint8_t pin;
    uint16_t threshold;
    bool lastState;
} PulseSensor;

// 센서 초기화 및 비트 감지 함수 선언
void PulseSensor_Init(PulseSensor* sensor, uint8_t channel, uint16_t threshold);
bool PulseSensor_IsBeatDetected(PulseSensor* sensor);

#endif