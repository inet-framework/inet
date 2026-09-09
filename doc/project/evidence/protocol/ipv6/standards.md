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

**Level 3 — Edge** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
Level 3 adds the negative requirements, the error reports, and the crafted or corrupted
input, and it brings in the document where those rules live for IPv6: the node
requirements. The in-scope set below therefore holds three documents, and the two documents
of level 2 are in scope in full where they were in scope in part.

The level decides which documents the next pass must add:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
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
| RFC 8504 | IPv6 Node Requirements | January 2019 | Best Current Practice (BCP 220); `obsoletes` RFC 6434 | `companion` (the node requirements; the RFC 1122 of IPv6) | [`standard/rfc8504/`](../../standard/rfc8504/rfc8504.txt), 2026-09-09 |
| RFC 4291 | IP Version 6 Addressing Architecture | February 2006 | Draft Standard | `companion` (RFC 8200 §3 delegates the address formats to it) | no |
| RFC 4861 | Neighbor Discovery for IP version 6 | September 2007 | Draft Standard | `companion` (address resolution on a link; the redirect message) | no |
| RFC 4862 | IPv6 Stateless Address Autoconfiguration | September 2007 | Draft Standard | `companion` (address configuration; duplicate address detection) | no |
| RFC 6437 | IPv6 Flow Label Specification | November 2011 | Proposed Standard | `companion` (RFC 8200 §6 delegates the flow label to it) | no |
| RFC 2474, RFC 3168 | Differentiated Services Field; Explicit Congestion Notification | 1998, 2001 | Proposed Standard | `companion` (RFC 8200 §7 delegates the traffic class to them) | no |
| RFC 2675 | IPv6 Jumbograms | August 1999 | Proposed Standard | `companion` (payload length zero with a jumbo payload option) | no |
| RFC 7739 | Security Implications of Predictable Fragment Identification Values | February 2016 | Informational | `companion` (identification algorithms; cited by RFC 8200 §4.5) | no |
| RFC 6936 | Applicability Statement for IPv6 UDP Datagrams with Zero Checksums | May 2013 | Proposed Standard | `companion` (the exception to the mandatory UDP checksum) | no |

Sources of the three cached texts. Each one lives in the folder of its own document, beside
the catalog of that document, and every quote in this tree cites it by file name and line
number:

- `rfc8200.txt` in [`evidence/standard/rfc8200/`](../../standard/rfc8200) —
  <https://www.rfc-editor.org/rfc/rfc8200.txt>, downloaded 2026-09-09.
- `rfc4443.txt` in [`evidence/standard/rfc4443/`](../../standard/rfc4443) —
  <https://www.rfc-editor.org/rfc/rfc4443.txt>, downloaded 2026-09-09.
- `rfc8504.txt` in [`evidence/standard/rfc8504/`](../../standard/rfc8504) —
  <https://www.rfc-editor.org/rfc/rfc8504.txt>, downloaded 2026-09-09.

Four relationship notes, because the register alone gives the wrong picture:

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
- **RFC 8504 is a Best Current Practice, not a standard, and it mostly points.** Most of
  its sentences say which document a node MUST or SHOULD implement; a few state a behavior
  of their own, and those sharpen RFC 8200 §4.5 (overlapping and atomic fragments, the
  header chain of a first fragment, unrecognized next headers). The catalog holds both
  kinds; the pointing kind gets the class `implementation`.
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
| Overlapping fragments | RFC 8200 §4.5, `rfc8200.txt:1050-1051` (must not create), `rfc8200.txt:1183-1185` (abandon reassembly) | RFC 8504 §5.1, `rfc8504.txt:327-331` (silently discard the entire datagram; RFC 5722) | RFC 8504 | **yes** |
| Atomic fragments | RFC 8200 §4.5, `rfc8200.txt:1133-1140` (a receiver processes one as a whole packet) | RFC 8504 §5.1, `rfc8504.txt:343-348` (a node MUST NOT generate one; RFC 8021) | RFC 8504 | **yes** |
| Unrecognized next header | RFC 8200 §4, `rfc8200.txt:465-473` (should discard, Parameter Problem code 1) | RFC 8504 §5.2, `rfc8504.txt:373-378` (MUST process as RFC 8200 says, for an upper-layer protocol too) | RFC 8504 | **yes** |
| The header chain of a first fragment | RFC 8200 §4.5, `rfc8200.txt:1018-1023` and `rfc8200.txt:1168-1171` | RFC 8504 §5.2, `rfc8504.txt:399-409` (the sender MUST include the whole chain) | RFC 8504 | **yes** |
| Hop-by-hop options processing by nodes en route | RFC 8200 §4, `rfc8200.txt:430-441` (nodes examine the header only if configured to) | RFC 9673 §5 (processing procedures for hosts and routers) | RFC 9673 | no |
| Extended ICMP error messages | RFC 4443 §2.4 (c), `rfc4443.txt:295-298` (as much of the invoking packet as fits) | RFC 4884 §4 (a length field and extension structure) | RFC 4884 | no |
| Fragment identification algorithm | RFC 8200 §4.5, `rfc8200.txt:864-879` (low reuse frequency) | RFC 7739 §5 (recommended algorithms; informational) | RFC 8200 (RFC 7739 is guidance) | no |
| UDP zero checksum | RFC 8200 §8.1, `rfc8200.txt:1530-1537` (a zero checksum is discarded) | RFC 6936 (an exception for tunnel encapsulations, per port) | RFC 8200, with the RFC 6936 exception | no |

