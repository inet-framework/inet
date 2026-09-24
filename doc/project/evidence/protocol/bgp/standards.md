# BGP-4 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around the Border Gateway Protocol version 4, records which document governs each contested
clause, and pins the set that the next pass tests against. BGP-4 has one base document, with a
long chain of updates: twelve documents currently update it, and several more documents define
companion mechanisms it can carry.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 4271 §3 to §5 (excluding the optional path attributes of §5), §6.8, §8 and §9; RFC 4760; RFC 5492; RFC 6793; RFC 8212 | the normal session: the Finite State Machine, the OPEN/UPDATE/KEEPALIVE exchange, connection collision, the decision and update-send process, the multiprotocol and capability negotiation the model uses for IPv6, and two still-current updates that change the normal exchange itself: the four-octet AS number capability and the default route-propagation policy of an EBGP session |
| level 3, Edge | RFC 4271 §6 (Error Handling, NOTIFICATION); RFC 6608; RFC 7606; RFC 7607; RFC 9072; RFC 9774 | the base document reports every session-ending fault with a NOTIFICATION message; these five updates add subcodes, revise the error handling of the UPDATE message, proscribe AS 0, extend the OPEN message length encoding for a long optional-parameter list, and forbid the AS_SET and AS_CONFED_SET segment types — none of them appears on the normal path |
| level 4, Dynamics | RFC 4271 §8 timer events (ConnectRetryTimer, HoldTimer, KeepaliveTimer) and §9.2.1 (MinASOriginationIntervalTimer, MinRouteAdvertisementIntervalTimer); RFC 9687 | every BGP timer, including the two damping timers of the base document and the newest addition, a sender-side hold timer |
| level 5, Complete | RFC 4724; RFC 8654; RFC 6286; RFC 1997; RFC 7911; RFC 4456; RFC 5065 (AS confederations) | optional capabilities and attributes: graceful restart, larger messages, the AS-wide identifier relaxation, communities, multiple paths, route reflection, and confederations |

