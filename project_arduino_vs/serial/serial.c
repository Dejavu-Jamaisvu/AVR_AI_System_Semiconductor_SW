#include <stdio.h>
#include <stdlib.h>
#include <libserialport.h>

int main() {
    struct sp_port* port;
    enum sp_return result;

    // 1. 포트 지정 (리눅스의 ttyACM0 -> 윈도우의 COM3)
    result = sp_get_port_by_name("COM3", &port);
    if (result != SP_OK) {
        printf("포트를 찾을 수 없습니다. (COM3 확인 필요)\n");
        return -1;
    }

    // 2. 포트 열기
    result = sp_open(port, SP_MODE_READ);
    if (result != SP_OK) {
        printf("포트를 열 수 없습니다. 다른 프로그램이 사용 중인지 확인하세요.\n");
        return -1;
    }

    // 3. 통신 설정 (보통 아두이노 등은 9600 또는 115200 사용)
    sp_set_baudrate(port, 9600);
    sp_set_bits(port, 8);
    sp_set_parity(port, SP_PARITY_NONE);
    sp_set_stopbits(port, 1);

    printf("--- COM3 데이터 모니터링 시작 (Ctrl+C로 종료) ---\n");

    // 4. 무한 루프 데이터 읽기
    char buffer[512];
    while (1) {
        // 데이터가 올 때까지 최대 100ms 대기하며 읽기
        int bytes_read = sp_blocking_read(port, buffer, sizeof(buffer) - 1, 100);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // 문자열 출력 안전 장치
            printf("%s", buffer);
            fflush(stdout); // 즉시 화면에 출력
        }
    }

    // 5. 종료 처리 (실제 실행 시엔 루프 때문에 도달하지 않음)
    sp_close(port);
    sp_free_port(port);
    return 0;
}