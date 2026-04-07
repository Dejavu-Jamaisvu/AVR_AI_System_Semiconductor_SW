#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

void init() {
    DDRB |= (1 << PB5);

    DDRD &= ~((1 << PD6) | (1 << PD7));

    ACSR = 0x00;
}

int main() {
    init();

    while (1) {
        if (ACSR & (1 << ACO)) {
            PORTB |= (1 << PB5);
        } else {
            PORTB &= ~(1 << PB5);
        }
    }
    return 0;
}