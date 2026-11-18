# Changelog

All notable changes. Versions follow [semantic versioning](https://semver.org/);
a change of [`eeprom-layout.id`](eeprom-layout.id) (keymap and macros are reset once) is always
called out here.

## [1.0.0] - 2026-11-18

First public release. Tested on one device (Koolertron S9B, APM32F103CB) since 2026-11-17.

- Vial firmware based on vial-qmk `dd43959a`, replacing the vendor firmware.
- Macro text is typed 1:1 on a German (QWERTZ) host for all printable ASCII characters;
  non-ASCII bytes are skipped. German special characters (`€ µ ² ³ § ° ´ Ä Ö Ü` and AltGr
  characters) as *User* keycodes.
- PIN lock without host software: 5-digit PIN, lock with 1 + 9, PIN setup/change on the pad,
  growing waiting time after wrong PINs, VIA/Vial access refused while locked, Vial's unlock
  dialog accepts the PIN.
- LED feedback (PWM + DMA WS2812 driver): lock state, PIN progress, OK / wrong PIN, waiting
  time, per-layer hue shift.
- 8 layers, 64 macros, 16 tap dances, adjustable tap code delay for macros.
- Emergency combination (1 + 9 while plugging in) erases all data and starts the bootloader.
- Fixed Vial build id: keymap and macros survive updates.
- `s9b-first-install.uf2` (erases the settings area) and `s9b-update.uf2`.
- Optional PowerShell converter for non-ASCII macro text.

EEPROM layout id: `0x5A9B02`.

[1.0.0]: https://github.com/DustyDiamond/s9b-vial-firmware/releases/tag/v1.0.0
