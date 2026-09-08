# QUIC — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around QUIC, records which document governs each contested clause, and pins the exact set
that the current pass tests against.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-08.

QUIC differs from the three protocols already in this tree in one way that shapes the whole
pass: **the base document is not self-sufficient.** RFC 9000 defines the transport, but its
handshake is defined in terms of TLS, which RFC 9001 supplies, and its loss detection and
congestion control live in RFC 9002. None of the three is obsolete, and none updates
another; they are one specification published in three parts.

## Target level

**Level 2 — Core** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
The in-scope set is RFC 9000 alone. The level table:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 3, Edge | nothing, for most of it | RFC 9000 carries its own error handling: stateless reset, connection errors, frame encoding errors, the anti-amplification limit, version negotiation. All need a crafted or dropped packet. |
| level 4, Dynamics | RFC 9002 | loss detection and congestion control, and with them the idle timeout, the probe timeout and the acknowledgment delay bound |
| level 5, Complete | RFC 9001, RFC 8999, RFC 9368, RFC 9369 | the cryptographic handshake, the version-independent properties, version negotiation, and QUIC version 2 |

What the pass actually reached is not recorded here. It is in
[`model/quic/coverage.md`](../../model/quic/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 9000 | QUIC: A UDP-Based Multiplexed and Secure Transport | May 2021 | Proposed Standard | `base` | [`standard/rfc9000/`](../../standard/rfc9000/rfc9000.txt), 2026-09-08 |
| RFC 9001 | Using TLS to Secure QUIC | May 2021 | Proposed Standard | `companion`; supplies the handshake that RFC 9000 requires | no |
| RFC 9002 | QUIC Loss Detection and Congestion Control | May 2021 | Proposed Standard | `companion`; supplies the recovery that RFC 9000 requires | no |
| RFC 8999 | Version-Independent Properties of QUIC | May 2021 | Proposed Standard | `companion`; the invariants across versions | no |
| RFC 9368 | Compatible Version Negotiation for QUIC | May 2023 | Proposed Standard | `updates` RFC 8999 | no |
| RFC 9369 | QUIC Version 2 | May 2023 | Proposed Standard | `companion`; a second version of the wire image | no |
| RFC 9114 | HTTP/3 | June 2022 | Proposed Standard | `companion`; an application above QUIC, not a requirement on it | no |

Source of the cached text:

- `rfc9000.txt` in [`evidence/standard/rfc9000/`](../../standard/rfc9000/) —
  <https://www.rfc-editor.org/rfc/rfc9000.txt>, downloaded 2026-09-08.

RFC 9000 has not been updated or obsoleted since publication. Unlike RFC 791 and RFC 793,
its 1980s-era ambiguity problem does not arise: it uses RFC 2119 keywords throughout, and
its requirements are numbered by section rather than by a requirement-summary table.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The handshake and packet protection | RFC 9000 §7, which defines the handshake in terms of CRYPTO frames and defers their content | RFC 9001, the TLS binding | RFC 9001 for the content, RFC 9000 for the carriage | no; the carriage is in scope, the content is not |
| Loss detection, congestion control, the probe timeout | RFC 9000 §13, which defines what an acknowledgment means and defers the recovery | RFC 9002 | RFC 9002 | no |
| The version field and connection identifiers across versions | RFC 9000 §17.2 for version 1 | RFC 8999, the invariants, as amended by RFC 9368 | RFC 8999 for what holds across versions | no |
| The wire image of a second version | RFC 9000 §17, version 1 | RFC 9369, version 2 | RFC 9369 for that version | no |

No row is in scope, so no catalog entry carries an `Overridden by` field. The first row is
the one that shapes this pass: a simulation model that does not implement TLS still carries
the packets that the handshake needs, and the catalog holds only the carriage.

## In-scope set

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 9000 | May 2021, Proposed Standard, never updated | [`standard/rfc9000/catalog.md`](../../standard/rfc9000/catalog.md) |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 9001 | The cryptographic handshake. A simulation model of a transport protocol commonly represents the handshake without the cryptography, and a check of TLS itself belongs to a different kind of test. Level 5. |
| RFC 9002 | Loss detection and congestion control, as RFC 5681 and RFC 6298 are for TCP: level 4, and statistical checks. |
| RFC 8999, RFC 9368 | The invariants and version negotiation. Their observable content overlaps the RFC 9000 packet header entries; a version negotiation check needs a version mismatch, which is level 3. |
| RFC 9369 | A second version of the wire image; nothing in a version 1 exchange exercises it. |
| RFC 9114 | HTTP/3 is an application above QUIC. It states no requirement on the transport that a QUIC protocol test can observe. |
