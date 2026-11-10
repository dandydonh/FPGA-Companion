# Why add Companion readiness announcements?

## The problem

The Zork FPGA project on Tang Nano 20K revision 3923 supports two startup input
paths: a serial terminal connected to a PC, or a USB keyboard handled by the
onboard BL616 running FPGA-Companion. The FPGA must select the appropriate
routing because the board connections used for serial and Companion SPI overlap.
The existing Zork bitstream defaults to USB and requires an S1 button override
for serial. A user without an active Companion can therefore boot into USB mode
and have no usable input through the expected serial connection.

We want automatic startup selection: use USB when Companion reports that its
USB host controller is ready; otherwise allow a bounded serial fallback when
there is no evidence of conflicting Companion activity. Preserve manual overrides.

The reviewed stock Companion does not provide the explicit, periodic readiness
report needed for this decision before FPGA discovery completes. Generic SPI
discovery shows communication activity, not successful USB host initialization.
HID registration does not necessarily identify a usable keyboard, and waiting
for a keyboard packet requires user input. These signals alone do not provide
the required startup decision, particularly after FPGA-only reconfiguration.

## Why maintain a fork?

This fork provides a place to develop and distribute the optional BL616 change
separately from the Zork FPGA project. It lets us maintain a matching Companion
firmware version while the protocol and hardware behavior are validated and an
upstream proposal is considered. The patch is not upstream approved; no upstream
pull request has been submitted.

## What this patch adds

The opt-in BL616 firmware sends a versioned status frame after SPI initialization,
on state changes and every 250 ms, independently of successful FPGA discovery
or keyboard attachment. It reports controller initialization state, transport
readiness and supported keyboard count. Readiness is based on the actual USB
host controller initialization result, rather than merely calling the framework
initialization function. Reports share the existing SPI mutex and include idle
intervals for safe capture by the proposed FPGA receiver.

This supplies the missing information. A matching FPGA implementation can listen
with SPI replies disabled, select USB after a fresh host-ready report, or select
serial after a bounded silent interval. Reports indicating starting/error states
must produce diagnostics and prevent conflicting serial routing. The FPGA owns
these selection rules and manual overrides; this firmware patch does not itself
change FPGA routing. See [the protocol](PROTOCOL.md) and [build status](BUILD.md).

## Important limits

"Installed" is the user's desired shortcut, but the implementation detects
**activity and reported readiness**, not persistent firmware inventory. Silence
can mean Companion is absent, is not running, has failed, or cannot acquire the
SPI bus. This Companion-only patch cannot distinguish all those cases. Older
Companion firmware does not send the new reports; legacy traffic and conflicts
still require separate handling in the FPGA. No Partner changes are included.

The firmware compiles and linked-code inspection confirms controller interception.
