# S9B uses the YRKB UF2 bootloader (16K at 0x08000000, app at 0x08004000).
# QMK's "uf2boot" uses a different magic word, so the jump is implemented in s9b.c.
BOARD = STM32_F103_STM32DUINO
MCU_LDSCRIPT = s9b_yrkb
FIRMWARE_FORMAT = uf2

# Wildcard to allow APM32 MCU
DFU_SUFFIX_ARGS = -p FFFF -v FFFF

SRC += sha256.c
