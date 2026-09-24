# Proxy Mobile IPv6 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md), [mipv6/standards.md](../mipv6/standards.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around Proxy Mobile IPv6, records which document governs each contested clause, and pins the
set that a level 2 pass tests against. Proxy Mobile IPv6 reuses the Mobility Header that
[Mobile IPv6](../mipv6/standards.md) defines; this pass reads the same RFC 6275 text as that
pass.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 5213 §5 (Local Mobility Anchor Operation, `rfc5213.txt:939`), §6 (Mobile Access Gateway Operation, `rfc5213.txt:2260`), §7 (Mobile Node Operation, `rfc5213.txt:3741`) and §8 (Message Formats, `rfc5213.txt:3828`); RFC 4283 §3 (Mobile Node Identifier option, `rfc4283.txt:125`), because RFC 5213 makes this option mandatory for its own exchange (see the override table); and the RFC 6275 clauses this document names, listed below | the normal exchange: a Mobile Access Gateway detects a mobile node, sends a Proxy Binding Update, the Local Mobility Anchor answers and re-points the tunnel, and the home network prefix stays stable across a handover |
| level 3, Edge | RFC 6543 | in scope now. It closes the "out of scope for this specification" gap that RFC 5213 §6.8 leaves open (see the override table): a fixed link-local and link-layer address for a Mobile Access Gateway's access link, checkable once the core exchange runs |
| level 4, Dynamics | nothing new | RFC 5213 §6.9.4 (Retransmissions and Rate Limiting, `rfc5213.txt:3216`) and the binding lifetime timers of §5.3.3 and §5.3.4 need a tolerance; both reuse the retransmission rule of RFC 6275 §11.8 |
| level 5, Complete | RFC 7864, RFC 5844, and the optional areas of RFC 5213 itself: §5.4 (Multihoming Support), §5.7 (Local Mobility Anchor Address Discovery), §5.9 (Route Optimization Considerations) | flow mobility over multiple interfaces, IPv4 transport and IPv4 home-address support, more than one anchor per mobile node, and route optimization — all extensions beyond the single-LMA, single-interface handover this pass targets |

What a pass actually reached is not recorded here. It is in
[`model/pmipv6/coverage.md`](../../model/pmipv6/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 5213 | Proxy Mobile IPv6 | August 2008 | Proposed Standard | `base` | [`standards/RFC/rfc5213.txt`](../../../../../../standards/RFC/rfc5213.txt), 2026-09-23 |
| RFC 4283 | Mobile Node Identifier Option for Mobile IPv6 (MIPv6) | December 2005 | Proposed Standard | `companion`, made mandatory for this exchange by RFC 5213 (see override table) | [`standards/RFC/rfc4283.txt`](../../../../../../standards/RFC/rfc4283.txt), 2026-09-23 |
| RFC 6275 | Mobility Support in IPv6 | July 2011 | Proposed Standard | `companion`; the Mobility Header this document's messages extend | [`standards/RFC/rfc6275.txt`](../../../../../../standards/RFC/rfc6275.txt), downloaded by the [Mobile IPv6 pass](../mipv6/standards.md), 2026-09-23 |
| RFC 6543 | Reserved IPv6 Interface Identifier for Proxy Mobile IPv6 | May 2012 | Proposed Standard | `updates` RFC 5213 | no |
| RFC 7864 | Proxy Mobile IPv6 Extensions to Support Flow Mobility | May 2016 | Proposed Standard | `updates` RFC 5213 | no |
| RFC 5844 | IPv4 Support for Proxy Mobile IPv6 | May 2010 | Proposed Standard | `companion`; adds IPv4 | no |

Source of the texts:

- `rfc5213.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc5213.txt) —
  <https://www.rfc-editor.org/rfc/rfc5213.txt>, downloaded 2026-09-23.
- `rfc4283.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc4283.txt) —
  <https://www.rfc-editor.org/rfc/rfc4283.txt>, downloaded 2026-09-23.
- `rfc6275.txt` — same file as [`standards/RFC/`](../../../../../../standards/RFC/rfc6275.txt), downloaded by
  the Mobile IPv6 pass; not downloaded again.

RFC 5213 is a Proposed Standard with no `obsoletes` and no `obsoleted by` in the register: it is
the current, sole base document, with no predecessor for a claim to fall behind.

