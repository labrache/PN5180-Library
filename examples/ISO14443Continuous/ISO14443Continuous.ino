// NAME: ISO14443Continuous.ino
//
// DESC: Continuous ISO14443 card reading example for the PN5180 library.
//       Demonstrates robust, freeze-free reading by cycling the RF field
//       on every iteration and checking all return values.
//
// Reading pattern:
//   reset() -> setupRF() -> readCardSerial() -> setRF_off() -> delay() -> repeat
//
// Cycling the RF field (setRF_off + setupRF each loop) prevents the firmware
// from freezing when a tag loads the antenna and the PN5180 misses an IRQ or
// a BUSY edge.  All blocking wait loops in the library are now guarded by
// PN5180_TIMEOUT_MS (default 100 ms); adjust before including the headers if
// you need a different value:
//
//   #define PN5180_TIMEOUT_MS 200   // increase for slower hardware
//   #include "PN5180ISO14443.h"
//
// Tested with: Raspberry Pi Pico 2 (RP2350), arduino-pico core, MIFARE Ultralight.
// Should compile unchanged on AVR, ESP32, and any other Arduino-compatible platform.
//
// WIRING (adjust pin numbers to your board):
//   PN5180 NSS  -> D10
//   PN5180 BUSY -> D9
//   PN5180 RST  -> D8
//   PN5180 MOSI -> MOSI (board default)
//   PN5180 MISO -> MISO (board default)
//   PN5180 SCK  -> SCK  (board default)
//   PN5180 3.3V -> 3.3 V
//   PN5180 GND  -> GND

#include <Arduino.h>
#include "PN5180ISO14443.h"

// ---------- pin configuration ----------
#define PN5180_NSS   10
#define PN5180_BUSY   9
#define PN5180_RST    8
// ---------------------------------------

PN5180ISO14443 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

// -----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial);  // wait for USB-CDC on native-USB boards (Pico, Leonardo…)

  Serial.println(F("PN5180 ISO14443 Continuous Reader"));
  Serial.println(F("=================================="));

  nfc.begin();
  nfc.reset();

  // Show firmware version stored in EEPROM
  uint8_t firmware[2];
  if (nfc.readEEprom(FIRMWARE_VERSION, firmware, sizeof(firmware))) {
    Serial.print(F("PN5180 firmware v"));
    Serial.print(firmware[1]);
    Serial.print('.');
    Serial.println(firmware[0]);
  } else {
    Serial.println(F("Warning: could not read firmware version (SPI timeout?)"));
  }

  Serial.println(F("Scanning for ISO14443 tags…"));
}

// -----------------------------------------------------------------------
void loop() {
  // --- Step 1: configure RF and turn field ON ---
  if (!nfc.setupRF()) {
    // setupRF() calls loadRFConfig() then setRF_on(); either timed out.
    Serial.println(F("setupRF() failed – retrying after 200 ms"));
    delay(200);
    return;
  }

  // --- Step 2: try to activate a Type-A tag and read its serial number ---
  uint8_t uid[7] = {0};
  uint8_t uidLen = nfc.readCardSerial(uid);

  if (uidLen >= 4) {
    Serial.print(F("Tag found  UID("));
    Serial.print(uidLen);
    Serial.print(F("): "));
    for (uint8_t i = 0; i < uidLen; i++) {
      if (i > 0) Serial.print(':');
      if (uid[i] < 0x10) Serial.print('0');
      Serial.print(uid[i], HEX);
    }
    Serial.println();
  } else {
    Serial.println(F("No tag in field."));
  }

  // --- Step 3: turn RF field OFF before the next cycle ---
  // This is the key step: it resets the tag and clears any stuck IRQ state
  // in the PN5180, preventing the freeze described in the library changelog.
  if (!nfc.setRF_off()) {
    Serial.println(F("setRF_off() timed out – continuing anyway"));
    // A failed RF-off is recoverable: reset() in the next iteration will
    // re-initialise the chip.
    nfc.reset();
  }

  // --- Step 4: brief pause before next read cycle ---
  delay(300);
}
