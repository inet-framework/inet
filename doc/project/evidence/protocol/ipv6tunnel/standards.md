# IPv6 tunneling — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around generic packet tunneling in IPv6, and pins the set that a level 2 pass tests against.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | nothing new; RFC 2473 §3 (IPv6 Tunneling, `rfc2473.txt:204`, with §3.1 to §3.4, `rfc2473.txt:292,350,360,407`), §5 (Tunnel IPv6 Header, `rfc2473.txt:862`, with §5.1, `rfc2473.txt:939`), and §6 (IPv6 Tunnel State Variables, `rfc2473.txt:1019`) | the normal path: encapsulation at the entry point, the outer header fields it sets, and decapsulation at the exit point |
| level 3, Edge | nothing new | §4 (Nested Encapsulation, `rfc2473.txt:585`, with the encapsulation limit option, loopback and routing-loop cases, `rfc2473.txt:643,679,804,820`), §7 (Packet Size Issues, `rfc2473.txt:1133`, with fragmentation, `rfc2473.txt:1166,1199`), and §8 (Error Processing and Reporting, `rfc2473.txt:1219`) all need a crafted or oversized packet to observe |
| level 4, Dynamics | nothing new | this document defines no timer and no control loop of its own; its fragmentation behaviour depends on the base IPv6 specification, already the concern of a different pass |
| level 5, Complete | nothing new | every mechanism this document defines is reachable at level 2 or 3; there is no optional area left over |

What a pass actually reached is not recorded here. It is in
[`model/ipv6tunnel/coverage.md`](../../model/ipv6tunnel/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 2473 | Generic Packet Tunneling in IPv6 Specification | December 1998 | Proposed Standard | `base` | [`standards/RFC/rfc2473.txt`](../../../../../../standards/RFC/rfc2473.txt), 2026-09-23 |

Source of the text:

- `rfc2473.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2473.txt) —
  <https://www.rfc-editor.org/rfc/rfc2473.txt>, downloaded 2026-09-23.

The register shows no `obsoletes`, `obsoleted by`, `updates` or `updated by` relation for RFC
2473 at all — checked directly. It is the sole document of this family, with no relative to add
at any level: the cleanest register entry of the three protocols in this wave.

RFC 2473 postdates RFC 2119 by a year and formally adopts its keywords
(`rfc2473.txt:101-103`: "The keywords MUST, MUST NOT, MAY, OPTIONAL, REQUIRED, RECOMMENDED,
SHALL, SHALL NOT, SHOULD, SHOULD NOT are to be interpreted as defined in RFC 2119."), but the
keywords never appear again: "MUST", "SHOULD" and "MAY" are each on exactly one line of the
36-page document — the boilerplate sentence itself. Every other statement of substance is
descriptive prose, so nearly every catalog entry a level 2 pass writes will carry the strength
`description`, the same pattern the RIP and ARP passes found in their oldest base documents, but
reached here by a document that cites RFC 2119 and then does not use it.

## Override table

No entries. RFC 2473 is the only document of the family, and the register shows no relative that
could override one of its clauses.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 2473 | December 1998, Proposed Standard, no revision of the body since | none yet; level 2 writes `standard/rfc2473/catalog.md` |

Out of scope: none. RFC 2473 has no relative in the register, so there is no document to place
in an out-of-scope table for this protocol.
