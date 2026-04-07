

// 4-1 수정된코드 
// #define F_CPU 16000000L
// #include <avr/io.h>
// #include <util/delay.h>
// #include <stdio.h>

// void UART0_init(void);
// // 1. 선언부 수정 (int 반환, FILE *stream 추가)
// int UART0_transmit(char data, FILE *stream);
 
// FILE OUTPUT \
// = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

// void UART0_init(void)
// {
//     UBRR0H = 0x00; 
//     UBRR0L = 207;
//     UCSR0A |= _BV(U2X0); 
//     UCSR0C |= 0x06;
//     UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
// }

// // 2. 구현부 수정
// int UART0_transmit(char data, FILE *stream)
// {
//     // printf에서 \n만 써도 터미널에서 제대로 줄바꿈되도록 \r 추가 (선택적 편의 기능)
//     if (data == '\n') UART0_transmit('\r', stream); 
    
//     while( !(UCSR0A & (1 << UDRE0)) ); 
//     UDR0 = data; 
    
//     return 0; // 반드시 0을 반환
// }

// int main(void)
// {
//     UART0_init(); 
//     stdout = &OUTPUT; 
    
//     unsigned int count = 0;
//     while(1)
//     {
//         printf("%d\n", count++); // \r을 위 함수에서 처리하므로 \n만 써도 됨
//         _delay_ms(1000);
//     }
//     return 0;
// }

// 4-1 //platformio.ini에 -Wno-incompatible-pointer-types ; 4-1 경고 숨기기 작성필요
#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

void UART0_init(void);
void UART0_transmit(char data);

FILE OUTPUT \
= FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

void UART0_init(void)
{
	UBRR0H = 0x00; // 9,600 보율로 설정
	UBRR0L = 207;
	
	UCSR0A |= _BV(U2X0); // 2배속 모드
	// 비동기, 8비트 데이터, 패리티 없음, 1비트 정지 비트 모드
	UCSR0C |= 0x06;
	
	UCSR0B |= _BV(RXEN0); // 송수신 가능
	UCSR0B |= _BV(TXEN0);
}

void UART0_transmit(char data)
{
	while( !(UCSR0A & (1 << UDRE0)) ); // 송신 가능 대기
	UDR0 = data; // 데이터 전송
}

int main(void)
{
	UART0_init(); // UART0 초기화
	stdout = &OUTPUT; // printf 사용 설정
	
	/**************** 사용자 작성 코드 시작 ****************/
	unsigned int count = 0;
	while(1)
	{
		printf("%d\n\r", count++);
		_delay_ms(1000);
	}
	/**************** 사용자 작성 코드 끝 *****************/
	
	return 0;
}