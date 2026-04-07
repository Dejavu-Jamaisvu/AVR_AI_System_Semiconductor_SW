#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

void UART0_init(void);
int UART0_transmit(char data, FILE *stream);
int UART0_receive(FILE *stream); 

//FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

// printf와 scanf를 위한 스트림 설정
FILE uart_str = FDEV_SETUP_STREAM(UART0_transmit, UART0_receive, _FDEV_SETUP_RW);

void UART0_init(void)
{
	UBRR0H = 0x00; // 9,600 보율로 설정 ////(16MHz, U2X0=1)
	UBRR0L = 207;
	
	UCSR0A |= _BV(U2X0); // 2배속 모드
	// 비동기, 8비트 데이터, 패리티 없음, 1비트 정지 비트 모드
	UCSR0C |= 0x06;
	
	UCSR0B |= _BV(RXEN0); // 송수신 가능
	UCSR0B |= _BV(TXEN0);
}


//// 수정된 전송 함수
int UART0_transmit(char data, FILE *stream) //FILE *stream 추가함
{
    // 줄바꿈 문자(\n)가 들어오면 캐리지 리턴(\r)을 먼저 보냄 (가독성용)
    if (data == '\n') UART0_transmit('\r', stream); 
    
    while( !(UCSR0A & (1 << UDRE0)) ); // 송신 가능 대기
    UDR0 = data; // 데이터 전송

    return 0;    // 성공 반환
}

/// 수신 함수 추가!!
int UART0_receive(FILE *stream) {
    while (!(UCSR0A & (1 << RXC0))); // 데이터 올 때까지 무한 대기
    return UDR0;
}

int main(void)
{
	UART0_init(); // UART0 초기화
	//stdout = &OUTPUT; // printf 사용 설정
	stdout = stdin = &uart_str; // 표준 입출력을 UART로 연결
	
	// LED 핀 설정 (PB3, PB4, PB5를 출력으로 - 아두이노 11, 12, 13번 해당)
    DDRB |= _BV(DDB3) | _BV(DDB4) | _BV(DDB5);
	char buffer[50]; // 문장을 저장할 바구니 (최대 50글자)


	/**************** 사용자 작성 코드 시작 ****************/
	//unsigned int count = 0;
	unsigned int mode = 1;
	while(1)
	{
		// printf("%d\n\r", count++);
		// _delay_ms(1000);


		// 데이터가 들어왔는지 확인 (비폐쇄형 읽기를 원할 때)
        if (UCSR0A & (1 << RXC0)) {
            //char c = getchar(); // 한 글자 읽기
			scanf("%s", buffer);
			printf("Received: %s\n", buffer);
            
            // 명령에 따른 LED 제어 (핀 반전)
            // if (c == '1') PORTB ^= _BV(PORTB3); // 11번 토글
            // if (c == '2') PORTB ^= _BV(PORTB4); // 12번 토글
            // if (c == '3') PORTB ^= _BV(PORTB5); // 13번 토글
            
           }

	}
	/**************** 사용자 작성 코드 끝 *****************/
	
	return 0;
}