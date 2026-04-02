#include <stdio.h>
#include <stdlib.h>
#include <libserialport.h>
#include <windows.h>      // 추가: 윈도우 API 사용

// 에러 처리용 매크로
#define CHECK(ret) if (ret != SP_OK) { fprintf(stderr, "Error %d\n", ret); exit(1); }

int main() {
    // 윈도우 콘솔 출력을 UTF-8로 설정하여 한글 깨짐 방지
    SetConsoleOutputCP(CP_UTF8);
    const char *port_name = "COM3"; // 대상 포트 COM5로 변경됨
    int baud_rate = 9600;
    struct sp_port *port;

    printf("포트 검색 중: %s\n", port_name);
    CHECK(sp_get_port_by_name(port_name, &port));

    printf("포트 개방 중...\n");
    CHECK(sp_open(port, SP_MODE_READ));

    printf("보드레이트 설정: %d\n", baud_rate);
    CHECK(sp_set_baudrate(port, baud_rate));

    printf("모니터링 시작 (대상: %s, 종료: Ctrl+C)\n", port_name);
    printf("==================================\n");

    char buf[256];
    while (1) {
        // 최대 1000ms 대기하며 데이터 수신 (블로킹 읽기)
        int bytes_read = sp_blocking_read(port, buf, sizeof(buf) - 1, 1000);
        
        if (bytes_read > 0) {
            buf[bytes_read] = '\0'; // 문자열 끝 처리
            printf("%s", buf);
            fflush(stdout);         // 버퍼 즉시 비우기
        } else if (bytes_read < 0) {
            fprintf(stderr, "\n수신 에러 발생 (포트 연결 끊김 등)\n");
            break;
        }
        // bytes_read == 0 이면 타임아웃, 계속 루프 진행
    }

    printf("\n포트 닫기\n");
    sp_close(port);
    sp_free_port(port);

    return 0;
}
