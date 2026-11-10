# Makefile for the MiSTeryNano firmware

COMX?=/dev/m0s_debugger
BAUDRATE?=2000000

SDK_DEMO_PATH ?= $(abspath .)
BL_SDK_BASE ?= $(abspath  ./../../../..)

CHIP ?= bl616
BOARD ?= bl616dk
TANG_BOARD ?= m0sdock
CROSS_COMPILE ?= riscv64-unknown-elf-
cmake_definition+=-DTANG_BOARD=$(TANG_BOARD)
COMPANION_STATUS_ANNOUNCE ?= OFF
cmake_definition+=-DCOMPANION_STATUS_ANNOUNCE=$(COMPANION_STATUS_ANNOUNCE)

include $(BL_SDK_BASE)/project.build

