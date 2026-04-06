//my_rfid.h
#ifndef MY_RFID_H
#define MY_RFID_H

#include <stdint.h>
#include "my_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t bytes[10];
    uint8_t size;
} rfid_uid_t;

void rfid_init(spi_cs_t cs);
uint8_t rfid_get_version(void);
int rfid_read_uid(rfid_uid_t *uid);

#ifdef __cplusplus
}
#endif

#endif // MY_RFID_H