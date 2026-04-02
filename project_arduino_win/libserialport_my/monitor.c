#include <stdio.h>
#include <libserialport.h>

int main() {
    struct sp_port *port;
    if (sp_get_port_by_name("COM3", &port) != SP_OK) {
        printf("COM3를 찾을 수 없습니다.\n");
        return 1;
    }
    sp_open(port, SP_MODE_READ);
    sp_set_baudrate(port, 9600);

    printf("--- COM3 모니터링 시작 ---\n");
    char buf[512];
    while (1) {
        int n = sp_blocking_read(port, buf, sizeof(buf)-1, 100);
        if (n > 0) {
            buf[n] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }
    }
    return 0;
}