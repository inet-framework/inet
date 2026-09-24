# ND — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around IPv6 Neighbor Discovery, records which document governs each contested clause, and
pins the set that the next pass tests against. Neighbor Discovery has two base documents:
the discovery and resolution protocol itself, and stateless address autoconfiguration, which
depends on it. [`ipv6/standards.md`](../ipv6/standards.md#document-list) names both as
protocols of their own and leaves them out of the IPv6 pass for that reason; this document is
where they get their own map.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 4861 §4 (message formats), §6 (router and host processing, minus the flags reserved for MIPv6 and SEND), §7.2 (address resolution), §8 (redirect); RFC 4862 §5 (address states, Duplicate Address Detection); RFC 5942 §6 (the on-link definition override); RFC 6980 §5 (the fragmentation prohibition) | the normal exchange: Router Solicitation answered by Router Advertisement, address resolution by Neighbor Solicitation and Advertisement, Duplicate Address Detection from tentative to preferred, and Redirect |
| level 3, Edge | nothing new | RFC 4861 §6.1.2, §7.1.1 and §7.1.2 hold the validation rules for a received Router Advertisement, Neighbor Solicitation and Neighbor Advertisement. A crafted message exercises them. No separate host-requirements companion exists for ND the way RFC 1122 serves ARP: RFC 8504 explicitly excludes the neighbor-discovery sections (see [`ipv6/standards.md`](../ipv6/standards.md#override-table)) |
| level 4, Dynamics | RFC 4861 §6.2.6 and §6.3.4 (the randomized Router Advertisement interval and reachable-time jitter); RFC 7048 (relaxed Neighbor Unreachability Detection timing); RFC 7559 (Router Solicitation retransmission after loss); RFC 8319 (raised bounds on the Router Advertisement interval and router lifetime) | the timers and the control loops: Neighbor Unreachability Detection, the periodic and jittered Router Advertisement, the retransmission of a lost solicitation |
| level 5, Complete | RFC 4429 (optimistic Duplicate Address Detection); RFC 7527 (enhanced DAD, loopback detection); RFC 8028 (first-hop router selection in a multi-prefix network); RFC 9131 (gratuitous Neighbor Discovery); RFC 9762 (the DHCPv6 prefix-delegation flag); RFC 9685, RFC 9926 (6LoWPAN and RPL neighbor-discovery extensions); anycast targets, and multiple global addresses from different advertised prefixes | the optional alternatives and the specialized scenarios: an address used before DAD completes, a second detection layer, a host with more than one upstream router, a router that helps a new neighbor, a constrained 6LoWPAN network |

What a pass actually reached is not recorded here. It is in
[`model/nd/coverage.md`](../../model/nd/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 4861 | Neighbor Discovery for IP version 6 (IPv6) | September 2007 | Draft Standard | `base`; `obsoletes` RFC 2461 | [`standards/RFC/rfc4861.txt`](../../../../../../standards/RFC/rfc4861.txt), 2026-09-23 |
| RFC 4862 | IPv6 Stateless Address Autoconfiguration | September 2007 | Draft Standard | `base`; `obsoletes` RFC 2462 | [`standards/RFC/rfc4862.txt`](../../../../../../standards/RFC/rfc4862.txt), 2026-09-23 |
| RFC 2461 | Neighbor Discovery for IP Version 6 (IPv6) | December 1998 | Draft Standard, obsoleted | obsoleted by RFC 4861 | no |
| RFC 2462 | IPv6 Stateless Address Autoconfiguration | December 1998 | Draft Standard, obsoleted | obsoleted by RFC 4862 | no |
| RFC 4429 | Optimistic Duplicate Address Detection (DAD) for IPv6 | April 2006 | Proposed Standard | `companion`; `updates` RFC 2461, RFC 2462 (both now obsoleted; RFC 7527 groups it with RFC 4861 and RFC 4862 as the documents it updates) | no |
| RFC 4443 | Internet Control Message Protocol (ICMPv6) | March 2006 | Internet Standard (STD 89) | `companion`; every ND message is an ICMPv6 message | already downloaded, [`standards/RFC/rfc4443.txt`](../../../../../../standards/RFC/rfc4443.txt) — belongs to the ipv6 pass; referenced here, not copied |
| RFC 5942 | IPv6 Subnet Model: The Relationship between Links and Subnet Prefixes | July 2010 | Proposed Standard | `updates` RFC 4861 | [`standards/RFC/rfc5942.txt`](../../../../../../standards/RFC/rfc5942.txt), 2026-09-23 |
| RFC 6980 | Security Implications of IPv6 Fragmentation with IPv6 Neighbor Discovery | August 2013 | Proposed Standard | `updates` RFC 4861 (also RFC 3971, out of this family) | [`standards/RFC/rfc6980.txt`](../../../../../../standards/RFC/rfc6980.txt), 2026-09-23 |
| RFC 7048 | Neighbor Unreachability Detection Is Too Impatient | January 2014 | Proposed Standard | `updates` RFC 4861 | no |
| RFC 7527 | Enhanced Duplicate Address Detection | April 2015 | Proposed Standard | `updates` RFC 4429, RFC 4861, RFC 4862 | no |
| RFC 7559 | Packet-Loss Resiliency for Router Solicitations | May 2015 | Proposed Standard | `updates` RFC 4861 | no |
| RFC 8028 | First-Hop Router Selection by Hosts in a Multi-Prefix Network | November 2016 | Proposed Standard | `updates` RFC 4861 | no |
| RFC 8319 | Support for Adjustable Maximum Router Lifetimes per Link | February 2018 | Proposed Standard | `updates` RFC 4861 | no |
| RFC 8425 | IANA Considerations for IPv6 Neighbor Discovery Prefix Information Option Flags | July 2018 | Proposed Standard | `updates` RFC 4861 (registry pointer only) | no |
| RFC 9131 | Gratuitous Neighbor Discovery: Creating Neighbor Cache Entries on First-Hop Routers | October 2021 | Proposed Standard | `updates` RFC 4861 | no |
| RFC 9685 | Listener Subscription for IPv6 Neighbor Discovery Multicast and Anycast Addresses | November 2024 | Proposed Standard | `updates` RFC 4861 (and RPL documents RFC 6550, RFC 6553, RFC 8505, RFC 9010) | no |
| RFC 9762 | Using Router Advertisements to Signal the Availability of DHCPv6 Prefix Delegation to Clients | June 2025 | Proposed Standard | `updates` RFC 4861, RFC 4862 | no |
| RFC 9926 | Prefix Registration for IPv6 Neighbor Discovery | February 2026 | Proposed Standard | `updates` RFC 4861 (and RPL documents RFC 6550, RFC 8505, RFC 8928, RFC 9010) | no |

Source of the texts:

- `rfc4861.txt`, `rfc4862.txt`, `rfc5942.txt`, `rfc6980.txt` in their own folders under
  [`evidence/standard/`](../../standard/) — `https://www.rfc-editor.org/rfc/rfcNNNN.txt`,
  downloaded 2026-09-23.
- `rfc4443.txt` is not fetched again; the ipv6 pass owns it at
  [`standards/RFC/rfc4443.txt`](../../../../../../standards/RFC/rfc4443.txt).

Both documents postdate RFC 2119 and use its keywords throughout: `grep -c '\bMUST\b'
rfc4861.txt` counts 151 lines, `\bSHOULD\b` counts 65, `\bMAY\b` counts 23. RFC 4862 counts 17
`MUST` lines, 11 `SHOULD` lines and 5 `MAY` lines. RFC 5942 and RFC 6980 also use the
keywords; RFC 6980 §5 is the one clause this pass quotes, and it is a `MUST NOT`.

Two relationship notes the register alone does not give:

- **RFC 4429 updates two obsoleted documents.** Its own header says `Updates: 2461, 2462`,
  because it predates RFC 4861 and RFC 4862 by over a year. RFC 7527 later updates "RFCs 4429,
  4861, and 4862" in the same sentence, which is the register's confirmation that RFC 4429
  travels with the current pair, not with the obsoleted one.
- **RFC 4861 and RFC 4862 are Draft Standard, not Internet Standard**, and nothing has
  reclassified them since 2007. This is the maturity level the IETF used before the 2011
  process change (RFC 6410) collapsed three stages into two; it does not mean the document is
  provisional. No later document proposes to obsolete either one.

## Override table

One row per clause-level conflict. `Governs` names the document a test must follow when the
two texts disagree. Line references are into the downloaded files; a clause reference stands for
a document that is not downloaded.

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The on-link definition | RFC 4861 §2.1, `rfc4861.txt:287-302`: a node considers an address on-link if it is covered by a prefix, a Redirect names it, **or** an NA or any ND message is received for or from it (the last two bullets, `rfc4861.txt:297-302`) | RFC 5942 §6, `rfc5942.txt:459-467`: "This document deprecates the following two bullets from the on-link definition... a Neighbor Advertisement message is received... or any Neighbor Discovery message is received from the address" | RFC 5942 | **yes**, level 2 |
| Fragmentation of a Neighbor Discovery message | RFC 4861 defines the message formats without forbidding IPv6 fragmentation | RFC 6980 §5, `rfc6980.txt:312-320`: "Nodes MUST NOT employ IPv6 fragmentation for sending any of the following Neighbor Discovery... messages: Neighbor Solicitation, Neighbor Advertisement, Router Solicitation, Router Advertisement, Redirect..." | RFC 6980 | **yes**, level 2 |
| Neighbor Unreachability Detection retransmission timing | RFC 4861 §7.3.3 fixes the retransmission behavior | RFC 7048 §1: "if there are no alternative neighbors, this timeout behavior is far too impatient", and it allows a longer timeout when no alternative exists | RFC 7048 | no; level 4 |
| DAD and the use of a tentative address | RFC 4862 §5.4, `rfc4862.txt:271-276`: a tentative address "is not considered assigned to an interface"; an interface discards packets addressed to it until DAD completes | RFC 4429 §1: Optimistic DAD lets a host use the address as a source before DAD completes, in the successful case, while remaining interoperable with unmodified nodes | RFC 4862 by default; RFC 4429 when the optimistic mode is configured | no; level 5, by the guide's rule that the in-scope set comes from the register, not the claim |
| Loopback of a self-generated DAD solicitation | RFC 4862 §5.4.3 treats any Neighbor Solicitation for a tentative address, other than the sender's own, as a duplicate | RFC 7527: a hardware or software loopback can make a node see its own solicitation and wrongly declare a duplicate; the Nonce option detects this | RFC 7527 | no; level 5 |
| The Router Advertisement interval and router-lifetime bounds | RFC 4861 §6.2.1, §4.2: `MaxRtrAdvInterval` capped at 1800 s, `AdvDefaultLifetime`/Router Lifetime capped at 9000 s | RFC 8319 §4: raises both caps to 65535 s, per link | RFC 8319 | no; level 4 |
| Creating a Neighbor Cache entry and sending an unsolicited NA | RFC 4861 §7.2.3 creates an entry only on receiving a solicitation with a Source Link-Layer Address option; §7.2.6 already makes an unsolicited NA on address assignment optional | RFC 9131: routers SHOULD proactively create an entry when a new address appears on-link, and nodes SHOULD send an unsolicited NA on assigning a new address | RFC 9131 | no; level 5 — every new rule is a `SHOULD`, and the router side needs configuration; RFC 4861 §7.2.6 already gives the host side an optional path to the same unsolicited NA, so a level 5 pass has to separate the two mechanisms |
| The Prefix Information Option's reserved flag bits | RFC 4861 §4.6.2 leaves six bits reserved with no registry | RFC 8425: creates an IANA registry for those bits; states no new host or router behavior | RFC 8425 | no; administrative only, no behavior to test |

RFC 7559, RFC 8028, RFC 9685, RFC 9762 and RFC 9926 update RFC 4861 without contesting a
clause a level 2 pass would check: RFC 7559 adds a retry timer around an already-optional
step (level 4); RFC 8028 is host-internal router selection among several already-valid
routers, needing a multi-router, multi-prefix mockup (level 5); RFC 9685 and RFC 9926 extend
the option set for 6LoWPAN and RPL routers, a constrained-network protocol family of its own
(level 5); RFC 9762 defines one new PIO flag bit that only changes behavior when a DHCPv6
prefix-delegation deployment sets it (level 5). None of them removes or negates a clause of
the level 2 in-scope set.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 4861 | September 2007, Draft Standard, no revision of the body since; §4, §6 minus the MIPv6/SEND-reserved flags, §7.2, §8 | none yet; level 2 writes `standard/rfc4861/catalog.md` |
| RFC 4862 | September 2007, Draft Standard, no revision of the body since; §5 | none yet; level 2 writes `standard/rfc4862/catalog.md` |
| RFC 5942 | July 2010, Proposed Standard; §6 (the on-link override) | none yet |
| RFC 6980 | August 2013, Proposed Standard; §5 (the fragmentation prohibition) | none yet |

RFC 4443 stays a background companion, exactly as in the ipv6 pass: every ND message is an
ICMPv6 message, and the checks tolerate its general rules without re-deriving them; see
[`ipv6/standards.md`](../ipv6/standards.md#document-list).

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 2461 | Obsoleted by RFC 4861. |
| RFC 2462 | Obsoleted by RFC 4862. |
| RFC 4429 | Level 5. Optimistic DAD is an alternative to the mandatory tentative-address procedure, on by configuration only. |
| RFC 4443 | Owned by the ipv6 pass; referenced as a companion, not re-scoped here. |
| RFC 7048, RFC 7559, RFC 8319 | Level 4. Retransmission and interval timing, all with a random or a configurable component. |
| RFC 7527 | Level 5. A second, optional detection layer on top of the mandatory DAD procedure. |
| RFC 8028 | Level 5. Host behavior in a multi-router, multi-prefix network; the mockup RFC 4861 alone does not need. |
| RFC 8425 | No behavior of a node changes; it only names a registry for bits RFC 4861 already reserves. |
| RFC 9131 | Level 5. Every new rule is a `SHOULD`, and it needs a configured router. |
| RFC 9685, RFC 9926 | 6LoWPAN and RPL neighbor-discovery extensions — a constrained-network protocol family of its own, the way MIPv6 and PMIPv6 are their own families relative to IPv6. |
| RFC 9762 | Level 5. A new PIO flag whose effect depends on a DHCPv6 prefix-delegation deployment. |
