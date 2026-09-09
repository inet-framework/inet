# UDP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around UDP, records which document governs each contested clause, and pins the exact set
that the current pass tests against.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-08.

## Target level

**Level 3 — Edge** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
The in-scope set below holds the base document, the companion that carries its error
report, and the host requirements. UDP is the smallest protocol in this tree, and the level
table shows it:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 3, Edge | RFC 1122 §4.1 | in scope now. The host requirements make the checksum a MUST to implement and a MUST to check, and say what a host does with an ICMP error; RFC 768 says none of that |
| level 4, Dynamics | nothing | UDP has no timer and no control loop; level 4 is empty for this protocol, and the ledger says `not applicable` rather than `not started` |
| level 5, Complete | RFC 9868 | the UDP options in the surplus area beyond the length field |

What the pass actually reached is not recorded here. It is in
[`model/udp/coverage.md`](../../model/udp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 768 | User Datagram Protocol | 28 August 1980 | Internet Standard (STD 6) | `base` | [`standard/rfc768/`](../../standard/rfc768/rfc768.txt), 2026-09-08 |
| RFC 792 | Internet Control Message Protocol | September 1981 | Internet Standard | `companion` (the port unreachable report) | [`standard/rfc792/`](../../standard/rfc792/rfc792.txt), 2026-09-02; one copy, shared with IPv4 |
| RFC 9868 | Transport Options for UDP | October 2025 | Proposed Standard | `updates` RFC 768 | no |
| RFC 1122 | Requirements for Internet Hosts — Communication Layers | October 1989 | Internet Standard | `companion` (§4.1, the UDP host requirements) | [`standard/rfc1122/`](../../standard/rfc1122/rfc1122.txt), 2026-09-09; one copy, shared with IPv4 |
| RFC 8085 | UDP Usage Guidelines | March 2017 | Best Current Practice | `companion`; guidance for applications, not requirements on the UDP module | no |
| RFC 6935, RFC 6936 | IPv6 and UDP checksums for tunneled packets; applicability | May 2013 | Proposed Standard | `companion`; IPv6 only | no |

Source of the cached text:

- `rfc768.txt` in [`evidence/standard/rfc768/`](../../standard/rfc768/) —
  <https://www.rfc-editor.org/rfc/rfc768.txt>, downloaded 2026-09-08.

The RFC 792 catalog is the one that IPv4 uses; the level 2 pass added one entry to it, the
port unreachable report, and changed nothing else in it. The RFC 1122 catalog is shared in
the same way. IPv4 owns the entries of §3 there; the level 3 pass of UDP added the entries
of §4.1, which start with `U`, and changed the scope statement of the document to name both
protocols.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Checksum optional or required | RFC 768, `rfc768.txt:92-95`: an all-zero checksum means none was generated | RFC 1122 §4.1.3.4 `UDP Checksums`: a host MUST implement the checksum, MUST check a received nonzero one and discard on failure, and MUST have it on by default | RFC 1122 | yes, from level 3 |
| Checksum over IPv6 | RFC 768 permits a zero checksum | RFC 8200 §8.1 makes the checksum mandatory over IPv6; RFC 6935 and RFC 6936 carve out tunnel exceptions | RFC 8200 | no; IPv6 is out of scope |
| The header beyond the length | RFC 768 defines four fields | RFC 9868 defines options in the area between the UDP length and the IP length | RFC 9868 | no |

The first row is in scope from level 3 on, and the catalog entry it names carries an
`Overridden by` field: [RFC768-CKSUM-1](../../standard/rfc768/catalog.md#rfc768-cksum-1)
points at [RFC1122-UCK-1](../../standard/rfc1122/catalog.md#rfc1122-uck-1). The change is
the one that matters most for this protocol: at level 2 the checksum is an optional feature
because RFC 768 says so, and RFC 1122 turns it into a mandatory one. RFC 1122 also raises
the strength of the closed-port report, from the may of RFC 792 to a should
([RFC1122-UPORT-1](../../standard/rfc1122/catalog.md#rfc1122-uport-1)). The other two rows
stay out of scope.

## In-scope set

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 768 | August 1980, Internet Standard, no revision since | [`standard/rfc768/catalog.md`](../../standard/rfc768/catalog.md) |
| RFC 792 | September 1981, Internet Standard; only the port unreachable entry serves UDP | [`standard/rfc792/catalog.md`](../../standard/rfc792/catalog.md) |
| RFC 1122 | October 1989, Internet Standard; §4.1 only, the host requirements for UDP | [`standard/rfc1122/catalog.md`](../../standard/rfc1122/catalog.md) |

Out of scope, with the reason:

| Document | Reason |
| RFC 9868 | Level 5. Options are optional per datagram and a new mechanism; the level table of the guide files them at level 5. |
| RFC 8085 | A best current practice for applications; it states no behavior of the UDP module that a protocol test can observe. |
| RFC 6935, RFC 6936 | IPv6 only. |
