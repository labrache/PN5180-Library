# PN5180-Library

Arduino library for the NXP PN5180 NFC/RFID module.  
Supports ISO15693, ISO14443 (MIFARE), FeliCa, and iClass protocols.  
Compatible with **all Arduino architectures** (`architectures=*`), including
AVR, ESP32, and RP2350 (Raspberry Pi Pico 2 with the arduino-pico core).

![PN5180-NFC module](./doc/PN5180-NFC.png)
![PN5180 Schematics](./doc/FritzingLayout.jpg)

---

## Timeout protection (v2.0.0)

All internal wait loops are now guarded against infinite blocking.  If the
PN5180 misses an IRQ flag or a BUSY edge (which can happen when a tag loads the
RF antenna), the affected function returns an error code after at most
`PN5180_TIMEOUT_MS` milliseconds instead of freezing the firmware forever.

The default timeout is **100 ms**.  You can override it globally by defining
the macro **before** including any library header:

```cpp
#define PN5180_TIMEOUT_MS 200   // use 200 ms instead
#include "PN5180ISO14443.h"
```

### Recommended continuous-reading pattern

Cycle the RF field on every iteration to reset any stuck tag/IRQ state:

```cpp
void loop() {
  if (!nfc.setupRF()) { delay(200); return; }   // RF on + configure

  uint8_t uid[7];
  uint8_t len = nfc.readCardSerial(uid);         // read tag (or timeout)
  if (len >= 4) { /* process uid */ }

  if (!nfc.setRF_off()) { nfc.reset(); }         // RF off; reset on error

  delay(300);
}
```

See `examples/ISO14443Continuous/` for a fully commented sketch.

---

Release Notes:

Version 2.0.0 - 22.05.2026

  * Timeout protection for all blocking IRQ/BUSY wait loops (PN5180_TIMEOUT_MS).
  * New example: examples/ISO14443Continuous/ (robust continuous ISO14443 reading).
  * Return-value propagation for transceiveCommand() and all callers.
  * See CHANGELOG.md for full details.

Version 1.8.1 - 19.08.2021

	* Added changes from Nettermann90

Version 1.7 - 12.07.2021

	* Migrated branch from Dirk Carstensen for ISO14443 tags to the library.
	* See https://github.com/tueddy/PN5180-Library/tree/ISO14443

Version 1.6 - 13.03.2021

	* Added PN5180::writeEEPROM

Version 1.5 - 29.01.2020

	* Fixed offset in readSingleBlock. Was off by 1.

Version 1.4 - 13.11.2019

	* ICODE SLIX2 specific commands, see https://www.nxp.com/docs/en/data-sheet/SL2S2602.pdf
	* Example usage, currently outcommented

Version 1.3 - 21.05.2019

	* Initialized Reset pin with HIGH
	* Made readBuffer static
	* Typo fixes
	* Data type corrections for length parameters

Version 1.2 - 28.01.2019

	* Cleared Option bit in PN5180ISO15693::readSingleBlock and ::writeSingleBlock

Version 1.1 - 26.10.2018

	* Cleanup, bug fixing, refactoring
	* Automatic check for Arduino vs. ESP-32 platform via compiler switches
	* Added open pull requests
	* Working on documentation

Version 1.0.x - 21.09.2018

	* Initial versions
