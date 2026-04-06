#include <Arduino.h>
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial); 
  Serial.println("\nI2C Scanner (PlatformIO) 시작...");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("스캔 중...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("장치 발견! 주소: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("");
      nDevices++;
    }
  }

  if (nDevices == 0) Serial.println("장치를 찾지 못했습니다.");
  else Serial.println("스캔 완료.");

  delay(5000); 
}