#include <Arduino.h>

void setup() {
    Serial.begin(9600);
    pinMode(13,OUTPUT);
}

unsigned int count = 0;

void loop() {
    digitalWrite(13,HIGH);
    delay(1000);
    digitalWrite(13,LOW);
    delay(1000);
}

// #include <Arduino.h>

// void setup() {
//     Serial.begin(9600);
// }

// unsigned int count = 0;

// void loop() {
//     Serial.println(count++);
//     delay(1000);
// }