# UDP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around UDP, records which document governs each contested clause, and pins the exact set
that the current pass tests against.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-08.

## Target level

**Level 2 — Core** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
The in-scope set below holds the base document and the companion that carries its error
report. UDP is the smallest protocol in this tree, and the level table shows it:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 3, Edge | RFC 1122 §4.1 | the host requirements make the checksum a MUST to implement and a MUST to check, and say what a host does with an ICMP error; RFC 768 says none of that |
| level 4, Dynamics | nothing | UDP has no timer and no control loop; level 4 is empty for this protocol, and the ledger will say `not applicable` rather than `not started` |
| level 5, Complete | RFC 9868 | the UDP options in the surplus area beyond the length field |

What the pass actually reached is not recorded here. It is in
[`model/udp/coverage.md`](../../model/udp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 768 | User Datagram Protocol | 28 August 1980 | Internet Standard (STD 6) | `base` | [`standard/rfc768/`](../../standard/rfc768/rfc768.txt), 2026-09-08 |
| RFC 792 | Internet Control Message Protocol | September 1981 | Internet Standard | `companion` (the port unreachable report) | [`standard/rfc792/`](../../standard/rfc792/rfc792.txt), 2026-09-02; one copy, shared with IPv4 |
| RFC 9868 | Transport Options for UDP | October 2025 | Proposed Standard | `updates` RFC 768 | no |
| RFC 1122 | Requirements for Internet Hosts — Communication Layers | October 1989 | Internet Standard | `companion` (§4.1, the UDP host requirements) | no |
| RFC 8085 | UDP Usage Guidelines | March 2017 | Best Current Practice | `companion`; guidance for applications, not requirements on the UDP module | no |
| RFC 6935, RFC 6936 | IPv6 and UDP checksums for tunneled packets; applicability | May 2013 | Proposed Standard | `companion`; IPv6 only | no |

Source of the cached text:

- `rfc768.txt` in [`evidence/standard/rfc768/`](../../standard/rfc768/) —
  <https://www.rfc-editor.org/rfc/rfc768.txt>, downloaded 2026-09-08.

The RFC 792 catalog is the one that IPv4 uses; this pass adds one entry to it, the port
unreachable report, and changes nothing else in it.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Checksum optional or required | RFC 768, `rfc768.txt:92-95`: an all-zero checksum means none was generated | RFC 1122 §4.1.3.4 `UDP Checksums`: a host MUST implement the checksum, MUST check a received nonzero one and discard on failure, SHOULD enable it by default | RFC 1122 | no |
| Checksum over IPv6 | RFC 768 permits a zero checksum | RFC 8200 §8.1 makes the checksum mandatory over IPv6; RFC 6935 and RFC 6936 carve out tunnel exceptions | RFC 8200 | no; IPv6 is out of scope |
| The header beyond the length | RFC 768 defines four fields | RFC 9868 defines options in the area between the UDP length and the IP length | RFC 9868 | no |

No row is in scope, so no catalog entry carries an `Overridden by` field. The first row is
the one that matters for a level 3 pass: at level 2 the checksum is an optional feature
because RFC 768 says so, and RFC 1122 will turn it into a mandatory one.

## In-scope set

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 768 | August 1980, Internet Standard, no revision since | [`standard/rfc768/catalog.md`](../../standard/rfc768/catalog.md) |
| RFC 792 | September 1981, Internet Standard; only the port unreachable entry serves UDP | [`standard/rfc792/catalog.md`](../../standard/rfc792/catalog.md) |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 1122 | Level 3. It changes the level of the checksum feature and adds the discard rule; both need a corrupted datagram in flight, which is the level 3 toolset. |
| RFC 9868 | Level 5. Options are optional per datagram and a new mechanism; the level table of the guide files them at level 5. |
| RFC 8085 | A best current practice for applications; it states no behavior of the UDP module that a protocol test can observe. |
| RFC 6935, RFC 6936 | IPv6 only. |