Every in-scope row is an `Overridden by` cross reference in the RFC 8200 catalog. The other
rows become one on the day their document enters the in-scope set.

## In-scope set

The current pass tests against these exact documents:

| Document | Version | Sections in scope | Catalog file |
| --- | --- | --- | --- |
| RFC 8200 | July 2017, Internet Standard, no revision since (RFC 9673 updates one clause of §4, out of scope) | §3, §4 (the processing rules; the fragment header), §4.5, §5, §8.1; not the other extension headers and their options, the flow label, the traffic class | [`rfc8200/catalog.md`](../../standard/rfc8200/catalog.md) |
| RFC 4443 | March 2006, Internet Standard, no revision since (RFC 4884 extends the error message format, out of scope) | §2 (the general rules) and §3 (the error messages); not §4 (the informational messages) | [`rfc4443/catalog.md`](../../standard/rfc4443/catalog.md) |
| RFC 8504 | January 2019, Best Current Practice 220, no revision since | §5.1, §5.2, §5.7, §5.8: the IP layer; not §5.3 (limits a host may set), the neighbor discovery and addressing sections, and §6 to §17 | [`rfc8504/catalog.md`](../../standard/rfc8504/catalog.md) |

The three documents revise by replacement, never in place, so the document identity pins the
version by itself.

RFC 8504 is a multi-protocol document, like RFC 1122. Only the IP-layer sections named above
enter the IPv6 set; the parts that stay out, with the reason:

| RFC 8504 part | Reason it is out of the IPv6 set |
| --- | --- |
| §4 Sub-IP layer | link layers |
| §5.3 Protecting a node from excessive options | every statement is a `may` with a `should` inside; the options are level 5 |
| §5.4 to §5.6, §5.9 to §5.11 | neighbor discovery, SEND, router advertisement flags, router preferences, first-hop selection, MLD: protocols of their own |
| §5.12 ECN, the flow label sentences of §5.1 | the traffic class and the flow label are out of scope in the RFC 8200 catalog |
| §6 to §17 | addressing and configuration, DNS, transition, applications, mobility, security, router functions, constrained devices, management |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 8201 | Path MTU discovery is a control loop driven by Packet Too Big; level 4. This pass checks that the router sends the message, not what the source does with it. |
| RFC 4291, RFC 4861, RFC 4862 | Addressing, neighbor discovery and autoconfiguration are protocols of their own. The mockup needs them to function; the checks do not judge them. |
| RFC 6437, RFC 2474, RFC 3168 | The flow label and the traffic class byte are out of scope in the catalog itself. |
| RFC 2675, RFC 9673, RFC 4884 | Jumbograms, the hop-by-hop procedures and extended ICMP change areas the catalog does not cover. |
| RFC 2460, RFC 2463 | Obsoleted. Named here because the model documentation cites them; see the conformance document. |
| RFC 5722, RFC 6946, RFC 7112, RFC 8021 | The fragment rules they added are in RFC 8200 §4.5 and in RFC 8504 §5.1 and §5.2, which quote them; the catalogs cite the two in-scope texts. |
| RFC 7739, RFC 6936 | Guidance and an exception; neither adds a requirement to the level 2 set. |
