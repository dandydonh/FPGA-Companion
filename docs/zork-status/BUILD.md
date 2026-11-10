# Experimental BL616 status-announcement branch

This branch is based on FPGA-Companion v1.4.33 commit
`cc741e8a48a511c102a7b9660706999d261991d2`. It contains an opt-in BL616
readiness reporter for Tang Nano 20K revision3923. It is not upstream approved.
No pull request has been submitted and protocol allocation remains provisional.

## Build locally

Use a persistent installation of Bouffalo GCC14.3.0/Picolibc1.8.10 and the official
SDK `https://github.com/bouffalolab/bouffalo_sdk` at
`eba87edced5a03ce8bcad8cf6002494982f83a5f`. Initialize only the application u8g2
submodule at its recorded revision `dc9fc73ddfc4fc8690afea370780ba322bd0cd79`:

```bash
git submodule update --init src/u8g2
export BL_SDK_BASE=/absolute/path/to/bouffalo_sdk
export CROSS_COMPILE=/absolute/path/to/riscv64-unknown-elf/bin/riscv64-unknown-elf-
export PATH="/absolute/path/to/riscv64-unknown-elf/bin:$PATH"
make -C src/bl616 TANG_BOARD=nano20k_v3923 COMPANION_STATUS_ANNOUNCE=ON BUILD_DIR=build-status
```

Install remaining host prerequisites required by the pinned SDK. Keep compiler,
SDK and u8g2 locally; rebuilding does not require downloading them again. Build
output is `src/bl616/build-status/build_out/fpga_companion_bl616.bin`.
The option defaults OFF; ON is limited to nano20k_v3923 and rejects compile LTO
so the controller interception can be inspected. Final-link LTO may still be
added by the SDK. No other MCU/board implementation is claimed.

The application compatibility adjustments add an explicit time.h include and
preserve existing SDK timer/errno definitions. No Partner code is changed.

## Evidence and remaining work

The exact seven-file source patch in this branch was built in a separate local
working tree with the pinned inputs above. Binary and ELF hashes are recorded
in VALIDATION.json; path-sensitive output means another build need not be
byte-identical. Object relocations and linked disassembly confirmed that the
USB hub invokes both __wrap_usb_hc_init/deinit functions, which call the real
controller functions and reporter updates. ELF TLS metadata/boot symbols exist.
Inherited application/SDK compiler warnings remain.

There has been no hardware installation or runtime validation. TLS/stack sizing,
USB behavior, legacy-core compatibility and paired FPGA startup behavior still
need testing. The matching Zork FPGA source is not built/validated. Announcements
cannot prove installed-firmware inventory when this application is not running.
This patch is not a demonstrated fix for intermittent startup failures.

PROTOCOL.md describes the private wire draft. INSTALL.md is the separate,
experimental Companion-only installation procedure for a future supplied binary
bundle, not an instruction to flash as part of building. No binaries, logs,
board backups, SDK/toolchain or generated outputs are committed in this branch.
Do not use SDK-generated standalone-board flash configs on the Partner setup.

## Upstream proposal scope

Propose optional periodic Companion role/readiness reports before FPGA discovery,
independent of keypresses, with serialized transactions and bounded mutex waits.
Ask upstream to allocate a suitable target/command and review transport timing,
BL616 readiness hooks and a generic format for other MCUs. Keep FPGA selection
policy and Partner inventory outside this patch. Update the proposal with actual
hardware results before requesting upstream review. Submission requires the
owner's explicit approval.
