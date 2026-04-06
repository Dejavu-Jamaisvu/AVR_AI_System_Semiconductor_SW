//my_spi.h
#ifndef MY_SPI_H
#define MY_SPI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    uint8_t pin;
} spi_cs_t;

void spi_init(void);
void spi_cs_init(spi_cs_t cs);
uint8_t spi_transfer(uint8_t data);
void spi_transaction(spi_cs_t cs, const uint8_t *tx, uint8_t *rx, size_t len);

#ifdef __cplusplus
}
#endif

#endif // MY_SPI_H