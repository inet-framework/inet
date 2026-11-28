# IEEE 802.15.4 — Level 1 model claim survey

> **Kind:** report · **Status:** snapshot 2026-09-18 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ieee802154/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

## Scope and result

Surveyed INET source revision `c913a63a8335ca39a319cb2089daf835ea5e95cd`, the
[user guide](../../../../../doc/src/users-guide/ch-802154.rst), and the link-layer and PHY
IEEE 802.15.4 subtrees. **Level 1 (Survey)** is established for these model families: the
reference edition is pinned, the documented and implemented claim areas below are mapped
to it, and older edition references are identified. This is not a conformance verdict or
a claim that every optional feature is implemented. No simulation was run.

## Claims and reference mapping

Source links identify claim evidence, not proof that behavior satisfies the standard.
The 2024 clauses are review destinations; they do not assert equivalence to older editions.

| Claim area and evidence | Claimed edition or limitation | 2024 review destination |
| --- | --- | --- |
| Generic MAC with ACKs and selectable constant, linear, or exponential backoff: [Ieee802154Mac.ned](../../../../../src/inet/linklayer/ieee802154/Ieee802154Mac.ned) | No edition pinned; generic alternatives are not all IEEE CSMA-CA claims | 6.3.1, 6.3.2.1, 6.6 |
| Narrowband MAC payload/header sizes, exponential backoff bounds, retry limit: [Ieee802154NarrowbandMac.ned](../../../../../src/inet/linklayer/ieee802154/Ieee802154NarrowbandMac.ned) | Explicit 2006 references; RX setup/CCA comment marks uncertainty | 6.3.2.1, 6.6.3, 7, 8.4, 11 |
| ACK wait/retry, reception filtering, duplicate handling, and sequence IDs: [Ieee802154Mac.cc](../../../../../src/inet/linklayer/ieee802154/Ieee802154Mac.cc), particularly `manageMissingAck`, `handleLowerPacket`, and `scheduleBackoff` | Implemented behavior is a claim; no 2024 conformance evidence | 6.3.2.1, 6.6.2, 6.6.3, 7.2 |
| MAC header representation and wire serialization: [header](../../../../../src/inet/linklayer/ieee802154/Ieee802154MacHeader.msg), [serializer](../../../../../src/inet/linklayer/ieee802154/Ieee802154MacHeaderSerializer.cc) | Serializer explicitly stores a payload protocol ID in the Source PAN ID field and pads internal 48-bit addresses to 64 bits | 7.2 and 7.3; wire-format fidelity needs separate checks |
| Narrowband interface assembly: [Ieee802154NarrowbandInterface.ned](../../../../../src/inet/linklayer/ieee802154/Ieee802154NarrowbandInterface.ned) | Uses the narrowband MAC and selectable radio | MAC 6–9 and PHY 11–13 |
| Narrowband packet timing, 250 kbps rate and PHY header/preamble: [radio](../../../../../src/inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandRadio.ned), [transmitter](../../../../../src/inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandTransmitter.cc) | 2006 citations; device-derived sensitivity/power settings and acknowledged sensitivity approximation | 11–13, especially O-QPSK applicability |
| Narrowband reception and BER/PER/SER: [receiver](../../../../../src/inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandReceiver.cc), [error model](../../../../../src/inet/physicallayer/wireless/ieee802154/packetlevel/errormodel/Ieee802154ErrorModel.cc) | Packet-level error abstraction; no waveform conformance claim established | 11 and 13 |
| UWB pulse, preamble, hopping and mode constants: [mode](../../../../../src/inet/physicallayer/wireless/ieee802154/bitlevel/Ieee802154UwbIrMode.h), [transmitter](../../../../../src/inet/physicallayer/wireless/ieee802154/bitlevel/Ieee802154UwbIrTransmitter.cc), [receiver](../../../../../src/inet/physicallayer/wireless/ieee802154/bitlevel/Ieee802154UwbIrReceiver.cc) | Header names IEEE 802.15.4A; 2007 amendment is historical context, exact edition comparison remains open | 16 (HRP UWB); does not imply all modern UWB modes or ranging services |
| UWB interface assembly: [Ieee802154UwbIrInterface.ned](../../../../../src/inet/linklayer/ieee802154/Ieee802154UwbIrInterface.ned) | Uses `AckingMac`, not `Ieee802154NarrowbandMac`; narrowband MAC claims cannot be transferred to it | Separate MAC applicability review before 6–9 checks |

## Older claims and survey limitations

The explicit 2006 citations and the 802.15.4A description predate the pinned reference.
They remain the model's historical claims; the availability of 2024 does not upgrade them.
The old PDF texts are absent from this corpus, so exact cross-edition compatibility remains
unverified. Broad introductory descriptions of security, guaranteed time slots, and power
management in the user guide describe the standard; they do not establish model support.

The user's guide calls the scalar radio a partially parameterized APSK radio; the current
NED hierarchy uses `Ieee802154NarrowbandRadio` and `FlatRadioBase`. Treat that guide wording
as a documentation mismatch to resolve before deriving precise PHY claims.

No clause-level pass/fail verdict, full mandatory-statement inventory, or absence-of-code
proof for every optional feature is established by this survey. The
[coverage ledger](coverage.md) records the next evidence needed.
