# DiffServ — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around Differentiated Services, records which document governs each contested clause, and
pins the set that the next pass tests against. DiffServ has no protocol data unit of its own.
Its mechanisms — classification, metering, marking, queueing — act inside one node on the
Differentiated Services (DS) field of the IP header that RFC 2474 defines; nothing is
exchanged between DiffServ peers. Because of this, most statements of the family are
algorithm descriptions rather than wire rules, and the "only path to an outcome" rule of the
guide, not an RFC 2119 keyword, will decide most feature levels once a catalog exists.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 2474 §3, §4.2.2.1; RFC 2475 §2.3.1 to §2.3.3; RFC 2597 §2, §4, §6; RFC 3246 §2; RFC 2697 §2 to §4; RFC 2698 §2 to §4; RFC 3260 §4, §7 | the normal path: the DS field and the Class Selector codepoints, what a classifier, a meter, a marker and a traffic conditioner do, the Assured Forwarding and Expedited Forwarding PHBs and their recommended codepoints, the srTCM and trTCM algorithms, and RFC 3260's correction that RFC 2598 is obsolete |
| level 3, Edge | nothing new | RFC 2475 §2.3.2 (in-profile and out-of-profile traffic) and RFC 3260 §6 (an unknown or improperly mapped DSCP) already hold the edge cases inside the documents above |
| level 4, Dynamics | nothing new | the srTCM and trTCM token buckets of RFC 2697 §3 and RFC 2698 §3 are distributions over time, already inside the base documents above; a level 4 pass adds the tolerance, not a new document |
| level 5, Complete | RFC 3290, RFC 4115 | the informal management model that the classifiers reference for configuration, and a second, alternative two-rate marker the model does not implement |

What a pass actually reached is not recorded here. It is in
[`model/diffserv/coverage.md`](../../model/diffserv/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 2474 | Definition of the Differentiated Services Field (DS Field) in the IPv4 and IPv6 Headers | December 1998 | Proposed Standard | `base`, the DS field | [`standards/RFC/rfc2474.txt`](../../../../../../standards/RFC/rfc2474.txt), 2026-09-23 |
| RFC 2475 | An Architecture for Differentiated Services | December 1998 | Informational | `base`, the architecture | [`standards/RFC/rfc2475.txt`](../../../../../../standards/RFC/rfc2475.txt), 2026-09-23 |
| RFC 2597 | Assured Forwarding PHB Group | June 1999 | Proposed Standard | `base`, the AF PHB | [`standards/RFC/rfc2597.txt`](../../../../../../standards/RFC/rfc2597.txt), 2026-09-23 |
| RFC 3246 | An Expedited Forwarding PHB (Per-Hop Behavior) | March 2002 | Proposed Standard | `base`, the EF PHB; `obsoletes` RFC 2598 | [`standards/RFC/rfc3246.txt`](../../../../../../standards/RFC/rfc3246.txt), 2026-09-23 |
| RFC 2697 | A Single Rate Three Color Marker | September 1999 | Informational | `base`, srTCM | [`standards/RFC/rfc2697.txt`](../../../../../../standards/RFC/rfc2697.txt), 2026-09-23 |
| RFC 2698 | A Two Rate Three Color Marker | September 1999 | Informational | `base`, trTCM | [`standards/RFC/rfc2698.txt`](../../../../../../standards/RFC/rfc2698.txt), 2026-09-23 |
| RFC 3260 | New Terminology and Clarifications for Diffserv | April 2002 | Informational | `updates` RFC 2474, RFC 2475, RFC 2597 | [`standards/RFC/rfc3260.txt`](../../../../../../standards/RFC/rfc3260.txt), 2026-09-23 |
| RFC 1349 | Type of Service in the Internet Protocol Suite | July 1992 | Proposed Standard | `obsoleted by` RFC 2474 | no |
| RFC 2598 | An Expedited Forwarding PHB | June 1999 | Proposed Standard | `obsoleted by` RFC 3246 | no |
| RFC 3168 | The Addition of Explicit Congestion Notification (ECN) to IP | September 2001 | Proposed Standard | `updates` RFC 2474 | no |
| RFC 8436 | Update to IANA Registration Procedures for Pool 3 Values in the DSCP Registry | August 2018 | Proposed Standard | `updates` RFC 2474 | no |
| RFC 3290 | An Informal Management Model for Diffserv Routers | June 2002 | Informational | `companion` | no |
| RFC 4115 | A Differentiated Service Two-Rate, Three-Color Marker with Efficient Handling of in-Profile Traffic | July 2005 | Informational | `companion`, an alternative to RFC 2698 | no |

Source of the texts:

- `rfc2474.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2474.txt) —
  <https://www.rfc-editor.org/rfc/rfc2474.txt>, downloaded 2026-09-23.
- `rfc2475.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2475.txt) —
  <https://www.rfc-editor.org/rfc/rfc2475.txt>, downloaded 2026-09-23.
- `rfc2597.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2597.txt) —
  <https://www.rfc-editor.org/rfc/rfc2597.txt>, downloaded 2026-09-23.
- `rfc3246.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3246.txt) —
  <https://www.rfc-editor.org/rfc/rfc3246.txt>, downloaded 2026-09-23.
