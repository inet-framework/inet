# IPv6 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around IPv6, records which document governs each contested clause, and pins the exact set
that the current pass tests against. Every later artifact of this folder is bound to the
in-scope set below.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNNN.json`),
retrieved 2026-09-09. The relationship words are the ones of the workflow: `base`,
`updates`, `obsoletes`, `amends`, `companion`.

## Target level

**Level 2 — Core** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
The in-scope set below holds exactly the documents that level 1 and level 2 need: the base
document and the companion that carries its error reports.

The level decides which documents the next pass must add:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 3, Edge | RFC 8504 (node requirements), RFC 4443 §2.4 in full, RFC 8200 §4 in full | the node requirements are the RFC 1122 of IPv6; the message processing rules and the extension header processing are where the negative requirements and the crafted-input rules live |
| level 4, Dynamics | RFC 8201 (path MTU discovery), the reassembly timer of RFC 8200 §4.5, the rate limit of RFC 4443 §2.4 (f) | control loops and timers |
| level 5, Complete | RFC 4291, RFC 4861, RFC 4862, RFC 6437, RFC 2474, RFC 3168, RFC 2675, RFC 9673, RFC 4884 | addressing, neighbor discovery, the flow label, the traffic class, jumbograms, the hop-by-hop procedures, extended ICMP |

What the pass actually reached is not recorded here. It is in
[`model/ipv6/coverage.md`](../../model/ipv6/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 8200 | Internet Protocol, Version 6 (IPv6) Specification | July 2017 | Internet Standard (STD 86) | `base`; `obsoletes` RFC 2460 | [`standard/rfc8200/`](../../standard/rfc8200/rfc8200.txt), 2026-09-09 |
| RFC 4443 | Internet Control Message Protocol (ICMPv6) for the IPv6 Specification | March 2006 | Internet Standard (STD 89) | `companion` of RFC 8200 (error reports); `obsoletes` RFC 2463 | [`standard/rfc4443/`](../../standard/rfc4443/rfc4443.txt), 2026-09-09 |
| RFC 2460 | Internet Protocol, Version 6 (IPv6) Specification | December 1998 | Draft Standard, obsoleted | obsoleted by RFC 8200 | no |
| RFC 2463 | ICMPv6 for the IPv6 Specification | December 1998 | Draft Standard, obsoleted | obsoleted by RFC 4443 | no |
| RFC 9673 | IPv6 Hop-by-Hop Options Processing Procedures | October 2024 | Proposed Standard | `updates` RFC 8200 | no |
| RFC 4884 | Extended ICMP to Support Multi-Part Messages | May 2007 | Proposed Standard | `updates` RFC 4443 | no |
| RFC 8201 | Path MTU Discovery for IP version 6 | July 2017 | Internet Standard | `companion` (RFC 8200 §5 strongly recommends it) | no |
| RFC 8504 | IPv6 Node Requirements | January 2019 | Best Current Practice | `companion` (the node requirements; the RFC 1122 of IPv6) | no |
| RFC 4291 | IP Version 6 Addressing Architecture | February 2006 | Draft Standard | `companion` (RFC 8200 §3 delegates the address formats to it) | no |
| RFC 4861 | Neighbor Discovery for IP version 6 | September 2007 | Draft Standard | `companion` (address resolution on a link; the redirect message) | no |
| RFC 4862 | IPv6 Stateless Address Autoconfiguration | September 2007 | Draft Standard | `companion` (address configuration; duplicate address detection) | no |
| RFC 6437 | IPv6 Flow Label Specification | November 2011 | Proposed Standard | `companion` (RFC 8200 §6 delegates the flow label to it) | no |
| RFC 2474, RFC 3168 | Differentiated Services Field; Explicit Congestion Notification | 1998, 2001 | Proposed Standard | `companion` (RFC 8200 §7 delegates the traffic class to them) | no |
| RFC 2675 | IPv6 Jumbograms | August 1999 | Proposed Standard | `companion` (payload length zero with a jumbo payload option) | no |
| RFC 7739 | Security Implications of Predictable Fragment Identification Values | February 2016 | Informational | `companion` (identification algorithms; cited by RFC 8200 §4.5) | no |
| RFC 6936 | Applicability Statement for IPv6 UDP Datagrams with Zero Checksums | May 2013 | Proposed Standard | `companion` (the exception to the mandatory UDP checksum) | no |

Sources of the two cached texts. Each one lives in the folder of its own document, beside
the catalog of that document, and every quote in this tree cites it by file name and line
number:

- `rfc8200.txt` in [`evidence/standard/rfc8200/`](../../standard/rfc8200) —
  <https://www.rfc-editor.org/rfc/rfc8200.txt>, downloaded 2026-09-09.
- `rfc4443.txt` in [`evidence/standard/rfc4443/`](../../standard/rfc4443) —
  <https://www.rfc-editor.org/rfc/rfc4443.txt>, downloaded 2026-09-09.

Three relationship notes, because the register alone gives the wrong picture:

- **RFC 8200 replaced RFC 2460 in 2017**, and RFC 2460 had nine updates folded into it
  (among them RFC 5722 on overlapping fragments, RFC 6946 on atomic fragments, and RFC 7112
  on oversized header chains). An implementation that cites RFC 2460 cites a text that no
  longer governs; the hop limit rule for a destination node and the reassembly rules are
  among the clauses that changed. This is the first thing the conformance document has to
  look for.
- **RFC 4443 is the ICMP of IPv6, and it is not optional.** Where RFC 792 says a gateway
  "may" report, RFC 4443 says a router "MUST" send Packet Too Big and Time Exceeded. The
  error reports of the base mechanisms are therefore mandatory features in IPv6, unlike
  IPv4.
- **Neighbor Discovery (RFC 4861) and address autoconfiguration (RFC 4862) are out of the
  set but present in every scenario.** They are the ARP of IPv6: address resolution,
  router discovery and duplicate address detection run before the first datagram can
  leave a host. The checks tolerate their messages on the wire and do not judge them.

## Override table

One row per clause-level conflict. `Governs` names the document a test must follow when the
two texts disagree. The RFC 8200 and RFC 4443 references are line numbers of the cached
files; the other references are clause numbers, because those texts are not cached.

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Hop-by-hop options processing by nodes en route | RFC 8200 §4, `rfc8200.txt:430-441` (nodes examine the header only if configured to) | RFC 9673 §5 (processing procedures for hosts and routers) | RFC 9673 | no |
| Extended ICMP error messages | RFC 4443 §2.4 (c), `rfc4443.txt:295-298` (as much of the invoking packet as fits) | RFC 4884 §4 (a length field and extension structure) | RFC 4884 | no |
| Fragment identification algorithm | RFC 8200 §4.5, `rfc8200.txt:864-879` (low reuse frequency) | RFC 7739 §5 (recommended algorithms; informational) | RFC 8200 (RFC 7739 is guidance) | no |
| UDP zero checksum | RFC 8200 §8.1, `rfc8200.txt:1530-1537` (a zero checksum is discarded) | RFC 6936 (an exception for tunnel encapsulations, per port) | RFC 8200, with the RFC 6936 exception | no |

No row is in scope in the current pass, so no catalog entry carries an `Overridden by`
field yet.

## In-scope set

The current pass tests against these exact documents:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 8200 | July 2017, Internet Standard, no revision since (RFC 9673 updates one clause of §4, out of scope) | [`rfc8200/catalog.md`](../../standard/rfc8200/catalog.md) |
| RFC 4443 | March 2006, Internet Standard, no revision since (RFC 4884 extends the error message format, out of scope) | [`rfc4443/catalog.md`](../../standard/rfc4443/catalog.md) |

Both documents revise by replacement, never in place, so the document identity pins the
version by itself.

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 8504 | The node requirements: the strengths, the prohibitions, and the crafted-input rules for a host. The largest and most valuable next step; level 3. |
| RFC 8201 | Path MTU discovery is a control loop driven by Packet Too Big; level 4. This pass checks that the router sends the message, not what the source does with it. |
| RFC 4291, RFC 4861, RFC 4862 | Addressing, neighbor discovery and autoconfiguration are protocols of their own. The mockup needs them to function; the checks do not judge them. |
| RFC 6437, RFC 2474, RFC 3168 | The flow label and the traffic class byte are out of scope in the catalog itself. |
| RFC 2675, RFC 9673, RFC 4884 | Jumbograms, the hop-by-hop procedures and extended ICMP change areas the catalog does not cover. |
| RFC 2460, RFC 2463 | Obsoleted. Named here because the model documentation cites them; see the conformance document. |
| RFC 7739, RFC 6936 | Guidance and an exception; neither adds a requirement to the level 2 set. |
