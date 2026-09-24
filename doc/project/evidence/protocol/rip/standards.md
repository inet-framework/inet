# RIP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around the Routing Information Protocol, records which document governs each contested clause,
and pins the set that the next pass tests against. RIP has two base documents: RIP version 2
for IPv4, and RIPng for IPv6. They describe the same protocol for two address families, in
parallel sections.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 2453 §3 and §4.2 to §4.6; RFC 2080 §2 | the normal exchange: a request and its response, the periodic and the triggered update, the metric rule, split horizon with poisoned reverse, the message formats |
| level 3, Edge | nothing new | RFC 2453 §3.9.2 and RFC 2080 §2.4.2 hold the validity checks of a response. A crafted response exercises them: a wrong source port, a source that is not a neighbor, a metric out of range, an own address |
| level 4, Dynamics | nothing new | RFC 2453 §3.8 and §3.10.1 and RFC 2080 §2.3 and §2.5.1 hold the timers: the update timer and its random offset, the timeout, the garbage-collection time, and the rate limit of triggered updates |
| level 5, Complete | RFC 4822, RFC 2453 §4.1, §5 and §6, RFC 2091 | authentication, the compatibility switch and the interaction with RIP version 1, and demand circuits |

What a pass actually reached is not recorded here. It is in
[`model/rip/coverage.md`](../../model/rip/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 2453 | RIP Version 2 | November 1998 | Internet Standard (STD 56) | `base`, IPv4 | [`standards/RFC/rfc2453.txt`](../../../../../../standards/RFC/rfc2453.txt), 2026-09-23 |
| RFC 2080 | RIPng for IPv6 | January 1997 | Proposed Standard | `base`, IPv6 | [`standards/RFC/rfc2080.txt`](../../../../../../standards/RFC/rfc2080.txt), 2026-09-23 |
| RFC 1058 | Routing Information Protocol | June 1988 | Historic | the RIP version 1 base; RFC 2453 restates its algorithm and its message format, and RFC 2080 §1.1 cites it for the theory | no |
| RFC 1723 | RIP Version 2 — Carrying Additional Information | November 1994 | Draft Standard | `obsoleted by` RFC 2453 | no |
| RFC 4822 | RIPv2 Cryptographic Authentication | March 2007 | Proposed Standard | `updates` RFC 2453 | no |
| RFC 2091 | Triggered Extensions to RIP to Support Demand Circuits | January 1997 | Proposed Standard | `companion` | no |

Source of the texts:

- `rfc2453.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2453.txt) —
  <https://www.rfc-editor.org/rfc/rfc2453.txt>, downloaded 2026-09-23.
- `rfc2080.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2080.txt) —
  <https://www.rfc-editor.org/rfc/rfc2080.txt>, downloaded 2026-09-23.

RFC 2080 is older than RFC 2453. It cites RFC 1058 and RFC 1723, not RFC 2453, and nothing
updates it. The two documents state the same mechanisms in parallel sections:

| Mechanism | RFC 2453 | RFC 2080 |
| --- | --- | --- |
| message format | §3.6, §4 | §2.1, §2.1.1 |
| addressing | §3.7 | §2.2 |
| timers | §3.8 | §2.3 |
| input processing | §3.9 | §2.4 |
| output processing, triggered updates | §3.10 | §2.5 |
| split horizon | §3.4.3 | §2.6 |

Keep one catalog for each document. The feature map of level 2 joins the two, feature by
feature.

Neither document uses the keywords of RFC 2119. In RFC 2453, "must" is on 50 lines and
"should" on 56; in RFC 2080, each is on 25 lines; all are in lower case. The catalogs record the words as
written, and many statements have the strength `description`.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Authentication of a RIPv2 message | RFC 2453 §4.1, `rfc2453.txt:1716-1757`: the first entry carries the authentication, and "the only Authentication Type is simple password and it is type 2" | RFC 4822: keyed MD5 and the SHA family for the same entry; it "obsoletes RFC 2082 and updates RFC 2453" | RFC 4822 for cryptographic authentication; RFC 2453 §4.1 for the layout of the entry | no; level 5 |
| The algorithm behind RIPng | RFC 2080 §1.1, `rfc2080.txt:125-129`: the theory is in reference [1], RFC 1058 (`rfc2080.txt:967-968`), and "has not been incorporated in this document" | RFC 2453 §3.4, `rfc2453.txt:306`: the current RIP document restates the distance vector algorithm of RFC 1058 in full | RFC 2080 §2 for every RIPng procedure; the theory is background and no statement rests on it | yes, as background |

RFC 2453 obsoletes RFC 1723 as a whole, and it carries the RIP version 1 text of RFC 1058 in
its §3. No clause-level conflict exists between the two base documents, because they govern
different address families.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 2453 | November 1998, Internet Standard (STD 56); §3, and §4.2 to §4.6 | none yet; level 2 writes `standard/rfc2453/catalog.md` |
| RFC 2080 | January 1997, Proposed Standard; §2 | none yet; level 2 writes `standard/rfc2080/catalog.md` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 1058 | Historic. RFC 2453 carries its algorithm and its message format, so the current text is in scope through RFC 2453. |
| RFC 1723 | Obsoleted by RFC 2453. |
| RFC 4822 | Level 5. Authentication is an option of the protocol, and the normal exchange does not use it. |
| RFC 2453 §4.1 | Level 5, with RFC 4822: the simple-password entry. |
| RFC 2453 §5 and §6 | Level 5. The compatibility switch and the interaction with a RIP version 1 router need a second protocol version in the network. |
| RFC 2091 | Level 5. Demand circuits are a separate mode of operation for links that charge by the connection. |
