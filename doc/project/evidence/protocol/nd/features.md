# ND — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `ND-F-*` · **Stands on:** [standards.md](standards.md), [rfc4861/catalog.md](../../standard/rfc4861/catalog.md), [rfc4862/catalog.md](../../standard/rfc4862/catalog.md), [rfc5942/catalog.md](../../standard/rfc5942/catalog.md), [rfc6980/catalog.md](../../standard/rfc6980/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and per
document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level of
each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/nd/coverage.md), and step 8 compares it with the claims of the
  model in [`conformance.md`](../../model/nd/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/nd/coverage.md).

## How this map decides the level

The rule of the guide: `mandatory` when any core statement says `must` or `shall`, or when the
mechanism is the only path the document gives to a state or an outcome; `optional` when every
core statement says `may` or `should`; `unstated` otherwise.

One refinement, which the DHCP and RIP maps state too: **a conditional keyword does not raise
the level of a feature.** RFC 4861 says that a router "SHOULD send a redirect message" and then
states with `MUST` what the Redirect holds; the `MUST` holds only once a Redirect is sent. The
level comes from the statement that decides whether the mechanism has to exist at all, and
each feature names it.

## Index

| ID | Feature |
| --- | --- |
| [ND-F-MESSAGE-FORMAT](#nd-f-message-format) | Every Neighbor Discovery message is an ICMPv6 message with its own type, code 0, hop limit 255 and zero in its reserved fields. |
| [ND-F-OPTIONS](#nd-f-options) | Options follow the message in type-length-value form: link-layer address, Prefix Information, Redirected Header and MTU. |
| [ND-F-MULTICAST-GROUPS](#nd-f-multicast-groups) | A node joins the all-nodes group and the solicited-node group of each of its addresses, and a router also joins the all-routers group. |
| [ND-F-ROUTER-SOLICITATION](#nd-f-router-solicitation) | A host that comes up asks for Router Advertisements with a few Router Solicitations to the all-routers group. |
| [ND-F-ROUTER-ADVERTISEMENT](#nd-f-router-advertisement) | A router advertises itself, its parameters and its prefixes from its link-local address, to all nodes or to the soliciting host. |
| [ND-F-ADVERTISEMENT-TIMING](#nd-f-advertisement-timing) | Unsolicited Router Advertisements come at a random interval between MinRtrAdvInterval and MaxRtrAdvInterval, faster at the start, and never closer than MIN_DELAY_BETWEEN_RAS. |
| [ND-F-ROUTER-DISCOVERY](#nd-f-router-discovery) | A host keeps a Default Router List from the Router Lifetime of the advertisements, and it sends off-link traffic to a router of that list. |
| [ND-F-PARAMETER-DISCOVERY](#nd-f-parameter-discovery) | A host adopts the hop limit, the reachable time, the retransmission timer and the MTU that a router advertises. |
| [ND-F-ON-LINK-DETERMINATION](#nd-f-on-link-determination) | A host treats as on-link exactly the addresses that an advertised on-link prefix or a Redirect covers, and sends everything else to a router. |
| [ND-F-ADDRESS-RESOLUTION](#nd-f-address-resolution) | A node finds the link-layer address of an on-link neighbor with a multicast Neighbor Solicitation, and the neighbor answers with a solicited Neighbor Advertisement. |
| [ND-F-RESOLUTION-FAILURE](#nd-f-resolution-failure) | A node repeats an unanswered solicitation every RetransTimer, keeps a small queue, and gives up after MAX_MULTICAST_SOLICIT tries with a Destination Unreachable for the queued packets. |
| [ND-F-UNSOLICITED-ADVERTISEMENT](#nd-f-unsolicited-advertisement) | A node whose link-layer address changes may tell its neighbors with a few unsolicited Neighbor Advertisements to all nodes. |
| [ND-F-ANYCAST-AND-PROXY](#nd-f-anycast-and-proxy) | Anycast and proxied addresses are answered with the Override flag clear and after a random delay. |
| [ND-F-REDIRECT](#nd-f-redirect) | A router that forwards a packet to a better first hop on the same link tells the sender with a Redirect. |
| [ND-F-REDIRECT-PROCESSING](#nd-f-redirect-processing) | A host that receives a valid Redirect sends later traffic for the destination to the target. |
| [ND-F-MESSAGE-VALIDATION](#nd-f-message-validation) | A node silently discards a Neighbor Discovery message that fails the validity checks, and ignores reserved fields and unknown options. |
| [ND-F-ROUTER-ROLE-CHANGE](#nd-f-router-role-change) | A router that stops advertising, changes its link-local address or becomes a host tells the hosts with final advertisements. |
| [ND-F-ROUTER-CONSISTENCY](#nd-f-router-consistency) | A router checks the advertisements of other routers on the link and logs the values that conflict with its own. |
| [ND-F-LINK-LOCAL-ADDRESS](#nd-f-link-local-address) | A node forms a link-local address from the prefix FE80::/64 and its interface identifier whenever an interface becomes enabled. |
| [ND-F-DUPLICATE-ADDRESS-DETECTION](#nd-f-duplicate-address-detection) | Before a node assigns a unicast address, it holds the address tentative and sends Neighbor Solicitations from the unspecified address for it; a solicitation or an advertisement from another node makes it a duplicate. |
| [ND-F-STATELESS-AUTOCONFIGURATION](#nd-f-stateless-autoconfiguration) | A host forms a global address from each advertised prefix with the A flag set and its interface identifier, with the lifetimes of the prefix. |
| [ND-F-ADDRESS-LIFETIME](#nd-f-address-lifetime) | An autoconfigured address becomes deprecated when its preferred lifetime ends and invalid when its valid lifetime ends. |
| [ND-F-NO-FRAGMENTATION](#nd-f-no-fragmentation) | Neighbor Discovery messages are never fragmented, and a node ignores one that arrives fragmented. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [ND-F-MESSAGE-FORMAT](#nd-f-message-format) | mandatory | RFC 4861 §4.1 to §4.5 | RFC4861-RS-5, RS-6, RS-7, RS-9, RA-5, RA-6, RA-7, RA-13, NS-6, NS-7, NS-8, NS-10, NA-6, NA-7, NA-8, NA-19, RDM-5, RDM-6, RDM-7, RDM-9 |
| [ND-F-OPTIONS](#nd-f-options) | mandatory | RFC 4861 §4.6, §4.6.1 to §4.6.4 | RFC4861-OPT-3, OPT-5, OPT-8, OPT-9, OPT-15, OPT-16, OPT-17, OPT-22, OPT-29, OPT-32, OPT-45, OPT-46, OPT-47 |
| [ND-F-MULTICAST-GROUPS](#nd-f-multicast-groups) | mandatory | RFC 4861 §6.2.2, §6.3.1, §7.2.1; RFC 4862 §5.4.2 | RFC4861-AR-4, AR-5, HOST-8, ADV-3 |
| [ND-F-ROUTER-SOLICITATION](#nd-f-router-solicitation) | optional | RFC 4861 §4.1, §6.3.7 | RFC4861-HOST-42, HOST-44, HOST-45, HOST-47, HOST-50, HOST-52, RS-3, RS-4, RS-11, RS-12 |
| [ND-F-ROUTER-ADVERTISEMENT](#nd-f-router-advertisement) | mandatory | RFC 4861 §4.2, §6.2.1 to §6.2.3, §6.2.6 | RFC4861-RA-3, RA-4, RVAL-11, ADV-1, ADV-4, ADV-5, ADV-6, ADV-7, ADV-8, ADV-9, ADV-11, ADV-12, ADV-13, ADV-14, ADV-15, ADV-16, ADV-29, RA-19, RA-21, RA-27 |
| [ND-F-ADVERTISEMENT-TIMING](#nd-f-advertisement-timing) | mandatory | RFC 4861 §6.2.4, §6.2.6 | RFC4861-ADV-22, ADV-23, ADV-32, ADV-33, ADV-36 |
| [ND-F-ROUTER-DISCOVERY](#nd-f-router-discovery) | mandatory | RFC 4861 §4.2, §6.3.4 to §6.3.6 | RFC4861-HOST-12, HOST-13, HOST-14, RA-17, HOST-37, HOST-39 |
| [ND-F-PARAMETER-DISCOVERY](#nd-f-parameter-discovery) | optional | RFC 4861 §6.3.2, §6.3.4; RFC 4862 §5.6 | RFC4861-HOST-16, HOST-17, HOST-19, HOST-24 |
| [ND-F-ON-LINK-DETERMINATION](#nd-f-on-link-determination) | mandatory | RFC 4861 §4.6.2, §6.3.4, §6.3.5; RFC 5942 §4, §6 | RFC4861-HOST-25, HOST-30, HOST-31, HOST-32, HOST-36, RFC5942-ONLINK-1, ONLINK-2, ONLINK-3, ONLINK-5, ONLINK-6, ONLINK-9, ONLINK-10 |
| [ND-F-ADDRESS-RESOLUTION](#nd-f-address-resolution) | mandatory | RFC 4861 §4.3, §4.4, §7.2.2 to §7.2.5 | RFC4861-AR-8, AR-9, AR-12, AR-18, AR-24, AR-31, AR-32, AR-34, AR-35, AR-37, AR-39, AR-44, AR-45, NS-5, NS-15, NA-4, NA-12, NA-24 |
| [ND-F-RESOLUTION-FAILURE](#nd-f-resolution-failure) | mandatory | RFC 4861 §7.2.2 | RFC4861-AR-19, AR-20, AR-21, AR-22 |
| [ND-F-UNSOLICITED-ADVERTISEMENT](#nd-f-unsolicited-advertisement) | optional | RFC 4861 §4.4, §7.2.6 | RFC4861-AR-57, AR-58, AR-59, AR-60, AR-61 |
| [ND-F-ANYCAST-AND-PROXY](#nd-f-anycast-and-proxy) | optional | RFC 4861 §7.2.7, §7.2.8 | RFC4861-ANY-2, ANY-3, ANY-8 |
| [ND-F-REDIRECT](#nd-f-redirect) | optional | RFC 4861 §4.5, §4.6.3, §8, §8.2 | RFC4861-RDR-3, RDR-4, RDR-5, RDR-6, RDM-3, RDM-4, RDM-11, RDM-12, RDM-13, RDM-14 |
| [ND-F-REDIRECT-PROCESSING](#nd-f-redirect-processing) | optional | RFC 4861 §8.3 | RFC4861-RDH-1, RDH-2, RDH-3, RDH-6, RDH-7 |
| [ND-F-MESSAGE-VALIDATION](#nd-f-message-validation) | mandatory | RFC 4861 §4, §4.6, §6.1, §8.1; RFC 4862 §5.4.1 | RFC4861-RVAL-2, RVAL-10, RVAL-12, RDVAL-1, RDVAL-3, RDVAL-7, OPT-6 |
| [ND-F-ROUTER-ROLE-CHANGE](#nd-f-router-role-change) | optional | RFC 4861 §6.2.2, §6.2.4, §6.2.5, §6.2.8 | RFC4861-ADV-25, ADV-27, ADV-28 |
| [ND-F-ROUTER-CONSISTENCY](#nd-f-router-consistency) | optional | RFC 4861 §6.2.7 | RFC4861-ADV-42, ADV-43 |
| [ND-F-LINK-LOCAL-ADDRESS](#nd-f-link-local-address) | mandatory | RFC 4862 §5.3 | RFC4862-LL-2, LL-5, LL-7 |
| [ND-F-DUPLICATE-ADDRESS-DETECTION](#nd-f-duplicate-address-detection) | mandatory | RFC 4862 §5.1, §5.4; RFC 4861 §4.3, §7.2.4 | RFC4862-DAD-2, DAD-6, DAD-9, DAD-11, DAD-12, DAD-13, DAD-20, DAD-21, DAD-25, DAD-27 |
| [ND-F-STATELESS-AUTOCONFIGURATION](#nd-f-stateless-autoconfiguration) | mandatory | RFC 4862 §5.5.1 to §5.5.3; RFC 4861 §4.6.2 | RFC4862-GLOB-1, GLOB-5, GLOB-6, GLOB-7, GLOB-8, GLOB-9, GLOB-11, GLOB-12, GLOB-13, GLOB-15, RFC4861-OPT-21 |
| [ND-F-ADDRESS-LIFETIME](#nd-f-address-lifetime) | mandatory | RFC 4862 §5.5.4, §5.7 | RFC4862-GLOB-16, GLOB-18, GLOB-22, GLOB-23, GLOB-24 |
| [ND-F-NO-FRAGMENTATION](#nd-f-no-fragmentation) | mandatory | RFC 6980 §5 | RFC6980-FRAG-1, FRAG-2, FRAG-3, FRAG-4, FRAG-5, FRAG-6 |

## ND-F-MESSAGE-FORMAT

**Every Neighbor Discovery message is an ICMPv6 message with its own type, code 0, hop limit 255 and zero in its reserved fields.**

- **Sources** — RFC 4861 §4.1 to §4.5.
- **Level** — mandatory (reason: only path). Each message has one format and no other; a message with another hop limit is discarded by every receiver, RFC4861-RVAL-2, RVAL-12, RDVAL-3.
- **Description** — The five messages are Router Solicitation (type 133), Router Advertisement (134), Neighbor Solicitation (135), Neighbor Advertisement (136) and Redirect (137). Each one leaves its sender with IP hop limit 255, so that a receiver can tell that it did not cross a router, and with zero in the reserved bits.
- **Checks** — core: RFC4861-RS-5, RS-6, RS-7, RS-9, RA-5, RA-6, RA-7, RA-13, NS-6, NS-7, NS-8, NS-10, NA-6, NA-7, NA-8, NA-19, RDM-5, RDM-6, RDM-7, RDM-9. Supporting: RFC4861-RS-2, RS-8, RA-2, RA-8, NS-3, NS-9, NA-2, NA-9, RDM-2, RDM-8.

## ND-F-OPTIONS

**Options follow the message in type-length-value form: link-layer address, Prefix Information, Redirected Header and MTU.**

- **Sources** — RFC 4861 §4.6, §4.6.1 to §4.6.4.
- **Level** — mandatory (reason: only path). The options carry the link-layer addresses and the prefixes; without them no node learns either.
- **Description** — An option starts with an 8-bit type and an 8-bit length in units of 8 octets. The link-layer address options have type 1 and 2 and, on Ethernet, length 1; Prefix Information has type 3 and length 4; Redirected Header type 4; MTU type 5 and length 1. Reserved fields are zero, and the bits of a prefix after its length are zero.
- **Checks** — core: RFC4861-OPT-3, OPT-5, OPT-8, OPT-9, OPT-15, OPT-16, OPT-17, OPT-22, OPT-29, OPT-32, OPT-45, OPT-46, OPT-47. Supporting: RFC4861-OPT-1, OPT-2, OPT-4, OPT-7, OPT-10, OPT-11, OPT-12, OPT-14, OPT-31, OPT-37, OPT-38, OPT-39, OPT-40, OPT-42, OPT-44.

## ND-F-MULTICAST-GROUPS

**A node joins the all-nodes group and the solicited-node group of each of its addresses, and a router also joins the all-routers group.**

- **Sources** — RFC 4861 §6.2.2, §6.3.1, §7.2.1; RFC 4862 §5.4.2.
- **Level** — mandatory (reason: keyword). RFC4861-AR-4 has the strength "must".
- **Description** — The groups are how the multicast messages of Neighbor Discovery reach a node. A node joins them with MLD, and it leaves a solicited-node group only when no address of it maps there.
- **Checks** — core: RFC4861-AR-4, AR-5, HOST-8, ADV-3. Supporting: RFC4861-AR-6, AR-7, RFC4862-DAD-11, DAD-14, DAD-15, DAD-16, DAD-17.

## ND-F-ROUTER-SOLICITATION

**A host that comes up asks for Router Advertisements with a few Router Solicitations to the all-routers group.**

- **Sources** — RFC 4861 §4.1, §6.3.7.
- **Level** — optional (reason: keyword). "a host SHOULD transmit up to MAX_RTR_SOLICITATIONS Router Solicitation messages", RFC4861-HOST-42: a host may also wait for the periodic advertisements. The MUST NOT of RFC4861-RS-11 holds once a solicitation is sent, a conditional keyword.
- **Description** — The host waits a random time of up to MAX_RTR_SOLICITATION_DELAY, sends up to MAX_RTR_SOLICITATIONS solicitations RTR_SOLICITATION_INTERVAL apart, and stops when a Router Advertisement arrives. A solicitation from the unspecified address carries no link-layer address option; one from an assigned address carries it.
- **Checks** — core: RFC4861-HOST-42, HOST-44, HOST-45, HOST-47, HOST-50, HOST-52, RS-3, RS-4, RS-11, RS-12. Supporting: RFC4861-RS-1, HOST-43, HOST-46, HOST-48, HOST-49, HOST-51.

## ND-F-ROUTER-ADVERTISEMENT

**A router advertises itself, its parameters and its prefixes from its link-local address, to all nodes or to the soliciting host.**

- **Sources** — RFC 4861 §4.2, §6.2.1 to §6.2.3, §6.2.6.
- **Level** — mandatory (reason: keyword). RFC4861-RA-3 has the strength "must".
- **Description** — Each field of the advertisement carries a router variable: Cur Hop Limit, the M and O flags, Router Lifetime, Reachable Time and Retrans Timer in milliseconds, the link-layer address, the MTU, and one Prefix Information option for each advertised prefix with its flags and lifetimes. A router answers a valid solicitation on an advertising interface.
- **Checks** — core: RFC4861-RA-3, RA-4, RVAL-11, ADV-1, ADV-4, ADV-5, ADV-6, ADV-7, ADV-8, ADV-9, ADV-11, ADV-12, ADV-13, ADV-14, ADV-15, ADV-16, ADV-29, RA-19, RA-21, RA-27. Supporting: RFC4861-RA-1, RA-9, RA-10, RA-11, RA-12, RA-15, RA-20, RA-22, RA-23, RA-24, RA-25, RA-26, ADV-10, ADV-17, ADV-18, ADV-19, ADV-20, ADV-21, ADV-30, ADV-37, ADV-38, ADV-39, ADV-40, ADV-41, RCFG-1, RCFG-2, RCFG-3, RCFG-4, RCFG-5, RCFG-6, RCFG-7, RCFG-8, RCFG-9, RCFG-10, RCFG-11, RCFG-12, RCFG-13, RCFG-14, RCFG-15, RCFG-16, RCFG-17, RCFG-18, RCFG-19, RCFG-20, RCFG-21, RCFG-22, RCFG-23, OPT-26, OPT-27, OPT-28, OPT-34.

## ND-F-ADVERTISEMENT-TIMING

**Unsolicited Router Advertisements come at a random interval between MinRtrAdvInterval and MaxRtrAdvInterval, faster at the start, and never closer than MIN_DELAY_BETWEEN_RAS.**

- **Sources** — RFC 4861 §6.2.4, §6.2.6.
- **Level** — mandatory (reason: keyword). RFC4861-ADV-32 has the strength "must".
- **Description** — The first MAX_INITIAL_RTR_ADVERTISEMENTS advertisements come at most MAX_INITIAL_RTR_ADVERT_INTERVAL apart. An answer to a solicitation waits a random time up to MAX_RA_DELAY_TIME, and multicast advertisements keep MIN_DELAY_BETWEEN_RAS apart.
- **Checks** — core: RFC4861-ADV-22, ADV-23, ADV-32, ADV-33, ADV-36. Supporting: RFC4861-ADV-31, ADV-34, ADV-35.

## ND-F-ROUTER-DISCOVERY

**A host keeps a Default Router List from the Router Lifetime of the advertisements, and it sends off-link traffic to a router of that list.**

- **Sources** — RFC 4861 §4.2, §6.3.4 to §6.3.6.
- **Level** — mandatory (reason: only path). The Default Router List is the only way the document gives a host to reach an off-link destination.
- **Description** — A new router with a non-zero lifetime enters the list; a later advertisement renews it; lifetime zero or an expired lifetime removes it. A router with Router Lifetime zero is never a default router. A host selects a reachable router first and turns to round-robin when none is known to be reachable.
- **Checks** — core: RFC4861-HOST-12, HOST-13, HOST-14, RA-17, HOST-37, HOST-39. Supporting: RFC4861-RA-15, RA-16, RA-18, HOST-1, HOST-9, HOST-10, HOST-11, HOST-15, HOST-20, HOST-21, HOST-22, HOST-23, HOST-38, HOST-40, HOST-41.

## ND-F-PARAMETER-DISCOVERY

**A host adopts the hop limit, the reachable time, the retransmission timer and the MTU that a router advertises.**

- **Sources** — RFC 4861 §6.3.2, §6.3.4; RFC 4862 §5.6.
- **Level** — optional (reason: keyword). Every core statement is a SHOULD: "the host SHOULD set its CurHopLimit variable to the received value".
- **Description** — A non-zero Cur Hop Limit, Reachable Time or Retrans Timer replaces the value of the host; zero means that the router does not specify it. An MTU option within the bounds of the link sets LinkMTU.
- **Checks** — core: RFC4861-HOST-16, HOST-17, HOST-19, HOST-24. Supporting: RFC4861-HOST-2, HOST-3, HOST-4, HOST-5, HOST-6, HOST-7, HOST-18, OPT-49, RFC4862-CONS-1, CONS-2.

## ND-F-ON-LINK-DETERMINATION

**A host treats as on-link exactly the addresses that an advertised on-link prefix or a Redirect covers, and sends everything else to a router.**

- **Sources** — RFC 4861 §4.6.2, §6.3.4, §6.3.5; RFC 5942 §4, §6.
- **Level** — mandatory (reason: keyword). RFC5942-ONLINK-1 has the strength "must not".
- **Description** — A Prefix Information option with the L flag set puts its prefix into the Prefix List for its Valid Lifetime, a new advertisement renews it, and lifetime zero removes it. RFC 5942 overrides RFC 4861: the assignment of an address, a Neighbor Advertisement or another ND message does not make an address on-link, and a host with no router and no on-link prefix does not assume that every destination is on-link.
- **Checks** — core: RFC4861-HOST-25, HOST-30, HOST-31, HOST-32, HOST-36, RFC5942-ONLINK-1, ONLINK-2, ONLINK-3, ONLINK-5, ONLINK-6, ONLINK-9, ONLINK-10. Supporting: RFC4861-OPT-18, OPT-19, OPT-20, OPT-24, OPT-25, OPT-35, HOST-26, HOST-27, HOST-28, HOST-29, HOST-33, HOST-34, HOST-35, RFC5942-ONLINK-4, ONLINK-7, ONLINK-8.

## ND-F-ADDRESS-RESOLUTION

**A node finds the link-layer address of an on-link neighbor with a multicast Neighbor Solicitation, and the neighbor answers with a solicited Neighbor Advertisement.**

- **Sources** — RFC 4861 §4.3, §4.4, §7.2.2 to §7.2.5.
- **Level** — mandatory (reason: only path). Address resolution is the only way the document gives a node to reach an on-link neighbor.
- **Description** — The solicitation goes to the solicited-node address of the target and carries the link-layer address of the sender. The target creates a STALE entry for the sender and answers by unicast with its own link-layer address, the Solicited flag set, the Override flag set and the Router flag of its role. The sender records the address and sends the packets it queued.
- **Checks** — core: RFC4861-AR-8, AR-9, AR-12, AR-18, AR-24, AR-31, AR-32, AR-34, AR-35, AR-37, AR-39, AR-44, AR-45, NS-5, NS-15, NA-4, NA-12, NA-24. Supporting: RFC4861-AR-1, AR-2, AR-3, AR-10, AR-11, AR-13, AR-23, AR-25, AR-26, AR-27, AR-28, AR-29, AR-30, AR-33, AR-38, AR-41, AR-42, AR-43, AR-46, AR-47, AR-48, AR-49, AR-50, AR-51, AR-52, AR-53, AR-54, AR-55, AR-56, NS-1, NS-2, NS-4, NS-12, NS-13, NS-14, NS-16, NA-1, NA-3, NA-10, NA-11, NA-13, NA-14, NA-15, NA-16, NA-18, NA-21, NA-23, NA-25.

## ND-F-RESOLUTION-FAILURE

**A node repeats an unanswered solicitation every RetransTimer, keeps a small queue, and gives up after MAX_MULTICAST_SOLICIT tries with a Destination Unreachable for the queued packets.**

- **Sources** — RFC 4861 §7.2.2.
- **Level** — mandatory (reason: keyword). RFC4861-AR-20 has the strength "must".
- **Description** — While address resolution waits, the node holds at least one packet for the neighbor and replaces the oldest when the queue overflows. After MAX_MULTICAST_SOLICIT solicitations without an answer, it returns ICMP Destination Unreachable, code 3, for each queued packet.
- **Checks** — core: RFC4861-AR-19, AR-20, AR-21, AR-22. Supporting: RFC4861-AR-14, AR-15, AR-16, AR-17.

## ND-F-UNSOLICITED-ADVERTISEMENT

**A node whose link-layer address changes may tell its neighbors with a few unsolicited Neighbor Advertisements to all nodes.**

- **Sources** — RFC 4861 §4.4, §7.2.6.
- **Level** — optional (reason: keyword). A node MAY send the unsolicited advertisements, RFC4861-AR-57; the MUST of their spacing, AR-58, holds once it sends them, a conditional keyword.
- **Description** — The advertisements carry the new link-layer address, the Solicited flag clear and the Router flag of the role of the sender, at least RetransTimer apart. A neighbor sets its entry to STALE.
- **Checks** — core: RFC4861-AR-57, AR-58, AR-59, AR-60, AR-61. Supporting: RFC4861-AR-62, AR-63, AR-64, AR-65, AR-66, NA-5, NA-22.

## ND-F-ANYCAST-AND-PROXY

**Anycast and proxied addresses are answered with the Override flag clear and after a random delay.**

- **Sources** — RFC 4861 §7.2.7, §7.2.8.
- **Level** — optional (reason: keyword). A node needs no anycast address and a router need not be a proxy; the MUST of the Override flag holds only when one is, a conditional keyword.
- **Description** — A node that holds an anycast address, and a router that acts as a proxy, answer a solicitation for that address like any other, but with the Override flag clear and after a random delay of up to MAX_ANYCAST_DELAY_TIME, so that one answer does not overwrite another.
- **Checks** — core: RFC4861-ANY-2, ANY-3, ANY-8. Supporting: RFC4861-ANY-1, ANY-4, ANY-5, ANY-6, ANY-7, ANY-9, ANY-10, AR-36, AR-40, AR-67, AR-68, NA-17.

## ND-F-REDIRECT

**A router that forwards a packet to a better first hop on the same link tells the sender with a Redirect.**

- **Sources** — RFC 4861 §4.5, §4.6.3, §8, §8.2.
- **Level** — optional (reason: keyword). "A router SHOULD send a redirect message", RFC4861-RDR-3; the rules of the content hold once a Redirect is sent, conditional keywords.
- **Description** — The Redirect goes from the link-local address of the router to the source of the packet. Its Target Address is the link-local address of the better router, or the destination itself when that is on-link; its Destination Address is the destination of the packet; it carries the link-layer address of the target when the router knows it, and as much of the packet as fits. The router limits the rate of its Redirects.
- **Checks** — core: RFC4861-RDR-3, RDR-4, RDR-5, RDR-6, RDM-3, RDM-4, RDM-11, RDM-12, RDM-13, RDM-14. Supporting: RFC4861-RDR-1, RDR-2, RDR-7, RDR-8, RDR-9, RDR-10, RDM-1, RDM-15, RDM-16, RDM-17, RDVAL-2, OPT-41, OPT-42, OPT-43, ADV-46.

## ND-F-REDIRECT-PROCESSING

**A host that receives a valid Redirect sends later traffic for the destination to the target.**

- **Sources** — RFC 4861 §8.3.
- **Level** — optional (reason: keyword). "A host receiving a valid redirect SHOULD update its Destination Cache", RFC4861-RDH-1; the MUST of RDH-6 holds once the host acts on a Redirect, a conditional keyword.
- **Description** — The host updates or creates the Destination Cache entry of the destination, records the link-layer address of the target when the Redirect carries it, treats the target as on-link when it equals the destination, and marks it a router when it does not. A host never sends a Redirect.
- **Checks** — core: RFC4861-RDH-1, RDH-2, RDH-3, RDH-6, RDH-7. Supporting: RFC4861-RDH-4, RDH-5, RDH-8, RDH-9, RDH-10.

## ND-F-MESSAGE-VALIDATION

**A node silently discards a Neighbor Discovery message that fails the validity checks, and ignores reserved fields and unknown options.**

- **Sources** — RFC 4861 §4, §4.6, §6.1, §8.1; RFC 4862 §5.4.1.
- **Level** — mandatory (reason: keyword). RFC4861-RVAL-2 has the strength "must".
- **Description** — The checks cover the hop limit of 255, the checksum, code 0, the length, options of length zero, a link-local source for Router Advertisements and Redirects, a link-layer address option with an unspecified source, and the Redirect rules on the current first hop and the target. Options of other messages and unknown options are ignored.
- **Checks** — core: RFC4861-RVAL-2, RVAL-10, RVAL-12, RDVAL-1, RDVAL-3, RDVAL-7, OPT-6. Supporting: RFC4861-RVAL-1, RVAL-3, RVAL-4, RVAL-5, RVAL-6, RVAL-7, RVAL-8, RVAL-9, RVAL-13, RVAL-14, RVAL-15, RVAL-16, RVAL-17, RVAL-18, RDVAL-4, RDVAL-5, RDVAL-6, RDVAL-8, RDVAL-9, RDVAL-10, RDVAL-11, RDVAL-12, RDVAL-13, RDVAL-14, RS-10, RS-13, RA-14, RA-28, NS-11, NS-17, NA-20, NA-26, RDM-10, OPT-13, OPT-23, OPT-30, OPT-33, OPT-36, OPT-48, OPT-50, RFC4862-DAD-10.

## ND-F-ROUTER-ROLE-CHANGE

**A router that stops advertising, changes its link-local address or becomes a host tells the hosts with final advertisements.**

- **Sources** — RFC 4861 §6.2.2, §6.2.4, §6.2.5, §6.2.8.
- **Level** — optional (reason: keyword). The final advertisements are a SHOULD, RFC4861-ADV-25; the MUSTs of ADV-27 and ADV-28 hold once a router changes its role, which nothing demands, conditional keywords.
- **Description** — An interface becomes an advertising interface at startup or by configuration. When it stops, the router sends final advertisements with Router Lifetime zero; a router that becomes a host leaves the all-routers group and clears the Router flag of its advertisements.
- **Checks** — core: RFC4861-ADV-25, ADV-27, ADV-28. Supporting: RFC4861-ADV-2, ADV-24, ADV-26, ADV-47.

## ND-F-ROUTER-CONSISTENCY

**A router checks the advertisements of other routers on the link and logs the values that conflict with its own.**

- **Sources** — RFC 4861 §6.2.7.
- **Level** — optional (reason: keyword). Every core statement says should or may.
- **Description** — The check covers the hop limit, the flags, the times, the MTU and the prefix lifetimes; it allows the difference that decrementing lifetimes cause, and it does not log zero values or different prefix sets.
- **Checks** — core: RFC4861-ADV-42, ADV-43. Supporting: RFC4861-ADV-44, ADV-45.

## ND-F-LINK-LOCAL-ADDRESS

**A node forms a link-local address from the prefix FE80::/64 and its interface identifier whenever an interface becomes enabled.**

- **Sources** — RFC 4862 §5.3.
- **Level** — mandatory (reason: only path). The link-local address is the source of every Router Solicitation and Router Advertisement of the node.
- **Description** — The address has infinite lifetimes. It is formed at startup, after a failure of the interface and on a first attachment to a link, and it goes through Duplicate Address Detection before it is assigned.
- **Checks** — core: RFC4862-LL-2, LL-5, LL-7. Supporting: RFC4862-LL-1, LL-3, LL-4, LL-6, CONF-1.

## ND-F-DUPLICATE-ADDRESS-DETECTION

**Before a node assigns a unicast address, it holds the address tentative and sends Neighbor Solicitations from the unspecified address for it; a solicitation or an advertisement from another node makes it a duplicate.**

- **Sources** — RFC 4862 §5.1, §5.4; RFC 4861 §4.3, §7.2.4.
- **Level** — mandatory (reason: keyword). RFC4862-DAD-2 has the strength "must".
- **Description** — The node sends DupAddrDetectTransmits solicitations, RetransTimer apart, to the solicited-node address of the tentative address, and assigns the address when nothing contradicts it within RetransTimer after the last one. It never answers a solicitation for a tentative address. On a duplicate, it does not assign the address, logs it, and, for a link-local address from the hardware address, disables IP on the interface.
- **Checks** — core: RFC4862-DAD-2, DAD-6, DAD-9, DAD-11, DAD-12, DAD-13, DAD-20, DAD-21, DAD-25, DAD-27. Supporting: RFC4862-CONF-2, CONF-3, CONF-4, CONF-5, DAD-1, DAD-3, DAD-4, DAD-5, DAD-7, DAD-8, DAD-18, DAD-19, DAD-22, DAD-23, DAD-24, DAD-26, DAD-28, DAD-29, DAD-30.

## ND-F-STATELESS-AUTOCONFIGURATION

**A host forms a global address from each advertised prefix with the A flag set and its interface identifier, with the lifetimes of the prefix.**

- **Sources** — RFC 4862 §5.5.1 to §5.5.3; RFC 4861 §4.6.2.
- **Level** — mandatory (reason: keyword). RFC4862-GLOB-9 has the strength "must (the ignore), may (the log)".
- **Description** — The host ignores a prefix with the A flag clear, the link-local prefix, a preferred lifetime above the valid lifetime, and a prefix whose length does not leave room for the interface identifier. A known prefix renews the preferred lifetime and, under the two-hour rule, the valid lifetime.
- **Checks** — core: RFC4862-GLOB-1, GLOB-5, GLOB-6, GLOB-7, GLOB-8, GLOB-9, GLOB-11, GLOB-12, GLOB-13, GLOB-15, RFC4861-OPT-21. Supporting: RFC4862-GLOB-2, GLOB-3, GLOB-4, GLOB-10, GLOB-14, CONF-6.

## ND-F-ADDRESS-LIFETIME

**An autoconfigured address becomes deprecated when its preferred lifetime ends and invalid when its valid lifetime ends.**

- **Sources** — RFC 4862 §5.5.4, §5.7.
- **Level** — mandatory (reason: keyword). RFC4862-GLOB-23 has the strength "must not".
- **Description** — A deprecated address still serves the communications it serves, and new ones when no alternative exists; an invalid address is never a source and is not accepted as a destination.
- **Checks** — core: RFC4862-GLOB-16, GLOB-18, GLOB-22, GLOB-23, GLOB-24. Supporting: RFC4862-GLOB-17, GLOB-19, GLOB-20, GLOB-21, CONS-3.

## ND-F-NO-FRAGMENTATION

**Neighbor Discovery messages are never fragmented, and a node ignores one that arrives fragmented.**

- **Sources** — RFC 6980 §5.
- **Level** — mandatory (reason: keyword). RFC6980-FRAG-1 has the strength "must not".
- **Description** — The rule covers all five messages. A sender does not fragment them, and a receiver silently ignores a message in a packet with a Fragment header.
- **Checks** — core: RFC6980-FRAG-1, FRAG-2, FRAG-3, FRAG-4, FRAG-5, FRAG-6.

## Coverage of the catalog entries

Every one of the 489 entries of the four catalogs is in at least one feature above, as core
or as supporting: RFC 4861 with 403, RFC 4862 with
70, RFC 5942 with 10 and RFC 6980 with
6.
