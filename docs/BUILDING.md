# Building from source

[Deutsch](BUILDING.de.md)

The firmware is vial-qmk at a pinned commit, plus this repository's keyboard folder and three
small patches. `scripts/build.sh` applies the patches, copies the keyboard, builds and writes
the UF2 files. Every release on GitHub is built exactly this way by GitHub Actions
([`.github/workflows/build.yml`](../.github/workflows/build.yml)).

The build is reproducible: with option A your `SHA256SUMS` match the release's byte for byte,
so you can check that a downloaded UF2 comes from exactly this source.

## Option A: container (recommended, any OS with Podman or Docker)

Uses the same pinned QMK build image as CI, so you need nothing else installed.

```bash
git clone https://github.com/DustyDiamond/s9b-vial-firmware.git
cd s9b-vial-firmware
podman run --rm -it -v "$PWD":/src:Z -w /src \
  ghcr.io/qmk/qmk_cli@sha256:b7d7fa8fb4432b569931de5ad59098cb788f440ed61a62c5126746b71aee0f4a \
  bash -c '
    set -e
    tests/run.sh
    ref=$(cat vial-qmk.ref)
    git init -q /tmp/vial-qmk
    git -C /tmp/vial-qmk remote add origin https://github.com/vial-kb/vial-qmk.git
    git -C /tmp/vial-qmk fetch -q --depth 1 origin "$ref"
    git -C /tmp/vial-qmk checkout -q FETCH_HEAD
    git -C /tmp/vial-qmk submodule update -q --init --depth 1 --recursive -j 4
    scripts/build.sh /tmp/vial-qmk out
  '
```

With Docker, replace `podman` by `docker` and drop `:Z`. On Windows, run it in WSL.
The result is in `out/` (owned by root when using Docker).

## Option B: local QMK toolchain (Linux/macOS)

1. Install the QMK toolchain as described in the
   [QMK docs](https://docs.qmk.fm/newbs_getting_started) (`arm-none-eabi-gcc`, `make`, Python 3).
2. Clone vial-qmk at the pinned commit:

   ```bash
   git clone https://github.com/vial-kb/vial-qmk.git ~/vial-qmk
   git -C ~/vial-qmk checkout "$(cat vial-qmk.ref)"
   git -C ~/vial-qmk submodule update --init --recursive
   ```

3. Build:

   ```bash
   tests/run.sh                      # optional: PIN lock host simulation (needs gcc)
   scripts/build.sh ~/vial-qmk out
   ```

`build.sh` patches the vial-qmk checkout in place (idempotent) and replaces its
`keyboards/mykb` folder. Use a dedicated checkout.

## Output

| File | Purpose |
|------|---------|
| `s9b-first-install.uf2` | Firmware plus an erase of the settings area (`0x0801E000`–`0x0801FFFF`). Use when coming from the vendor firmware or to start clean. |
| `s9b-update.uf2` | Firmware only. Meant to keep keymap, macros and PIN; still save the layout in Vial (*File → Save current layout*) before flashing. |
| `SHA256SUMS` | Checksums of both UF2 files. |
| `s9b.elf`, `s9b-app.bin` | For debugging / SWD flashing (`s9b-app.bin` goes to `0x08004000`). |

The script fails if the firmware does not fit into the 104 KB between bootloader and settings.

## Things you may want to change

| What | Where |
|------|-------|
| Host keyboard layout of macro text | `keyboards/mykb/s9b/keymaps/vial/keymap.c`: replace `sendstring_german.h` by another `quantum/keymap_extras/sendstring_*.h`. The *User* keycodes in `s9b.c` / `vial.json` are German-specific. |
| PIN length, timings, waiting times | `PINLOCK_*` in `keyboards/mykb/s9b/config.h` |
| Hue shift per layer | `LAYER_HUE_STEP` in `keyboards/mykb/s9b/config.h` |
| Layers, macros, tap dances | `keyboards/mykb/s9b/keymaps/vial/config.h` |
| Default keymap | `keyboards/mykb/s9b/keymaps/vial/keymap.c` |

**Changing the EEPROM layout** (number of layers, macros or tap dances, Vial features, QMK
settings, `EECONFIG_KB_DATA_SIZE`): bump the value in [`eeprom-layout.id`](../eeprom-layout.id).
Vial resets keymap and macros once when this id changes; without the bump, old data would be
misread. (Upstream vial-qmk uses a random id per build, which would reset everything on every
update; [`patches/0003`](../patches/0003-vial-fixed-build-id.patch) makes it fixed.)

**Changing `PINLOCK_LENGTH`** drops a stored PIN of a different length on the next start.

Do not change `VIAL_KEYBOARD_UID` unless you build for different hardware: Vial uses it to match
saved layouts to the keyboard.

## Patches

| Patch | Why |
|-------|-----|
| `0001-via-command-permission-hook` | Adds hooks to `quantum/via.c` so the PIN lock can refuse VIA/Vial commands (reading macros, bootloader jump) while locked. |
| `0002-send-string-drop-non-ascii` | `send_string` read past the end of its lookup tables for bytes ≥ 0x80 (e.g. UTF-8 from Vial's text field); such bytes are now skipped. |
| `0003-vial-fixed-build-id` | Takes Vial's EEPROM build id from the environment (`eeprom-layout.id`) instead of a random value. |

## Updating vial-qmk

1. Put the new commit into `vial-qmk.ref`.
2. Build; if a patch no longer applies, rebase it against the new vial-qmk.
3. Check the vial-qmk and QMK changelogs for EEPROM layout changes and bump `eeprom-layout.id`
   if needed.
4. Test on the device: text output, LEDs, lock/unlock, update keeps the keymap.

## Releasing (maintainer)

1. Merge the changes via pull request; add a `## [X.Y.Z]` section to `CHANGELOG.md`.
2. Tag the merged commit on `main` and push the tag: `git tag -a vX.Y.Z -m "S9B Vial firmware X.Y.Z" && git push origin vX.Y.Z`.
3. GitHub Actions builds and publishes the release with the changelog section as notes. Without
   that section the release step fails on purpose.
