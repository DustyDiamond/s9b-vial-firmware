#!/usr/bin/env python3
# Copyright 2026 DustyDiamond
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build a UF2 file for the YRKB bootloader (STM32F1 family 0x5ee21072).

Optionally appends 0xFF blocks for an address range (erases it on flashing)."""
import argparse
import struct

MAGIC0, MAGIC1, MAGIC_END = 0x0A324655, 0x9E5D5157, 0x0AB16F30
FAMILY_STM32F1 = 0x5EE21072
FLAG_FAMILY = 0x00002000


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", type=lambda x: int(x, 0), required=True)
    ap.add_argument("--erase", help="start:end (end exclusive), filled with 0xFF")
    ap.add_argument("bin")
    ap.add_argument("out")
    a = ap.parse_args()

    data = open(a.bin, "rb").read()
    chunks = []
    for off in range(0, len(data), 256):
        chunk = data[off:off + 256]
        chunks.append((a.base + off, chunk + b"\xff" * (256 - len(chunk))))
    if a.erase:
        lo, hi = (int(x, 0) for x in a.erase.split(":"))
        for addr in range(lo, hi, 256):
            chunks.append((addr, b"\xff" * 256))

    out = bytearray()
    for n, (addr, payload) in enumerate(chunks):
        hdr = struct.pack("<8I", MAGIC0, MAGIC1, FLAG_FAMILY, addr, 256, n, len(chunks), FAMILY_STM32F1)
        out += hdr + payload + b"\x00" * (476 - 256) + struct.pack("<I", MAGIC_END)
    open(a.out, "wb").write(out)
    print(f"{a.out}: {len(chunks)} blocks")


if __name__ == "__main__":
    main()