What a pass actually reached is not recorded here. It is in
[`model/bgp/coverage.md`](../../model/bgp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 4271 | A Border Gateway Protocol 4 (BGP-4) | January 2006 | Draft Standard | `base`; obsoletes RFC 1771 | [`standards/RFC/rfc4271.txt`](../../../../../../standards/RFC/rfc4271.txt), 2026-09-23 |
| RFC 1771 | A Border Gateway Protocol 4 (BGP-4) | March 1995 | Draft Standard | `obsoleted by` RFC 4271 | no |
| RFC 4760 | Multiprotocol Extensions for BGP-4 | January 2007 | Draft Standard | `companion`; obsoletes RFC 2858; the multiprotocol mechanism the model uses for IPv6 | [`standards/RFC/rfc4760.txt`](../../../../../../standards/RFC/rfc4760.txt), 2026-09-23 |
| RFC 5492 | Capabilities Advertisement with BGP-4 | February 2009 | Draft Standard | `companion`; obsoletes RFC 3392; the capability-negotiation mechanism RFC 4760 and RFC 6793 both use | [`standards/RFC/rfc5492.txt`](../../../../../../standards/RFC/rfc5492.txt), 2026-09-23 |
| RFC 8810 | Revision to Capability Codes Registration Procedures | August 2020 | Proposed Standard | `updates` RFC 5492 | no |
| RFC 6793 | BGP Support for Four-Octet Autonomous System (AS) Number Space | December 2012 | Proposed Standard | `updates` RFC 4271; obsoletes RFC 4893 | [`standards/RFC/rfc6793.txt`](../../../../../../standards/RFC/rfc6793.txt), 2026-09-23 |
| RFC 8212 | Default External BGP (EBGP) Route Propagation Behavior without Policies | July 2017 | Proposed Standard | `updates` RFC 4271 | [`standards/RFC/rfc8212.txt`](../../../../../../standards/RFC/rfc8212.txt), 2026-09-23 |
| RFC 6286 | Autonomous-System-Wide Unique BGP Identifier for BGP-4 | June 2011 | Proposed Standard | `updates` RFC 4271 | no |
| RFC 6608 | Subcodes for BGP Finite State Machine Error | May 2012 | Proposed Standard | `updates` RFC 4271 | no |
| RFC 7606 | Revised Error Handling for BGP UPDATE Messages | August 2015 | Proposed Standard | `updates` RFC 4271, RFC 4760, RFC 4456, RFC 1997, and others | no |
| RFC 7607 | Codification of AS 0 Processing | August 2015 | Proposed Standard | `updates` RFC 4271 | no |
| RFC 7705 | Autonomous System Migration Mechanisms and Their Effects on the BGP AS_PATH Attribute | December 2015 | Proposed Standard | `updates` RFC 4271 | no |
| RFC 8654 | Extended Message Support for BGP | October 2019 | Proposed Standard | `updates` RFC 4271 | no |
| RFC 9072 | Extended Optional Parameters Length for BGP OPEN Message | July 2021 | Proposed Standard | `updates` RFC 4271 | no |
| RFC 9687 | Border Gateway Protocol 4 (BGP-4) Send Hold Timer | November 2024 | Proposed Standard | `updates` RFC 4271 | no |
| RFC 9774 | Deprecation of AS_SET and AS_CONFED_SET in BGP | May 2025 | Proposed Standard | `updates` RFC 4271, RFC 5065; obsoletes RFC 6472 | no |
| RFC 4724 | Graceful Restart Mechanism for BGP | January 2007 | Proposed Standard | `updates` RFC 4271; `companion` | no |
| RFC 1997 | BGP Communities Attribute | August 1996 | Proposed Standard | `companion`; updated by RFC 7606, RFC 8642 | no |
| RFC 7911 | Advertisement of Multiple Paths in BGP | July 2016 | Proposed Standard | `companion` | no |
| RFC 4456 | BGP Route Reflection: An Alternative to Full Mesh Internal BGP (IBGP) | April 2006 | Draft Standard | `companion`; obsoletes RFC 1966 and RFC 2796; updated by RFC 7606 | no |

Source of the texts:

- `rfc4271.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc4271.txt) —
  <https://www.rfc-editor.org/rfc/rfc4271.txt>, downloaded 2026-09-23.
- `rfc4760.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc4760.txt) —
  <https://www.rfc-editor.org/rfc/rfc4760.txt>, downloaded 2026-09-23.
- `rfc5492.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc5492.txt) —
  <https://www.rfc-editor.org/rfc/rfc5492.txt>, downloaded 2026-09-23.
- `rfc6793.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc6793.txt) —
  <https://www.rfc-editor.org/rfc/rfc6793.txt>, downloaded 2026-09-23.
