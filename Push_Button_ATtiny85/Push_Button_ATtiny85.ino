#include <avr/sleep.h>
#include <avr/interrupt.h>

const int checkPin   = 3; // PB3
const int triggerPin = 2; // PB2
const int buttonPin  = 0; // PB0
const int ledPin     = 1; // PB1

bool ledState = false;
volatile bool wakeUpFlag = false;

void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(triggerPin, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(checkPin, INPUT);

  digitalWrite(triggerPin, LOW);
  digitalWrite(ledPin, LOW);

  disableUnusedPins();

  // Enable Pin Change Interrupt
  GIMSK |= (1 << PCIE);

  // Interrupt untuk PB0
  PCMSK |= (1 << PCINT0);

  sei();
}

void loop() {

  goToSleep();

  if (wakeUpFlag) {

    wakeUpFlag = false;

    delay(30);

    // tombol ditekan
    if (digitalRead(buttonPin) == LOW) {

      unsigned long pressStart = millis();
      bool actionTaken = false;

      // tunggu long press 1.5 detik
      while (digitalRead(buttonPin) == LOW) {

        if (!actionTaken && millis() - pressStart >= 1100) {

          actionTaken = true;

          ledState = true;

          digitalWrite(ledPin, HIGH);

          // tunggu tombol dilepas
          while (digitalRead(buttonPin) == LOW);

          delay(30);

          break;
        }
      }

      // jika LED aktif → jalankan mode aktif
      if (ledState) {

        initialLedOnPeriod(25000);
      }
    }
  }
}

// ======================================================
// LED wajib ON minimal 25 detik
// ======================================================

void initialLedOnPeriod(unsigned long durationMs) {

  unsigned long startTime = millis();

  while (millis() - startTime < durationMs) {

    // Tombol ditekan lama lagi
    if (digitalRead(buttonPin) == LOW) {

      unsigned long secondPress = millis();

      while (digitalRead(buttonPin) == LOW) {

        if (millis() - secondPress >= 1500) {

          holdPin2AndCheckPin1();

          return;
        }
      }
    }
  }

  // lanjut monitoring normal
  monitorLedActive();
}

// ======================================================
// Monitoring saat sistem aktif
// ======================================================

void monitorLedActive() {

  while (ledState) {

    // checkPin LOW selama 5 detik
    if (digitalRead(checkPin) == LOW) {

      unsigned long lowStart = millis();

      while (digitalRead(checkPin) == LOW) {

        if (millis() - lowStart >= 5000) {

          ledState = false;

          digitalWrite(ledPin, LOW);

          return;
        }
      }
    }

    // Tombol ditekan lama
    if (digitalRead(buttonPin) == LOW) {

      unsigned long secondPress = millis();

      while (digitalRead(buttonPin) == LOW) {

        if (millis() - secondPress >= 1500) {

          holdPin2AndCheckPin1();

          return;
        }
      }
    }
  }
}

// ======================================================
// Trigger pin aktif maksimal 15 detik
// atau sampai checkPin LOW 5 detik
// ======================================================

void holdPin2AndCheckPin1() {

  digitalWrite(triggerPin, HIGH);

  unsigned long startTime = millis();

  const unsigned long maxTime = 15000;

  while (millis() - startTime < maxTime) {

    if (digitalRead(checkPin) == LOW) {

      unsigned long lowStart = millis();

      while (digitalRead(checkPin) == LOW) {

        if (millis() - lowStart >= 5000) {

          digitalWrite(triggerPin, LOW);

          digitalWrite(ledPin, LOW);

          ledState = false;

          return;
        }
      }
    }
  }

  // timeout 15 detik
  digitalWrite(triggerPin, LOW);

  digitalWrite(ledPin, LOW);

  ledState = false;
}

// ======================================================
// Sleep Mode
// ======================================================

void goToSleep() {

  cli();

  // Disable ADC
  ADCSRA &= ~(1 << ADEN);

  // Disable Watchdog
  MCUSR &= ~(1 << WDRF);

  WDTCR |= (1 << WDCE) | (1 << WDE);

  WDTCR = 0x00;

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  sleep_enable();

  sleep_bod_disable();

  sei();

  sleep_cpu();

  sleep_disable();
}

// ======================================================
// Interrupt Wakeup
// ======================================================

ISR(PCINT0_vect) {

  wakeUpFlag = true;
}

// ======================================================
// Disable unused pin
// ======================================================

void disableUnusedPins() {

  pinMode(4, INPUT_PULLUP);
}

// ======================================================
// Compatibility
// ======================================================

#ifndef sleep_bod_disable
#define sleep_bod_disable() \
  do { \
    MCUCR |= (1 << BODS) | (1 << BODSE); \
    MCUCR &= ~(1 << BODSE); \
  } while (0)
#endif