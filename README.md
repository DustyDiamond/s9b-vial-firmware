# S9B Vial Firmware

Open-source [Vial](https://get.vial.today/)/[QMK](https://qmk.fm/) firmware for the
**Koolertron 9-key macro pad "S9B"** (APM32F103, YRKB UF2 bootloader), built for PCs with a
**German keyboard layout**. It replaces the vendor firmware and adds:

- **Macro text arrives 1:1** on a German (QWERTZ) PC: `y z @ - { } [ ] \ | ~ ^` and all other
  printable ASCII characters, exactly as typed in Vial's macro editor.
- **PIN lock on the pad itself**: lock and unlock with a 5-digit PIN, no software on the PC.
  While locked, the pad types nothing and Vial can neither read nor change it.
- **LED feedback**: lock state, PIN progress, wrong PIN, waiting time, one hue per layer.
- **More room**: 8 layers, 64 macros, 16 tap dances, adjustable typing speed.

[Deutsche Version](README.de.md) · [Download](../../releases/latest) · [Build from source](docs/BUILDING.md) · [Changelog](CHANGELOG.md)

> [!NOTE]
> **Built with Claude Code.** Most of this project — the analysis of the vendor firmware, the
> firmware code, tests, build tooling and documentation — was written by
> [Claude Code](https://claude.com/claude-code), Anthropic's AI coding agent, and development
> continues that way. A human set the requirements, made the decisions and tested each release
> on a real device. See [How this project is made](#how-this-project-is-made).

> [!WARNING]
> Flashing custom firmware is at your own risk and can erase your macros or leave the pad
> unusable. Read the [disclaimer](#disclaimer) and **make the backup in step 3 below** first.
>
> **Before every flash, including updates:** save your layout in Vial with
> *File → Save current layout* (a `.vil` file, JSON). `s9b-update.uf2` is meant to keep keymap
> and macros, but a backup is the only way to get them back if something goes wrong. The file
> contains your macros in plain text, so store it like a password.

---

## Is my pad supported?

Tested on **one** device: Koolertron one-handed 9-key keyboard, Amazon.de ASIN `B07VY2HKFZ`.
Your pad is very likely the same hardware if **all** of these are true:

- Vial (0.7.x) shows it as **"S9B"**, manufacturer **"MYKB"**; Windows Device Manager shows
  USB ID **`A286:1DAD`**.
- Pressing a key mapped to **Reset** in Vial makes a USB drive named **`YRKB_UPDATE`** appear,
  containing a file `CURRENT.UF2`.
- The chip on the board is a **Geehy APM32F103CB** (or STM32F103CB) with 9 RGB LEDs.

Newer revisions (the product photos show a "V2") may use different pins. If keys or LEDs do
not work after flashing, restore your backup (see below) and open an issue.

## Install (first time, coming from the vendor firmware)

You need [Vial](https://get.vial.today/) 0.7.5 or newer and a PC (Windows, macOS or Linux).

1. **Write down your macros.** They will be erased. (Vial: *File → Save current layout* keeps a
   `.vil` copy, which can be loaded again after the switch. It contains your macros in plain text.)
2. **Enter the bootloader:** in Vial put the **Reset** key (*Quantum* tab, sometimes called
   `QK_BOOT`) on any key and press it. The drive **`YRKB_UPDATE`** appears.
3. **Back up the vendor firmware:** copy `CURRENT.UF2` from the drive to a safe place. It is the
   complete flash of your pad, **including the vendor macros in plain text** — store it like a
   password (e.g. as an attachment in your password manager). It is your way back.
4. Download **`s9b-first-install.uf2`** from the [latest release](../../releases/latest)
   (optionally check it against `SHA256SUMS`; the build is
   [reproducible](docs/BUILDING.md)) and copy it onto the `YRKB_UPDATE` drive.
5. The pad restarts with the new firmware. The LEDs light up green; Vial now shows
   "S9B" with 8 layers. By default key *n* plays macro `M(n−1)`.

## Update

1. Unlock the pad (if locked).
2. **Back up your layout:** Vial → *File → Save current layout* (`.vil`, JSON with keymap, macros
   and settings). Do this before every update, even though `s9b-update.uf2` should keep
   everything. To restore: *File → Load saved layout*.
3. Press the key mapped to **Reset** / `QK_BOOT` (map it once in Vial, e.g. on a higher layer),
   or use the emergency combination (erases everything, see below).
4. Copy **`s9b-update.uf2`** onto the `YRKB_UPDATE` drive.

Keymap, macros and PIN should be kept unless the [changelog](CHANGELOG.md) says otherwise.

## Back to the vendor firmware

Enter the bootloader and copy your `CURRENT.UF2` backup onto the `YRKB_UPDATE` drive. The
bootloader itself is never overwritten by any of these files.

> Copying the backup back is the standard UF2 way; at the time of writing it has **not** been
> tried on the test device.

---

## Usage

### Key numbers

PIN digits and descriptions use the numpad layout of the keycaps:

```
┌───┬───┬───┐
│ 7 │ 8 │ 9 │
├───┼───┼───┤
│ 4 │ 5 │ 6 │
├───┼───┼───┤
│ 1 │ 2 │ 3 │
└───┴───┴───┘
```

### PIN lock

| What | How |
|------|-----|
| **Lock** | Hold **1** (bottom left) and **9** (top right) for **1 s**, release. LEDs turn dim red. |
| **Unlock** | Type the 5-digit PIN. LEDs blink green. |
| **Set the PIN** (first time) | Same as locking. Without a PIN, setup starts: enter the PIN (blue), repeat it (purple). The pad is locked afterwards. |
| **Change the PIN** | While unlocked, hold 1 + 9 for **5 s** (LEDs turn blue), release, enter the new PIN twice. |
| **Abort setup** | Press nothing for 30 s. The old PIN stays valid. |
| **Emergency: wipe everything** | Hold 1 + 9 **while plugging in**. Keymap, macros and PIN are erased, then the bootloader starts. |

The PIN has 5 digits from 1 to 9 (no 0, digits may repeat). The lock state survives unplugging:
an unlocked pad stays unlocked (at home), a locked pad stays locked until the PIN is entered.

**Wrong PINs:** attempts 1 and 2 are free. From the 3rd wrong attempt a waiting time runs
(30 s, then doubled per further wrong attempt, max. 60 min), during which input is ignored.
Nothing is ever erased. Re-plugging restarts a running waiting time, so just wait.

### LEDs

| Display | Meaning |
|---------|---------|
| Your colour (set in Vial) | Unlocked |
| Slightly shifted hue | A higher layer is active (≈14° per layer) |
| **Dim red** | Locked |
| White flash + one more white LED | Digit accepted (white LEDs = digits entered) |
| **Green**, 2 blinks | PIN correct / new PIN saved |
| **Red**, 3 blinks | Wrong PIN / repetition does not match |
| **Orange, pulsing** | Waiting time after wrong PINs |
| Short red flash | Key pressed during waiting time, or entry discarded after a 5 s pause |
| **Blue** / **purple** | PIN setup: enter / repeat the new PIN |
| Bright red / bright blue while holding 1 + 9 | Release now = lock / set new PIN |

### Macros in Vial

1. Unlock the pad. If Vial shows *"Press and hold the following keys…"*, **don't hold keys —
   type your PIN on the pad**; the dialog closes once the PIN is accepted.
2. *Macros* tab: add a **Text** action and type the text exactly as it should arrive.
3. *Keymap* tab: put the macro (`M0`…`M63`) on a key. Test in a text editor.

**The PC must use the German layout with Caps Lock off.** Login screens, remote desktops or
virtual machines with a US layout will type different characters.

**Text actions** accept all printable ASCII characters:

```
A-Z a-z 0-9 space ! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
```

Characters outside ASCII (`€ µ ä ö ü Ä Ö Ü ß § ° ² ³ ´`) are dropped from text actions. Add them
as a **Tap** action instead: `ä ö ü ß` are on the *Basic* tab, and the *User* tab has keys that
arrive correctly on a German PC:

```
€  @  {  }  [  ]  \  |  ~  µ  ²  ³  §  °  ´  `  ^  Ä  Ö  Ü
```

Tip for generated passwords: all of a password manager's ASCII groups are fine; turn off
"extended ASCII".

### Typing speed

Vial → *QMK Settings* → *Tap-Hold* → **Tap Code Delay** (ms per key event, default 10) → *Save*.
Increase it if login screens or remote sessions drop characters.

### Layers, tap dance, colour

- **8 layers:** put `MO(1)` (hold), `TG(1)` (toggle) or `TO(1)` (switch) on a key of layer 0.
- **64 macros**, about 3.5 KB shared; Vial shows the usage at the bottom of the *Macros* tab.
- **Tap dance** (16): one key, different actions for tap, hold, double tap, tap-and-hold.
- **Combos** are disabled, because 1 + 9 is the lock gesture.
- **Colour:** *Lighting* tab, set on layer 0; higher layers shift the hue automatically.
- Vial cannot name macros; keep a note "macro number → purpose" in your password manager.

### Converter for non-ASCII text (optional)

[`tools/ConvertTo-VialMacroDE.ps1`](tools/ConvertTo-VialMacroDE.ps1) (PowerShell, German UI)
turns any text, including `€ ä ö ü ß`, into a Vial macro key sequence for a German PC. It runs
locally, reads the text hidden and puts the result only into the clipboard. In Vial: macro →
*Open Text Editor…* → paste → *Apply* → *Save*. It also works with the vendor firmware.

---

## Security notes

The PIN lock is a convenience feature, not certified security.

**It protects against:** someone pressing keys on a found or unattended pad; reading or changing
keymap/macros via Vial/VIA on another PC while locked; jumping to the bootloader while locked
(the emergency path erases all data first); guessing (59,049 PINs, waiting time up to 60 min
per attempt).

**It does not protect against:** opening the case and reading the flash through the SWD
header — macros are stored in plain text and no read-out protection is set; malware on a PC
while the pad is unlocked. Vial backups (`.vil`) and `CURRENT.UF2` contain your macros in plain
text. Check whether passwords on a macro pad are allowed by your employer.

Found a security problem? Please open an issue without sensitive details, or contact the
maintainer through GitHub.

## Troubleshooting

| Problem | Cause / fix |
|---------|-------------|
| Keys do nothing, LEDs dim red | Locked → type the PIN. |
| Vial shows errors or an empty keymap | Locked → unlock, reconnect Vial. |
| LEDs pulse orange | Waiting time → wait, do **not** unplug. |
| Wrong characters on the PC | PC layout is not German, Caps Lock is on, or non-ASCII text in a text action. |
| `QK_BOOT` does nothing | Works only while unlocked. |
| PIN forgotten / pad unusable | Hold 1 + 9 while plugging in → everything is erased → copy `s9b-update.uf2` again. |
| Old `.vil` loads with a "different keyboard" warning | Can be skipped. Test every macro afterwards: workarounds typed for the vendor firmware (e.g. `y` for `z`) now come out literally. |

## Technical details

| Item | Value |
|------|-------|
| MCU | Geehy APM32F103CB (STM32F103 compatible), 16 MHz crystal, 72 MHz |
| Matrix | rows B12, A6, A7 · cols B11, B10, B1 · COL2ROW |
| LEDs | 9 × WS2812 on PA8, driven by TIM1 CH1 PWM + DMA |
| Bootloader | YRKB UF2 ([kb_bootloaders](https://github.com/Oh-My-Mechanical-Keyboard/kb_bootloaders) `uf2_stm32f103`), 16 KB, app at `0x08004000`, drive `YRKB_UPDATE` |
| Settings / macros | wear-levelled EEPROM emulation in the last 8 KB of flash (`0x0801E000`) |
| PIN storage | salted SHA-256 (salt + chip UID + PIN) in the keyboard data block |
| Base | [vial-qmk](https://github.com/vial-kb/vial-qmk) at the commit in [`vial-qmk.ref`](vial-qmk.ref) + [`patches/`](patches) |

Repository layout:

| Path | Content |
|------|---------|
| `keyboards/mykb/s9b/` | Keyboard definition, PIN lock and LED logic (`s9b.c`), linker script for the bootloader |
| `keyboards/mykb/s9b/keymaps/vial/` | Vial keymap, layer/macro counts, `vial.json` |
| `patches/` | Small vial-qmk patches: VIA command filter, drop non-ASCII in send_string, fixed Vial build id |
| `scripts/` | `build.sh` (patch, build, UF2), `mkuf2.py` |
| `tests/` | Host simulation of the PIN lock (runs in CI) |
| `tools/` | Optional PowerShell macro converter |

---

## How this project is made

This project is developed with [Claude Code](https://claude.com/claude-code), Anthropic's AI
coding agent, and it will stay that way:

- **Claude Code** analysed the board and the vendor firmware, researched QMK, Vial and the
  bootloader, and wrote the firmware, patches, tests, build scripts, CI, the converter and this
  documentation.
- **DustyDiamond** (the maintainer) set the requirements, made the design decisions, tested every
  release on the real device and decides what gets published.
- Commits carry a `Co-Authored-By: Claude` trailer.

So: the code has been reviewed by one person and tested on one device. Treat it accordingly,
read it before you trust it with secrets, and report problems.

## Contributing

Issues and pull requests are welcome, in **English or German** — see [CONTRIBUTING.md](CONTRIBUTING.md).
Never post passwords, macro contents or `.vil` files.

Development happens here on GitHub: every change, including the maintainer's own, goes through a
pull request that GitHub Actions builds and tests before it is merged. Releases are built from
version tags (see [docs/BUILDING.md](docs/BUILDING.md)).

## Disclaimer

This is an unofficial hobby project. It is **not affiliated with or endorsed by** Koolertron,
Geehy, STMicroelectronics, the QMK or Vial projects. All product names and trademarks belong to
their owners.

The firmware, tools and documentation are provided **"as is", without warranty of any kind**,
express or implied (see sections 11 and 12 of the [license](LICENSE)). Flashing custom firmware
can make the device unusable, void its warranty or erase stored data, including your macros and
passwords. You use this project **at your own risk**. To the extent permitted by applicable law,
the authors and contributors are not liable for any damage, data loss, locked accounts or other
consequences of using it.

## License

[GNU General Public License v2.0 or later](LICENSE) (`GPL-2.0-or-later`), the same license as
QMK and Vial. The released firmware contains QMK, Vial and ChibiOS code under their respective
licenses; the corresponding source is this repository plus vial-qmk at the commit in
[`vial-qmk.ref`](vial-qmk.ref).

**Thanks to** [QMK](https://github.com/qmk/qmk_firmware), [Vial](https://github.com/vial-kb)
and the authors of the [YRKB UF2 bootloader](https://github.com/Oh-My-Mechanical-Keyboard/kb_bootloaders).
