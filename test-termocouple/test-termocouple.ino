#include "max6675.h"

int thermoDO = 12;
int thermoCLK = 13;
int thermoCS1 = 10;
int thermoCS2 = 9;

#define BUZZER 5
#define VOLTAGE_PIN A0

float vin;
float vout;

float R1 = 10000.0;
float R2 = 10000.0;
float ratio = R2 / (R1 + R2);
float VREF = 5.0;
float K = 1.024;

MAX6675 thermocouple1(thermoCLK, thermoCS1, thermoDO);
MAX6675 thermocouple2(thermoCLK, thermoCS2, thermoDO);

unsigned long lastPrintTime = 0;
const unsigned long printInterval = 1000;

// =======================
void setup() {

  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);

  delay(500);
}

// =======================
void loop() {

  checkSerialCommand();

  if (millis() - lastPrintTime >= printInterval) {

    lastPrintTime = millis();

    float offset1 = -2.3;
    float offset2 = -2.3;

    float temp1 = thermocouple1.readCelsius() + offset1;
    float temp2 = thermocouple2.readCelsius() + offset2;

    long sum = 0;
    float offset_v = 0.67;
    for (int i = 0; i < 10; i++) {
      sum += analogRead(VOLTAGE_PIN);
    }

    vin = sum / 10.0;
    vout = vin * (VREF / 1023.0) / ratio * K - offset_v;
    float percent = (vout - 6.5) / (8.15 - 6.5) * 100.0;

    if (percent > 100) percent = 100;
    if (percent < 0) percent = 0;

    Serial.print(temp1);
    Serial.print(",");
    Serial.print(temp2);
    Serial.print(",");
    Serial.println(percent);
  }
}

// =======================
// SERIAL COMMAND
// =======================
void checkSerialCommand() {

  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "BEEP") {

      tone(BUZZER, 2000);
      delay(300);
      noTone(BUZZER);
    }

    else if (cmd == "ALARM") {

      for (int i = 0; i < 6; i++) {

        tone(BUZZER, 2000);
        delay(1000);
        noTone(BUZZER);
        delay(300);

      }
    }
  }
}