**RFC 5213 depends on RFC 6275, under its old number.** RFC 5213 predates RFC 6275 by three
years and cites the Mobility Header base document throughout as "[RFC3775]" — 58 lines, for
example `rfc5213.txt:920,1090,1094,2850,2868`. Two of those citations point at specific sections
this pass checked directly against the RFC 6275 text: `rfc5213.txt:1090` ("the local
mobility anchor MUST observe the rules described in Section 9.2 of [RFC3775] when processing the
Mobility Header") matches RFC 6275 §9.2 ("Processing Mobility Headers", `rfc6275.txt:4319`), and
`rfc5213.txt:1094` ("the local mobility anchor MUST ignore the check... related to the presence
of the Home Address destination option") matches RFC 6275 §10.3.1 ("Primary Care-of Address
Registration", `rfc6275.txt:5026`), which does state that check (`rfc6275.txt:5074`, "A Home
Address destination option MUST be present in the message"). RFC 6275 keeps RFC 3775's section
numbering (see [`mipv6/standards.md`](../mipv6/standards.md#document-list)), so PMIPv6's
dependency stands on the RFC 6275 text without adjustment. A third rule, at
`rfc5213.txt:2868` ("the mobile access gateway MUST ignore any checks... related to the presence
of a Type 2 Routing header in the Proxy Binding Acknowledgement message"), points at RFC 6275
§6.4 ("Type 2 Routing Header", `rfc6275.txt:3064`).

Nearly every clause of RFC 5213 carries a keyword: "MUST" is on 310 lines, "SHOULD" on 40, "MAY"
on 35, in a document of 92 pages — a density close to RFC 6275's.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The Mobile Node Identifier option, for PMIPv6's own exchange | RFC 4283 §3, `rfc4283.txt:127-129`: "a new **optional** data field that is carried in the Mobile IPv6-defined messages" | RFC 5213 §5.3.6, `rfc5213.txt:1414`, and §6.9.1.5, `rfc5213.txt:3100`: "The Mobile Node Identifier option [RFC4283] **MUST** be present" in both the Proxy Binding Update and its Acknowledgement | RFC 5213 for PMIPv6: it elevates an optional MIPv6 option to a mandatory field of its own core exchange | yes, from level 2 |
| The link-local and link-layer address a Mobile Access Gateway uses on a shared access link | RFC 5213 §6.8, `rfc5213.txt:2607-2616`: the anchor generates the address, and "the specific method... is out of scope for this specification" | RFC 6543: reserves a specific address for this purpose, "updates RFC 5213" (register) | RFC 6543 for the exact address value | no; level 3 |

RFC 6275's own `updated_by` field is empty in the register (see
[`mipv6/standards.md`](../mipv6/standards.md#override-table)), so the two checks RFC 5213 tells
its roles to skip (the Home Address option check of §10.3.1 and the Type 2 Routing Header check
of §6.4) are not themselves overridden by a further document; RFC 5213 only instructs PMIPv6 not
to apply them, which is a difference in which role runs which check, not a clause-level conflict.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 5213 | August 2008, Proposed Standard; §5, §6, §7, §8 | none yet; level 2 writes `standard/rfc5213/catalog.md` |
| RFC 4283 | December 2005, Proposed Standard; §3, as made mandatory by RFC 5213 §5.3.6 and §6.9.1.5 | none yet; level 2 writes `standard/rfc4283/catalog.md` |

RFC 6275 enters no new catalog: PMIPv6 reuses the Mobility Header format and the two processing
rules named above without adding a statement of its own to `standard/rfc6275/catalog.md`, which
the Mobile IPv6 level 2 pass writes.

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 6543 | Level 3. A specific address value, checkable only once the core Proxy Binding Update/Acknowledgement exchange of level 2 is in place. |
| RFC 7864 | Level 5. Flow mobility over multiple physical interfaces is an extension to the single-interface handover this pass targets; the register lists the relation as `updates` RFC 5213, not `obsoletes`. |
| RFC 5844 | Level 5. IPv4 transport and IPv4 home-address support are a separate address family; PMIPv6's normal exchange, in scope here, is IPv6-only. |
| RFC 5213 §5.4, §5.7, §5.9 | Level 5. Multihoming, anchor address discovery, and route optimization are areas RFC 5213 itself sets apart from the single-anchor handover of §5.3 and §6.9. |
