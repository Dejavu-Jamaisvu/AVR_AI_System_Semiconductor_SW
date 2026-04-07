#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t adc_result = 0; // 변환 결과를 저장할 전역 변수

void adc_init(void) {
    // ★ 추가: LED 사용 준비 (13번 핀)
    DDRB |= (1 << DDB5);


    // 1. 기준 전압 설정 (AVCC with external capacitor at AREF pin)
    ADMUX |= (1 << REFS0);

    // 2. 입력 채널 선택 (A0 핀 선택: MUX3:0 = 0000)
    ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));

    // 3. ADCSRA 설정
    // ADEN: ADC 켬
    // ADIE: 변환 완료 시 인터럽트 발생 허용
    // ADPS2:0: 128 분주 (16MHz / 128 = 125kHz, 안정적인 변환 속도)
    ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    sei(); // 전역 인터럽트 허용
}

// [핵심] ADC 변환 완료 인터럽트 서비스 루틴
ISR(ADC_vect) {
    // ADC 레지스터는 ADCL과 ADCH로 나뉘어 있으나,
    // C 컴파일러가 'ADC'라는 이름으로 한 번에 16비트를 읽도록 지원함
    adc_result = ADC;

    // (옵션) 다음 변환을 자동으로 시작하고 싶다면 여기서 다시 ADSC를 켬
    // 또는 Free Running 모드를 설정할 수도 있음
}

int main(void) {
    adc_init();


    while (1) {
        // 변환 시작 신호를 보냄 (ADSC 비트 세트)
        ADCSRA |= (1 << ADSC);

        // 변환 도중 메인 루프는 다른 작업을 수행할 수 있음 (Non-blocking)
        // 여기서는 예시로 adc_result 값을 이용해 로직 처리
        if (adc_result > 512) {
            // 전압이 약 2.5V 이상일 때 처리
            
			//★ 추가 LED 가변저항으로 끄고 켜기
            PORTB |= (1 << PORTB5);  // LED 켜기 (2.5V 이상)
        } else {
            PORTB &= ~(1 << PORTB5); // LED 끄기 (2.5V 미만)
        }

        // 너무 자주 읽지 않도록 적절한 딜레이나 타이머 연동 권장
    }
    return 0;
}