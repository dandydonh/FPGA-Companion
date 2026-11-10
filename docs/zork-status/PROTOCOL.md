# Experimental Companion status protocol

Implemented sender subset: reporter1, unknown/host role only, flags0–3. Reporter2, PC serial role and inventory flags are reserved and never emitted. Target/command allocation remains provisional.

## Transport and frame

Use the existing BL616-master SPI mode 1, MSB first. One complete message occupies one active-low CS transaction. The FPGA listens without enabling MISO replies, and the sender ignores MISO. No serial-pin probing, GPIO additions, FPGA-originated poll, or acknowledgement is required for discovery of the announcement.

Proposed fixed 28-byte frame, with multibyte integers little-endian:

| Byte offset | Size | Field / v1 value |
| --- | --- | --- |
| 0 | 1 | Private target `0x7e`; not allocated by upstream. |
| 1 | 1 | Command `0x01`, status announcement |
| 2 | 4 | Magic ASCII `ZKCP` (`5a 4b 43 50`) |
| 6 | 1 | Protocol major version `0x01` |
| 7 | 1 | Total frame length `0x1c` (28) |
| 8 | 1 | Reporter: `1` Companion, `2` cooperating Partner; other values unsupported |
| 9 | 1 | USB role: `0` unknown, `1` host, `2` PC serial bridge |
| 10 | 1 | Stage: `0` starting, `1` ready, `2` error, `3` stopping |
| 11 | 1 | Flags, defined below |
| 12 | 4 | Sequence number, modulo 2^32 |
| 16 | 4 | Sender uptime in milliseconds, modulo 2^32 |
| 20 | 2 | Supported keyboard count |
| 22 | 2 | Diagnostic code: `0` no reported error; nonzero values allocated with the implementation |
| 24 | 2 | Reserved, both zero for v1 |
| 26 | 2 | CRC-16/CCITT-FALSE of bytes 0–25, transmitted **high byte first** (the exception to integer endianness) |

CRC parameters: polynomial `0x1021`, init `0xffff`, refin=false, refout=false, xorout=`0x0000`. This detects corruption/false matches; it is not sender authentication. Sequence and uptime are diagnostic freshness aids, not persistent identifiers or proof of health.

Flags byte:

- bit0: role is authoritative for this report; clear means byte9 must be unknown.
- bit1: USB host controller initialization/readiness checks succeeded, as defined by the pinned SDK implementation.
- bit2: sender's SPI transport initialization is ready. It does not mean that FPGA discovery/configuration succeeded or prove the entire game input path.
- bit3: supported keyboard count is nonzero and the associated descriptor/client setup succeeded.
- bit4: installed-firmware inventory is valid, supported only by a cooperating Partner reporter in this draft.
- bit5: Companion is installed, meaningful only when bit4 is set.
- bits6–7: reserved zero.

A Companion report leaves bits4–5 clear. It cannot report persistent installation or absence solely from execution. A Companion-only implementation must not claim PC serial role; that would require authoritative bridge state from Partner. For unknown role, transmit byte9=0 and bit0=0. Never turn a call to `usb_host()` into an authoritative ready flag without checking the controller's actual result/state.

For automatic USB selection require: valid complete frame, supported version and reporter, authoritative host role, ready stage, and controller/transport-ready flags. Keyboard-present is deliberately not required: USB mode must also support later keyboard attachment. A host-starting/error report is positive conflict/diagnostic evidence, not permission to enable serial.


Sender runs every250ms plus state changes after SPI initialization, independently of core discovery. Nonblocking shared mutex acquisition skips a busy bus; CS remains high and SCK stopped for at least1us before and after each frame while the mutex is held. USB readiness comes from the wrapped bus0 controller initialization result, and deinitialization clears readiness. No Partner inventory or silent-FPGA installation claim is supported.
