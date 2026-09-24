# Mobile IPv6 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around Mobile IPv6, records which document governs, and pins the set that a level 2 pass tests
against.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | nothing new; RFC 6275 §6 (Mobility Header format and options, `rfc6275.txt:1864`), §6.3 to §6.4 (Home Address option, Type 2 Routing Header, `rfc6275.txt:2975`, `rfc6275.txt:3064`), §9.5.1 and §10.3 (Binding Update processing at a correspondent node and a home agent, `rfc6275.txt:4628`, `rfc6275.txt:5024`), and §11.6 to §11.7 (return routability and sending a Binding Update, mobile node side, `rfc6275.txt:7231`, `rfc6275.txt:7375`) | the normal exchange: home registration, the return-routability procedure, route optimization, and the two route-optimized-traffic mechanisms |
| level 3, Edge | nothing new | §9.5.1 lists the Binding Update validity checks (a wrong sequence number, a failed authorization); §9.3.3 and §9.4 hold the Binding Error and return-routability rejection cases |
| level 4, Dynamics | nothing new | §5.2 holds the nonce and key lifetimes; §11.8 holds the retransmission and rate-limit rules; the binding lifetime fields of §6.1.7, §9.5 and §11.7 need a tolerance |
| level 5, Complete | RFC 4877, if a later pass adds IPsec-protected signaling | §10.5 and §11.4.1 (Dynamic Home Agent Address Discovery), §11.4.2 to §11.4.3 (Mobile Prefix Discovery), the full mobility-option set of §6.2, and IPsec protection of Binding Updates (§5, §11.3.2) |

What a pass actually reached is not recorded here. It is in
[`model/mipv6/coverage.md`](../../model/mipv6/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 6275 | Mobility Support in IPv6 | July 2011 | Proposed Standard | `base`, obsoletes RFC 3775 | [`standards/RFC/rfc6275.txt`](../../../../../../standards/RFC/rfc6275.txt), 2026-09-23 |
| RFC 3775 | Mobility Support in IPv6 | June 2004 | Proposed Standard | `obsoleted by` RFC 6275 | no |
| RFC 4877 | Mobile IPv6 Operation with IKEv2 and the Revised IPsec Architecture | April 2007 | Proposed Standard | `updates` RFC 3776, a document outside this family | no |
| RFC 5095 | Deprecation of Type 0 Routing Headers in IPv6 | December 2007 | Proposed Standard | `updates` RFC 2460 and RFC 4294, both outside this family | no |
| RFC 4283 | Mobile Node Identifier Option for Mobile IPv6 (MIPv6) | December 2005 | Proposed Standard | `companion`; no formal register relation to RFC 6275 | no |

Source of the text:

- `rfc6275.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc6275.txt) —
  <https://www.rfc-editor.org/rfc/rfc6275.txt>, downloaded 2026-09-23. This copy is shared with
  the Proxy Mobile IPv6 pass; see [`pmipv6/standards.md`](../pmipv6/standards.md).

RFC 6275 obsoletes RFC 3775 as a whole, not clause by clause: `rfc6275.txt:413-415` says
"This document obsoletes RFC 3775. Issues with the original document have been observed during
the integration, testing, and deployment of RFC 3775." Appendix B (`rfc6275.txt:9312-9467`) lists
the changes issue by issue, and the section numbering carries over unchanged — checked directly:
§9.2 ("Processing Mobility Headers") and §10.3.1 ("Primary Care-of Address Registration") hold at
the same section numbers in both documents, confirmed against the two clauses that
[`pmipv6/standards.md`](../pmipv6/standards.md) needs, which RFC 5213 still cites under the
old RFC 3775 numbers.

RFC 6275 uses the keywords of RFC 2119 (`rfc6275.txt:475-476`) throughout: "MUST" is on 433
lines, "SHOULD" on 154, "MAY" on 77, in a document of 169 pages. Nearly every normative
statement carries a keyword, unlike the older, keyword-poor documents of the other two protocols
of this wave.

## Override table

No entries. RFC 6275 is the sole base document of the family, and the register lists no
`updated_by` document for it, so no later document overrides one of its clauses within the scope
of this pass. RFC 4877, RFC 5095 and RFC 4283 are all out of scope (see below), and none of them
updates RFC 6275 in the register either — RFC 4877 updates RFC 3776, RFC 5095 updates RFC 2460
and RFC 4294, and RFC 4283 updates nothing.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 6275 | July 2011, Proposed Standard | none yet; level 2 writes `standard/rfc6275/catalog.md` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 3775 | Obsoleted by RFC 6275. |
| RFC 4877 | No register relation to RFC 6275: it updates RFC 3776, a companion IPsec document of the RFC 3775 era, not RFC 6275 itself. IPsec protection of the signaling is an optional mechanism; level 5. |
| RFC 5095 | No register relation to RFC 6275: it updates RFC 2460 and RFC 4294, the generic IPv6 base and node-requirements documents, not RFC 6275. It deprecates the Type 0 Routing Header; RFC 6275 §6.4 defines a Type 2 Routing Header, a different value, which RFC 5095 does not mention. |
| RFC 4283 | No register relation to RFC 6275: `updates` and `updated_by` are both empty for RFC 4283 in the register, and RFC 6275's `updated_by` is also empty. It defines one mobility option, the Mobile Node Identifier; a companion at the option-catalog level (level 2 or 5, once the option catalog is built), not a change to the base exchange. |
