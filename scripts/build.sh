#!/usr/bin/env bash
# Copyright 2026 DustyDiamond
# SPDX-License-Identifier: GPL-2.0-or-later
# Build the S9B firmware.
# Usage: scripts/build.sh <path-to-vial-qmk-checkout> [outdir]
# The vial-qmk checkout must be at VIAL_QMK_REF (see vial-qmk.ref) with submodules.
set -euo pipefail

QMK_DIR=$(realpath "$1")
OUT_DIR=$(realpath -m "${2:-out}")
REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)

mkdir -p "$OUT_DIR"

# 1. patches (idempotent: skip if already applied)
for p in "$REPO_DIR"/patches/*.patch; do
    if git -C "$QMK_DIR" apply --check "$p" 2>/dev/null; then
        git -C "$QMK_DIR" apply "$p"
        echo "applied $(basename "$p")"
    elif git -C "$QMK_DIR" apply --reverse --check "$p" 2>/dev/null; then
        echo "already applied $(basename "$p")"
    else
        echo "patch does not apply: $p" >&2
        exit 1
    fi
done

# 2. keyboard sources
rm -rf "$QMK_DIR/keyboards/mykb"
cp -r "$REPO_DIR/keyboards/mykb" "$QMK_DIR/keyboards/mykb"

# 3. build
# Fixed Vial BUILD_ID: keymap and macros survive updates. Bump eeprom-layout.id
# whenever the EEPROM layout changes (layer/macro count, Vial features, QMK settings).
export VIAL_BUILD_ID=$(cat "$REPO_DIR/eeprom-layout.id")
make -C "$QMK_DIR" -j"$(nproc)" mykb/s9b:vial 2>&1 | grep -v "^Compiling: \\|userspace" || true
test -f "$QMK_DIR/.build/mykb_s9b_vial.elf"

# 4. outputs
ELF="$QMK_DIR/.build/mykb_s9b_vial.elf"
BIN="$OUT_DIR/s9b-app.bin"
arm-none-eabi-objcopy -O binary "$ELF" "$BIN"
cp "$ELF" "$OUT_DIR/s9b.elf"
python3 "$REPO_DIR/scripts/mkuf2.py" --base 0x08004000 "$BIN" "$OUT_DIR/s9b-update.uf2"

# First install: same code plus 0xFF blocks for the EEPROM area
# (0x0801E000-0x0801FFFF), which wipes the vendor firmware's stored macros.
python3 "$REPO_DIR/scripts/mkuf2.py" --base 0x08004000 --erase 0x0801E000:0x08020000 \
    "$BIN" "$OUT_DIR/s9b-first-install.uf2"

python3 - "$BIN" <<'PY'
import os, sys
size = os.path.getsize(sys.argv[1])
limit = 128*1024 - 16*1024 - 8*1024
print(f"app size {size} bytes, limit {limit} ({100*size/limit:.0f} %)")
assert size <= limit, "firmware too large"
PY
(cd "$OUT_DIR" && sha256sum s9b-update.uf2 s9b-first-install.uf2 | tee SHA256SUMS)
