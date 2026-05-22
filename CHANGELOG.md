# Changelog

All notable changes to this project will be documented in this file.

---

## [2.0.0] — 2026-05-22

### Problem solved
On platforms with a shared SPI bus (e.g. Raspberry Pi Pico 2 / RP2350 running
the arduino-pico core) the firmware could **freeze permanently** when an
ISO14443 tag loaded the RF antenna.  The root cause was that several internal
wait loops spun forever if the PN5180 missed an IRQ flag or a BUSY edge.

### Added
- **`PN5180_TIMEOUT_MS`** macro in `PN5180.h` (default: **100 ms**).  
  Every wait loop in the library now uses this deadline.  Override it before
  including any library header:
  ```cpp
  #define PN5180_TIMEOUT_MS 200   // extend to 200 ms
  #include "PN5180ISO14443.h"
  ```
- **`examples/ISO14443Continuous/`** — new sketch that reads ISO14443 tags
  continuously by cycling the RF field on each iteration
  (`setupRF()` → `readCardSerial()` → `setRF_off()` → `delay()`).
  Shows the recommended pattern for freeze-free continuous reading.

### Fixed
- Infinite blocking wait loops in:
  - `PN5180::transceiveCommand()` — five `while(BUSY…)` waits
  - `PN5180::setRF_on()` — `while(TX_RFON_IRQ_STAT…)`
  - `PN5180::setRF_off()` — `while(TX_RFOFF_IRQ_STAT…)`
  - `PN5180::reset()` — `while(IDLE_IRQ_STAT…)`
  - `PN5180ISO15693::issueISO15693Command()` — `while(RX_IRQ_STAT…)`

### Changed
- `PN5180::transceiveCommand()` — all five `while(BUSY…)` spins are guarded
  by `PN5180_TIMEOUT_MS`.  When a timeout fires in the middle of an SPI frame
  (NSS is LOW), NSS is deasserted before returning `false`.
- `PN5180::setRF_on()` — returns `false` on IRQ wait timeout (was: infinite
  loop).
- `PN5180::setRF_off()` — returns `false` on IRQ wait timeout.
- `PN5180::reset()` — exits the IRQ wait loop via `break` on timeout (function
  is `void`; continues to clear all IRQ flags).
- `PN5180ISO15693::issueISO15693Command()` — returns `EC_NO_CARD` on RX-IRQ
  wait timeout.
- `PN5180::writeRegister()`, `writeRegisterWithOrMask()`,
  `writeRegisterWithAndMask()`, `readRegister()`, `writeEEPROM()`,
  `readEEprom()`, `sendData()`, `loadRFConfig()` — now propagate the `bool`
  return value of `transceiveCommand()` instead of always returning `true`.
- `PN5180::readData()` — returns `NULL` when `transceiveCommand()` times out,
  instead of returning a pointer to a partially-filled buffer.  All existing
  callers (`issueISO15693Command`, `issueiClassCommand`, `pol_req`,
  `activateTypeA`) already check for `NULL`.

### Compatibility
- **No public API changes**: all method signatures are identical to v1.8.1.
- **Nominal (non-timeout) behaviour is unchanged**: no latency is added when
  the hardware responds within `PN5180_TIMEOUT_MS`.
- Compiles on all `architectures=*` targets; no platform-specific code added.
- Rollover-safe timeout arithmetic: `millis() - start > PN5180_TIMEOUT_MS`
  (not `millis() > start + …`).

---

## [1.8.1] — 2021-08-19

- Added changes from Nettermann90.

## [1.7] — 2021-07-12

- Migrated ISO14443 branch from Dirk Carstensen.

## [1.6] — 2021-03-13

- Added `PN5180::writeEEPROM`.

## [1.5] — 2020-01-29

- Fixed offset in `readSingleBlock` (was off by 1).

## [1.4] — 2019-11-13

- ICODE SLIX2 specific commands.

## [1.3] — 2019-05-21

- Initialised Reset pin with HIGH; made `readBuffer` static; typo fixes;
  data type corrections for length parameters.

## [1.2] — 2019-01-28

- Cleared Option bit in `readSingleBlock` and `writeSingleBlock`.

## [1.1] — 2018-10-26

- Cleanup, bug fixing, refactoring; automatic Arduino/ESP-32 detection.

## [1.0.x] — 2018-09-21

- Initial versions.
