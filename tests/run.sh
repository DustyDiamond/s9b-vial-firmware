#!/usr/bin/env bash
# Copyright 2026 DustyDiamond
# SPDX-License-Identifier: GPL-2.0-or-later
# Host simulation of the PIN lock logic in keyboards/mykb/s9b/s9b.c
set -euo pipefail
cd "$(dirname "$0")"
KB=../keyboards/mykb/s9b
gcc -std=gnu11 -Wall -Wextra -Werror -Wno-unused-parameter -O1 \
    -Istubs -I"$KB" \
    -DQMK_KEYBOARD_H='"qmk_stub.h"' -DVIA_ENABLE -DVIAL_ENABLE -DS9B_HOST_TEST \
    -DS9B_UID_PTR='((const uint8_t *)"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c")' \
    -include "$KB/config.h" \
    -o /tmp/test_pinlock test_pinlock.c "$KB/s9b.c" "$KB/sha256.c"
/tmp/test_pinlock
