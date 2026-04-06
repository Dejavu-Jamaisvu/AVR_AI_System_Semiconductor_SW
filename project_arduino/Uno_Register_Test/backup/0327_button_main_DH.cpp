#include <Arduino.h>
#include <stdint.h>
#include <TaskScheduler.h>

Scheduler runner;

void control_led13();
void control_led12();
void control_led11();

Task task1(5, TASK_FOREVER, &control_led13, &runner, true);
Task task2(5, TASK_FOREVER, &control_led12, &runner, true);
Task task3(5, TASK_FOREVER, &control_led11, &runner, true);

void setup() {
    Serial.begin(9600);
    PORTB |= 0x38;  // set output

    PORTB &= ~0x07; // set input
    //PORTB |= 0x07;  // set pullup //활성화시 내부

    runner.startNow();
}

void loop() {

    //DDRB = ~(PINB << 3); // direct

    /*
    // pulldown
    if ((PINB & 0x07) != 0x00) {
        _delay_ms(20);
        DDRB ^= ((PINB & 0x07) << 3);
        while (((PINB & 0x07) & 0x07));
    }
    */

    /*
    // pullup
    if ((PINB & 0x07) != 0x07) {
        _delay_ms(20);
        DDRB ^= ~(PINB << 3);
        while (!(PINB & 0x07));
    }
    */

    runner.execute();
}

void control_led13() {
    uint8_t reg = (PINB & 0x04);

    if (reg != 0x00) {
        _delay_ms(15);
        if (!((PINB & 0x04) & reg)) {
            DDRB ^= (0x1 << 5);
        }
    }
}

void control_led12() {
    uint8_t reg = (PINB & 0x02);

    if (reg != 0x00) {
        _delay_ms(15);
        if (!((PINB & 0x02) & reg)) {
            DDRB ^= (0x1 << 4);
        }
    }
}

void control_led11() {
    uint8_t reg = (PINB & 0x01);

    if (reg != 0x00) {
        _delay_ms(15);
        if (!((PINB & 0x01) & reg)) {
            DDRB ^= (0x1 << 3);
        }
    }
}