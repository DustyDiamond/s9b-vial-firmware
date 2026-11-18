# Selbst bauen

[English](BUILDING.md)

Die Firmware besteht aus vial-qmk in einem festen Commit plus dem Keyboard-Ordner und drei
kleinen Patches aus diesem Repository. `scripts/build.sh` spielt die Patches ein, kopiert das
Keyboard, baut und schreibt die UF2-Dateien. Jedes Release auf GitHub baut GitHub Actions genau
so ([`.github/workflows/build.yml`](../.github/workflows/build.yml)).

Der Build ist reproduzierbar: Mit Weg A stimmt deine `SHA256SUMS` Byte für Byte mit der des
Releases überein. So lässt sich prüfen, dass eine heruntergeladene UF2 genau aus diesem Quellcode stammt.

## Weg A: Container (empfohlen, jedes System mit Podman oder Docker)

Nutzt dasselbe fest gepinnte QMK-Build-Image wie die CI, sonst muss nichts installiert sein.

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

Mit Docker `podman` durch `docker` ersetzen und `:Z` weglassen. Unter Windows in WSL ausführen.
Das Ergebnis liegt in `out/` (mit Docker gehört es root).

## Weg B: lokale QMK-Toolchain (Linux/macOS)

1. QMK-Toolchain nach der [QMK-Anleitung](https://docs.qmk.fm/newbs_getting_started) installieren
   (`arm-none-eabi-gcc`, `make`, Python 3).
2. vial-qmk im festen Commit klonen:

   ```bash
   git clone https://github.com/vial-kb/vial-qmk.git ~/vial-qmk
   git -C ~/vial-qmk checkout "$(cat vial-qmk.ref)"
   git -C ~/vial-qmk submodule update --init --recursive
   ```

3. Bauen:

   ```bash
   tests/run.sh                      # optional: Simulation der PIN-Sperre (braucht gcc)
   scripts/build.sh ~/vial-qmk out
   ```

`build.sh` patcht den vial-qmk-Checkout direkt (mehrfach ausführbar) und ersetzt dessen Ordner
`keyboards/mykb`. Am besten einen eigenen Checkout dafür nehmen.

## Ergebnis

| Datei | Zweck |
|-------|-------|
| `s9b-first-install.uf2` | Firmware plus Löschen des Einstellungsbereichs (`0x0801E000`–`0x0801FFFF`). Beim Wechsel von der Herstellerfirmware oder für einen sauberen Neustart. |
| `s9b-update.uf2` | Nur die Firmware. Soll Belegung, Makros und PIN behalten; trotzdem vor dem Flashen die Belegung in Vial sichern (*File → Save current layout*). |
| `SHA256SUMS` | Prüfsummen beider UF2-Dateien. |
| `s9b.elf`, `s9b-app.bin` | Zum Debuggen / Flashen per SWD (`s9b-app.bin` gehört nach `0x08004000`). |

Das Skript bricht ab, wenn die Firmware nicht in die 104 KB zwischen Bootloader und
Einstellungsbereich passt.

## Was man anpassen kann

| Was | Wo |
|-----|----|
| Tastaturbelegung des PCs für Makrotext | `keyboards/mykb/s9b/keymaps/vial/keymap.c`: `sendstring_german.h` durch ein anderes `quantum/keymap_extras/sendstring_*.h` ersetzen. Die *User*-Keycodes in `s9b.c` / `vial.json` sind deutsch-spezifisch. |
| PIN-Länge, Zeiten, Wartezeiten | `PINLOCK_*` in `keyboards/mykb/s9b/config.h` |
| Farbtonverschiebung je Ebene | `LAYER_HUE_STEP` in `keyboards/mykb/s9b/config.h` |
| Ebenen, Makros, Tap-Dances | `keyboards/mykb/s9b/keymaps/vial/config.h` |
| Standardbelegung | `keyboards/mykb/s9b/keymaps/vial/keymap.c` |

**Änderung am EEPROM-Layout** (Anzahl Ebenen, Makros oder Tap-Dances, Vial-Funktionen,
QMK-Einstellungen, `EECONFIG_KB_DATA_SIZE`): den Wert in [`eeprom-layout.id`](../eeprom-layout.id)
erhöhen. Vial setzt Belegung und Makros einmal zurück, wenn sich diese ID ändert; ohne Erhöhung
würden alte Daten falsch gelesen. (vial-qmk nimmt von Haus aus bei jedem Build eine zufällige ID,
was bei jedem Update alles zurücksetzen würde;
[`patches/0003`](../patches/0003-vial-fixed-build-id.patch) macht sie fest.)

**Änderung an `PINLOCK_LENGTH`** verwirft beim nächsten Start eine gespeicherte PIN anderer Länge.

`VIAL_KEYBOARD_UID` nur ändern, wenn du für andere Hardware baust: Vial ordnet damit gespeicherte
Layouts dem Keyboard zu.

## Patches

| Patch | Warum |
|-------|-------|
| `0001-via-command-permission-hook` | Hooks in `quantum/via.c`, damit die PIN-Sperre VIA/Vial-Befehle (Makros lesen, Sprung in den Bootloader) im gesperrten Zustand ablehnen kann. |
| `0002-send-string-drop-non-ascii` | `send_string` las bei Bytes ≥ 0x80 (z. B. UTF-8 aus Vials Textfeld) hinter das Ende seiner Tabellen; solche Bytes werden jetzt übersprungen. |
| `0003-vial-fixed-build-id` | Nimmt Vials EEPROM-Build-ID aus der Umgebung (`eeprom-layout.id`) statt eines Zufallswerts. |

## vial-qmk aktualisieren

1. Neuen Commit in `vial-qmk.ref` eintragen.
2. Bauen; passt ein Patch nicht mehr, ihn auf das neue vial-qmk anpassen.
3. In den Changelogs von vial-qmk und QMK nach Änderungen am EEPROM-Layout sehen und bei Bedarf
   `eeprom-layout.id` erhöhen.
4. Am Gerät testen: Textausgabe, LEDs, Sperren/Entsperren, Update behält die Belegung.

## Release erstellen (Maintainer)

1. Änderungen per Pull Request mergen; in `CHANGELOG.md` einen Abschnitt `## [X.Y.Z]` anlegen.
2. Den gemergten Commit auf `main` taggen und das Tag pushen: `git tag -a vX.Y.Z -m "S9B Vial firmware X.Y.Z" && git push origin vX.Y.Z`.
3. GitHub Actions baut und veröffentlicht das Release mit dem Changelog-Abschnitt als Text. Fehlt
   der Abschnitt, bricht der Release-Schritt bewusst ab.
