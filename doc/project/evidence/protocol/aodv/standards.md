# AODV — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around the Ad hoc On-Demand Distance Vector routing protocol, records which document governs
each contested clause, and pins the set that the next pass tests against. AODV has one base
document and one companion that neither updates nor obsoletes it.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 3561 §5 (message formats), §6.1, §6.3 to §6.7, §6.11 | the normal reactive exchange: a node originates a route request, an intermediate or the destination node generates a route reply, sequence numbers stay fresh, and a broken link produces a route error |
| level 3, Edge | nothing new | §6.4 and §6.5 hold the request-validation and duplicate-suppression rules; §6.8 governs operation over a link that works in one direction only, which is a fault case on top of the normal exchange |
| level 4, Dynamics | nothing new | §6.9 and §6.10 hold the periodic hello message and the local-connectivity check it supports; §6.3 holds the expanding-ring search backoff; RFC 5148 governs the jitter the standard applies to periodic and forwarded messages |
| level 5, Complete | RFC 3561 §6.12, §6.13, §6.14, §7, §8 | local repair is a path the standard itself marks optional; actions after reboot is a full restart scenario; multiple interfaces, subnet aggregation and interaction with a wired network are each a separate mode of operation |

What a pass actually reached is not recorded here. It is in
[`model/aodv/coverage.md`](../../model/aodv/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 3561 | Ad hoc On-Demand Distance Vector (AODV) Routing | July 2003 | Experimental | `base` | [`standards/RFC/rfc3561.txt`](../../../../../../standards/RFC/rfc3561.txt), 2026-09-23 |
| RFC 5148 | Jitter Considerations in Mobile Ad Hoc Networks (MANETs) | February 2008 | Informational | `companion` (jitter) | [`standards/RFC/rfc5148.txt`](../../../../../../standards/RFC/rfc5148.txt), 2026-09-23 |

Source of the texts:

- `rfc3561.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3561.txt) —
  <https://www.rfc-editor.org/rfc/rfc3561.txt>, downloaded 2026-09-23.
- `rfc5148.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc5148.txt) —
  <https://www.rfc-editor.org/rfc/rfc5148.txt>, downloaded 2026-09-23.

Neither document has an `obsoletes`, `obsoleted_by`, `updates` or `updated_by` entry in the
register. AODV is the one protocol of this pass whose family carries no later document at
all: what RFC 3561 says in 2003 is still the whole of the base text today.

**Both documents carry a status below Internet Standard, and that bounds what their keywords
mean.** RFC 3561 has the status Experimental. The document itself says an Experimental
specification "does not specify an Internet standard of any kind". A `must` or `should` inside
it states what an implementation needs to do to interoperate with another AODV
implementation. It does not carry the weight of an IETF-wide requirement the way a `must` in
an Internet Standard does. RFC 5148 has the status Informational. It is not on the standards
track at all; its `should` and `may` words state advice for a protocol designer, not a
requirement on a conforming implementation. A later pass keeps this distinction when it turns
a clause into a mandatory or optional feature.

Both documents use the keywords of RFC 2119, in upper case
(`rfc3561.txt:1914`, `rfc5148.txt:134` and `rfc5148.txt:516`). In RFC 3561, `MUST` is on 32
lines, `SHOULD` on 34, `MAY` on 14, and `SHALL` does not appear. In RFC 5148, `MUST` is on 4
lines, `SHOULD` on 18, `MAY` on 9, and `SHALL` on 1.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Jitter on periodic and forwarded messages | RFC 3561 defines the periodic Hello message (§6.9) and the forwarding of a request or a reply, but the word "jitter" does not appear anywhere in the document | RFC 5148 §5, `rfc5148.txt:231-472`: a general jitter mechanism for a periodic message (§5.1), an externally triggered message (§5.2) and a forwarded message (§5.3), with the bound on the jitter value in §5.4, `rfc5148.txt:435-445`: it "MUST NOT be negative", "MUST NOT be greater than MESSAGE_INTERVAL/2", and "SHOULD NOT be greater than MESSAGE_INTERVAL/4" | RFC 5148 for the jitter mechanism; RFC 3561 for which AODV messages it applies to | yes, as background; the mechanism itself is level 4 |

This is not a conflict between the two documents; RFC 5148 fills a gap RFC 3561 leaves open.
No clause of RFC 5148 contradicts a clause of RFC 3561.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 3561 | July 2003, Experimental; §5, §6.1, §6.3 to §6.7, §6.11 | none yet; level 2 writes `standard/rfc3561/catalog.md` |
| RFC 5148 | February 2008, Informational; §5 | none yet; level 2 writes `standard/rfc5148/catalog.md` |

Out of scope, with the reason:

| Document or clause | Reason |
| --- | --- |
| RFC 3561 §6.8 | Level 3. Operation over a link that works in one direction only is a fault case on top of the normal, bidirectional exchange. |
| RFC 3561 §6.9, §6.10 | Level 4. The Hello message interval and the local-connectivity check it supports are timers, not part of a single reactive exchange. |
| RFC 3561 §6.3, the expanding-ring search backoff | Level 4. The retry schedule of a route request is a timer series, not a single value. |
| RFC 3561 §6.12 | Level 5. The standard itself marks local repair `MAY`: "the node upstream of that break MAY choose to repair the link locally" (`rfc3561.txt:1409-1410`). |
| RFC 3561 §6.13 | Level 5. Actions after reboot is a full node-restart scenario, and needs a richer mockup than a normal exchange. |
| RFC 3561 §6.14 | Level 5. Interfaces describes a node with more than one interface into the ad hoc network; the normal-path exchange of level 2 needs only one. |
| RFC 3561 §7 | Level 5. Aggregated networks is a mode where several nodes share one subnet prefix and one of them advertises for the rest; it is not the base exchange. |
| RFC 3561 §8 | Level 5. Using AODV with other networks describes a node that also reaches a wired network through a gateway; it is a second network attached to the ad hoc one, not the base exchange. |
| RFC 3561 §9 to §13 | Out of scope at every level except as background. Extensions (§9), configuration-parameter defaults (§10), security (§11), IANA (§12) and IPv6 considerations (§13) state no additional wire behavior of the normal exchange. |