- `rfc2697.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2697.txt) —
  <https://www.rfc-editor.org/rfc/rfc2697.txt>, downloaded 2026-09-23.
- `rfc2698.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2698.txt) —
  <https://www.rfc-editor.org/rfc/rfc2698.txt>, downloaded 2026-09-23.
- `rfc3260.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3260.txt) —
  <https://www.rfc-editor.org/rfc/rfc3260.txt>, downloaded 2026-09-23.

RFC 2474 obsoletes RFC 1349 (and RFC 1455) as a whole; no predecessor text is downloaded. RFC
3246 obsoletes RFC 2598 as a whole; RFC 3260 itself records the reason
(`rfc3260.txt:43-45`): "RFC 2598 has been obsoleted by RFC 3246, and clarifications agreed
by the group were incorporated in that revision." Neither obsoleted document is downloaded.

Keyword use is mixed across the family. RFC 2474 has "MUST" on 21 lines, "SHOULD" on 11 and
"MAY" on 15; RFC 2597 has 20, 6 and 7; RFC 3246 has 11, 8 and 10; RFC 3260 has 4, 3 and 0
(`grep -c '\bMUST\b'` and so on). RFC 2475, RFC 2697 and RFC 2698 use none of the three
words at all: the architecture document and the two meter algorithms are pure description.
This matches the note above — DiffServ statements lean on the "only path" rule more than on
RFC 2119 keywords, and the three zero-keyword documents will lean on it entirely.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The two least significant bits of the DS octet | RFC 2474 §3, `rfc2474.txt:372-374`: "A two-bit currently unused (CU) field is reserved and its definition and interpretation are outside the scope of this document" | RFC 3168 (`rfc3260.txt:176-179`, quoting the relation): "the 'Currently Unused' (CU) bits of the DS Field have not been assigned to Diffserv, and subsequent to the publication of RFC 2474, they were assigned for explicit congestion notification, as defined in RFC 3168" | RFC 3168, for the two bits; RFC 2474 for the six-bit DSCP, unaffected | no; the DSCP field DiffServ mechanisms read and write is the six-bit field of RFC 2474 §3, `rfc2474.txt:365-390`; the CU/ECN bits are a distinct mechanism (RFC 3168), out of scope of this protocol |
| The Expedited Forwarding PHB | RFC 2598, June 1999: the original EF PHB definition and its recommended codepoint | RFC 3246, March 2002, `rfc3246.txt:1-16` (title page: "obsoletes RFC 2598"): a full restatement of the EF PHB, incorporating the RFC 3260 clarifications | RFC 3246 | yes; RFC 2598 is obsolete and out of scope |
| Registration procedure for DSCP pool 3 | RFC 2474 §6, IANA Considerations | RFC 8436: changes the IANA registration procedure for one pool of codepoint values | RFC 8436 | no; an administrative change to the registry procedure, not a rule a DiffServ node follows |
| Terminology of the DS field, the PHB group, and unknown codepoints | RFC 2474 §3, RFC 2475 §2.3, RFC 2597 §2 | RFC 3260 §4 to §7, `rfc3260.txt:159-244`: renames and clarifies terms, and states the handling of an unknown or improperly mapped DSCP | RFC 3260 | yes |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 2474 | December 1998, Proposed Standard; §3, §4.2.2.1, as clarified by RFC 3260 §4 | none yet; level 2 writes `standard/rfc2474/catalog.md` |
| RFC 2475 | December 1998, Informational; §2.3.1 to §2.3.3, as clarified by RFC 3260 | none yet; level 2 writes `standard/rfc2475/catalog.md` |
| RFC 2597 | June 1999, Proposed Standard; §2, §4, §6, as clarified by RFC 3260 | none yet; level 2 writes `standard/rfc2597/catalog.md` |
| RFC 3246 | March 2002, Proposed Standard; §2 (supersedes RFC 2598 in full) | none yet; level 2 writes `standard/rfc3246/catalog.md` |
| RFC 2697 | September 1999, Informational; §2 to §4 | none yet; level 2 writes `standard/rfc2697/catalog.md` |
| RFC 2698 | September 1999, Informational; §2 to §4 | none yet; level 2 writes `standard/rfc2698/catalog.md` |
| RFC 3260 | April 2002, Informational; §4 to §7 | none yet; level 2 writes `standard/rfc3260/catalog.md` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 1349 | Obsoleted by RFC 2474. |
| RFC 2598 | Obsoleted by RFC 3246. |
| RFC 3168 | A separate mechanism (ECN). Governs the two DS-octet bits DiffServ mechanisms do not read or write; see the override table. |
| RFC 8436 | Administrative. An IANA registration procedure, not a node behavior. |
| RFC 3290 | Level 5. An informal management and configuration model for a DiffServ router's policy database, not a per-hop-behavior or DS-field rule. |
| RFC 4115 | Level 5. An alternative two-rate, three-color marker to RFC 2698, with a different in-profile handling rule; a companion algorithm, not a revision of RFC 2697 or RFC 2698. |
