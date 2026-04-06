#include <Arduino.h>

// 1. ADC 초기화 함수 (이미지의 128 분주, 125KHz 설정 반영)
void ADC_init(unsigned char channel) {
    ADMUX = (1 << REFS0);                // AVCC(5V) 기준 전압 설정
    ADCSRA = 0x07;                       // 분주비 128 (16MHz/128 = 125KHz)
    ADCSRA |= (1 << ADEN);               // ADC 활성화
    
    // 채널 선택 (A0 핀을 쓰려면 channel에 0을 넣습니다)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); 
    
    ADCSRA |= (1 << ADSC);               // 첫 변환 시작
}

// 2. ADC 값 읽기 함수
int read_ADC(void) {
    // 변환이 완료될 때까지 대기 (ADSC 비트가 0이 되면 완료)
    while(ADCSRA & (1 << ADSC)); 
    
    int result = ADC;                    // 0~1023 사이의 값 읽기
    ADCSRA |= (1 << ADSC);               // 다음 변환 시작 명령
    return result;
}

void setup() {
    // 이미지 우측 상단의 Serial 9600 설정
    Serial.begin(9600);
    
    // ADC 0번 채널(PC0/A0 핀) 초기화
    ADC_init(0);
    
    Serial.println("--- 가변저항 읽기 테스트 시작 ---");
}

void loop() {
    // 가변저항 값 읽어오기
    int val = read_ADC();

    // 시리얼 모니터에 출력
    Serial.print("Potentiometer Value: ");
    Serial.println(val);

    // 너무 빨리 올라가면 보기 힘드니까 0.1초씩 쉬어줍니다.
    delay(500); 
}