- `rfc8212.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc8212.txt) —
  <https://www.rfc-editor.org/rfc/rfc8212.txt>, downloaded 2026-09-23.

All five downloaded documents use the keywords of RFC 2119. Counts of whole-word matches: RFC 4271
has `MUST` on 98 lines, `MUST NOT` on 9, `SHOULD` on 77, `SHALL` on 32, `MAY` on 38. RFC 4760 has
`MUST` on 10 lines and `SHOULD` on 10. RFC 5492 has `MUST` on 11 lines and `SHOULD` on 6.
RFC 6793 has `MUST` on 23 lines, `SHALL` on 12. RFC 8212 has `MUST` once and `SHALL` three
times, all in its two-paragraph change to RFC 4271 §9.1 and §9.1.3.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| AS number width in the OPEN capability and the AS_PATH and AGGREGATOR attributes | RFC 4271 §4.2, `rfc4271.txt:710-713`: "My Autonomous System: This 2-octet unsigned integer..."; §5.1.2, `rfc4271.txt:1005-1006`: "one or more AS numbers, each encoded as a 2-octet length field" | RFC 6793 §3 and §4.1-4.2.2, `rfc6793.txt:93-266`: a BGP Capability advertises support for a 4-octet AS number, and a speaker that negotiates it with a peer encodes AS_PATH and AGGREGATOR with 4-octet AS numbers | RFC 6793, when both peers negotiate the capability; RFC 4271 §4.2 still governs the wire width for a peer that does not | yes, from level 2 |
| Default route-propagation policy of an EBGP session with no configured policy | RFC 4271 §9.1, `rfc4271.txt:4207`, and §9.1.3, `rfc4271.txt:4571`: the Decision Process and Route Dissemination phases state no default for the case where no Import or Export Policy is configured | RFC 8212 §3, `rfc8212.txt:145-166`: "Routes ... SHALL NOT be considered eligible in the Decision Process if no explicit Import Policy has been applied" and the matching SHALL NOT for the Adj-RIB-Out | RFC 8212 | yes, from level 2 |
| Uniqueness scope of the BGP Identifier | RFC 4271 §4.2, `rfc4271.txt:238-243`: the BGP Identifier is "the same for every local interface and BGP peer", with no scope stated for its uniqueness | RFC 6286 §3, "Remarks": relaxes the requirement from a globally unique value to one unique within the Autonomous System | RFC 6286 | no; folds into the RFC 4271 field with no independent wire behavior, see the out-of-scope table |
| The Optional Parameters Length field of the OPEN message | RFC 4271 §4.2, one octet, limiting the optional-parameter list to 255 octets | RFC 9072 §3, an implementation "MUST accept an OPEN message that uses" an extended, backward-compatible encoding once the list would exceed 255 octets | RFC 9072, only when the optional-parameter list is long enough to need it | no; level 3, see the target-level table |

RFC 4271 obsoletes RFC 1771 as a whole; no clause-level conflict exists between the two, because
RFC 1771 is superseded in full.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 4271 | January 2006, Draft Standard; §3 to §5 excluding the optional path attributes of §5.1.3 to §5.1.6, §6.8, §8, §9 | none yet; level 2 writes `standard/rfc4271/catalog.md` |
| RFC 4760 | January 2007, Draft Standard; in full | none yet; level 2 writes `standard/rfc4760/catalog.md` |
| RFC 5492 | February 2009, Draft Standard; in full | none yet; level 2 writes `standard/rfc5492/catalog.md` |
| RFC 6793 | December 2012, Proposed Standard; §3 and §4 | none yet; level 2 writes `standard/rfc6793/catalog.md` |
| RFC 8212 | July 2017, Proposed Standard; §3 | none yet; level 2 writes `standard/rfc8212/catalog.md` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 1771 | Obsoleted by RFC 4271. |
| RFC 8810 | Revises the IANA registration procedure for BGP Capability Codes. It states no behavior of a BGP speaker. |
| RFC 6286 | No `MUST` or `SHOULD` in the whole document; it relaxes a definition with no wire-format change and no independent test, see the override table. |
| RFC 6608 | Level 3. It adds subcodes to the NOTIFICATION message of §6, which the base document uses only to report a fault. |
| RFC 7606 | Level 3. It revises the error handling of the UPDATE message: what a speaker does with a malformed optional attribute. |
| RFC 7607 | Level 3. It proscribes AS 0 in five fields; a check needs a crafted OPEN or UPDATE message that carries it. |
| RFC 7705 | Not a protocol requirement. Its own abstract calls the AS-migration techniques it documents "not formally part of the BGP4 protocol specification". |
| RFC 8654 | Level 5. A larger maximum message size is an optional capability; the normal path stays at 4096 octets. |
| RFC 9072 | Level 3. The extended Optional Parameters length encoding of the OPEN message applies only when the parameter list would exceed 255 octets, a boundary condition and not the normal case. |
| RFC 9687 | Level 4. It adds a new, sender-side hold timer to the Finite State Machine. |
| RFC 9774 | Level 3. It turns an existing `SHOULD NOT` on the AS_SET and AS_CONFED_SET path segments into a `MUST NOT`; a check needs a crafted AS_PATH that carries one. |
| RFC 4724 | Level 5. Graceful restart is a capability that changes FSM behavior only when both peers negotiate it. |
| RFC 1997 | Level 5. The COMMUNITIES attribute is optional and transitive; the normal exchange does not require it. |
| RFC 7911 | Level 5. Advertising multiple paths for one prefix is an optional capability. |
| RFC 4456 | Level 5. Route reflection is an optional iBGP scalability mechanism for a topology of more than a handful of routers. |
| RFC 5065 | Level 5. AS confederations are an optional way to structure one AS as several sub-ASes; the normal exchange does not use them. |
