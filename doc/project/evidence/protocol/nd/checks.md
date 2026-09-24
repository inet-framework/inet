# ND — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc4861/catalog.md](../../standard/rfc4861/catalog.md), [rfc4862/catalog.md](../../standard/rfc4862/catalog.md), [rfc5942/catalog.md](../../standard/rfc5942/catalog.md), [rfc6980/catalog.md](../../standard/rfc6980/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, and the index. The checks themselves live in one file per feature
group under [`checks/`](checks); the section names are the anchors that the coverage ledger
links to.

## Common mockups

Every link is an Ethernet link with an MTU of 1500 octets. The routers are configured with an
advertised prefix for each link, as below; the hosts configure themselves: a link-local
address from their interface identifier, and a global address from each advertised prefix.
Where a check sets a router variable to a value other than its default, the check says so.

| Link | Prefix |
| --- | --- |
| L1 | 2001:db8:1::/64 |
| L2 | 2001:db8:2::/64 |

**The link.** Router R and hosts A and B on L1, through a switch.

```
        R
        |
  ------+------ L1
    |       |
    A       B
```

**The path.** Host A on L1, router R between L1 and L2, host C on L2.

```
A --L1-- R --L2-- C
```

**The two routers.** Routers R1 and R2 and host A on L1; R2 also on L2 with host C. R1 is the
default router of A, because R2 advertises Router Lifetime 0, and R1 reaches L2 through R2.

```
  R1      R2 --L2-- C
   |       |
 --+---+---+-- L1
       |
       A
```

**The link without a router.** Hosts A and B on L1, with no router.

**The link with a duplicate.** The link, where hosts A and B have the same link-layer address, so
that the interface identifiers — and the link-local addresses formed from them — are the same.

The interface identifier of a node comes from its 48-bit link-layer address in the modified
EUI-64 form, so that a link-local address is `fe80::` followed by it, and a global address is the
prefix followed by it.

## Rules every check obeys

- **An observation names the link and the sender.** "On L1, from A" means the frames that A
  sends onto L1, as they leave A. The ND message is read from the frame; the addresses and the
  hop limit from the IPv6 header, the destination link-layer address from the Ethernet header.
- **One observation confirms the stimulus.** Each procedure has an observation that shows the
  situation the rule needs — the host came up, the router advertised, the link-layer address was
  resolved — before the observation that checks the rule.
- **A random interval of the standard is a window, not a value.** Where the standard gives a
  random delay, the check tests its bounds and never a value drawn from it.
- **Start instants are fixed where a timer is under test**, so that the instant under test does
  not fall on another event of the same node by chance.
- **The fields of the header come last** in a check that also observes behavior, so that a wrong
  field does not keep the behavior from a verdict; a check never puts two rules into one
  observation when one could hide the other.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Router Solicitation of a host that comes up](checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `checks/router-discovery.md` | RFC4861-HOST-44, HOST-45, RS-3, RS-4, RS-5, RS-6, RS-7, RS-9, HOST-50, RS-11, RS-12, HOST-46, HOST-51; covers RFC4861-RS-2, OPT-11, HOST-43 |
| [Router Solicitations on a link without a router](checks/router-discovery.md#router-solicitations-on-a-link-without-a-router) | `checks/router-discovery.md` | RFC4861-HOST-42, HOST-52, RFC4862-GLOB-4 |
| [Router Advertisement header and addressing](checks/router-discovery.md#router-advertisement-header-and-addressing) | `checks/router-discovery.md` | RFC4861-RA-3, RA-13, RA-1, RA-4, RA-5, RA-6, RA-7, ADV-4, RVAL-11, ADV-21; covers RFC4862-GLOB-3 |
| [Router Advertisement fields](checks/router-discovery.md#router-advertisement-fields) | `checks/router-discovery.md` | RFC4861-ADV-5, ADV-6, ADV-7, ADV-8, ADV-9, ADV-11, RA-19, RA-21, RA-23, OPT-8, OPT-9, OPT-45, OPT-46, RCFG-7, RCFG-8, RCFG-11, OPT-47, RCFG-10, RCFG-13, RCFG-12; covers RFC4861-RA-2, OPT-3, OPT-5, OPT-7, OPT-10, OPT-44, ADV-10, RCFG-9, RA-26, OPT-1, OPT-2 |
| [Prefix Information option](checks/router-discovery.md#prefix-information-option) | `checks/router-discovery.md` | RFC4861-ADV-12, ADV-13, ADV-14, ADV-15, ADV-16, OPT-15, OPT-16, RCFG-14, RCFG-16, RCFG-18, RCFG-19, RCFG-22, OPT-22, OPT-29, OPT-32, OPT-28, RCFG-21, RA-27, OPT-34, RCFG-15; covers RFC4861-OPT-14, OPT-17, OPT-31, OPT-4, RCFG-17, RCFG-20 |
| [Router Advertisement in answer to a solicitation](checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `checks/router-discovery.md` | RFC4861-ADV-29, RS-1, ADV-34, ADV-35, ADV-32, ADV-33, ADV-19; covers RFC4861-ADV-30 |
| [Unsolicited Router Advertisements](checks/router-discovery.md#unsolicited-router-advertisements) | `checks/router-discovery.md` | RFC4861-ADV-22, ADV-31, ADV-23, ADV-36, RCFG-5, RCFG-6 |
| [The default router](checks/router-discovery.md#the-default-router) | `checks/router-discovery.md` | RFC4861-HOST-12, HOST-39, RA-15 |
| [A router with Router Lifetime zero](checks/router-discovery.md#a-router-with-router-lifetime-zero) | `checks/router-discovery.md` | RFC4861-RA-17; covers RFC4861-ADV-17 |
| [Hop limit from the router](checks/parameters.md#hop-limit-from-the-router) | `checks/parameters.md` | RFC4861-HOST-16, RA-9 |
| [Hop limit without a router](checks/parameters.md#hop-limit-without-a-router) | `checks/parameters.md` | RFC4861-HOST-1, HOST-4 |
| [Retransmission timer from the router](checks/parameters.md#retransmission-timer-from-the-router) | `checks/parameters.md` | RFC4861-HOST-19 |
| [MTU from the router](checks/parameters.md#mtu-from-the-router) | `checks/parameters.md` | RFC4861-HOST-24, HOST-3, OPT-49 |
| [On-link neighbor reached directly](checks/on-link.md#on-link-neighbor-reached-directly) | `checks/on-link.md` | RFC4861-HOST-30, OPT-18, OPT-19, AR-1, RFC5942-ONLINK-2, RFC4861-HOST-25 |
| [Prefix with the on-link flag clear](checks/on-link.md#prefix-with-the-on-link-flag-clear) | `checks/on-link.md` | RFC5942-ONLINK-1, RFC4861-HOST-26, RFC4861-HOST-28, RFC5942-ONLINK-8 |
| [No router and no on-link prefix](checks/on-link.md#no-router-and-no-on-link-prefix) | `checks/on-link.md` | RFC5942-ONLINK-5, ONLINK-6 |
| [Prefix after its valid lifetime](checks/on-link.md#prefix-after-its-valid-lifetime) | `checks/on-link.md` | RFC5942-ONLINK-3, RFC4861-HOST-36, OPT-24; covers RFC4861-HOST-34 |
| [Address resolution of a host](checks/address-resolution.md#address-resolution-of-a-host) | `checks/address-resolution.md` | RFC4861-AR-8, AR-9, AR-18, AR-31, AR-32, AR-41, AR-44, NS-1, NS-2, NS-5, NS-12, NA-1, NA-4, NA-21, AR-12, AR-14, AR-34, NS-15, NA-24, AR-15, AR-10, AR-24; covers RFC4861-AR-30, RFC4862-DAD-18, DAD-26 |
| [Neighbor Solicitation and Advertisement fields](checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `checks/address-resolution.md` | RFC4861-NS-4, NS-6, NS-7, NS-8, NA-3, NA-6, NA-7, NA-8, NA-12, NS-10, NA-19, AR-35, AR-39, NS-13, NA-23, AR-37, NA-18; covers RFC4861-NS-3, NA-2, AR-2, AR-53 |
| [Address resolution of a router](checks/address-resolution.md#address-resolution-of-a-router) | `checks/address-resolution.md` | RFC4861-NA-10; covers RFC4861-AR-35 |
| [Address resolution failure](checks/address-resolution.md#address-resolution-failure) | `checks/address-resolution.md` | RFC4861-AR-21, HOST-7, AR-20, AR-22, AR-23, RCFG-23, AR-19, AR-11 |
| [Redirect from the first-hop router](checks/redirect.md#redirect-from-the-first-hop-router) | `checks/redirect.md` | RFC4861-RDM-1, RDM-4, RDM-11, RDM-14, RDR-1, RDR-6, RDM-3, RDM-13, RDR-2, RDR-4, RDVAL-2, ADV-46, RDR-3 |
| [Redirect fields](checks/redirect.md#redirect-fields) | `checks/redirect.md` | RFC4861-RDM-5, RDM-6, RDM-7, RDR-7, OPT-12, OPT-38, OPT-39, RDM-9, OPT-40, RDM-15; covers RFC4861-RDM-2, OPT-37 |
| [Redirect of a large packet](checks/redirect.md#redirect-of-a-large-packet) | `checks/redirect.md` | RFC4861-RDM-17, RDR-8, OPT-42, RFC6980-FRAG-1 |
| [Host follows a Redirect](checks/redirect.md#host-follows-a-redirect) | `checks/redirect.md` | RFC4861-RDH-1, RDH-2, RDH-10 |
| [Redirect to an on-link destination](checks/redirect.md#redirect-to-an-on-link-destination) | `checks/redirect.md` | RFC4861-RDM-12, RDR-5, RDH-6 |
| [Link-local address and its Duplicate Address Detection](checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `checks/autoconfiguration.md` | RFC4862-DAD-2, DAD-6, DAD-12, DAD-13, LL-5, CONF-3, CONF-5, LL-2, CONF-4, DAD-9, RFC4861-NS-14; covers RFC4862-CONF-1 |
| [Duplicate Address Detection of a router](checks/autoconfiguration.md#duplicate-address-detection-of-a-router) | `checks/autoconfiguration.md` | RFC4862-DAD-1, LL-1 |
| [Global address and its Duplicate Address Detection](checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | `checks/autoconfiguration.md` | RFC4862-GLOB-1, GLOB-8, GLOB-11, RFC4861-OPT-21, RFC4862-DAD-5, GLOB-2 |
| [Duplicate link-local address](checks/autoconfiguration.md#duplicate-link-local-address) | `checks/autoconfiguration.md` | RFC4862-DAD-25, DAD-27, DAD-29, RFC4861-AR-38, NA-14; covers RFC4862-DAD-7 |
| [Prefix with the Autonomous flag clear](checks/autoconfiguration.md#prefix-with-the-autonomous-flag-clear) | `checks/autoconfiguration.md` | RFC4862-GLOB-5 |
| [Address after its valid lifetime](checks/autoconfiguration.md#address-after-its-valid-lifetime) | `checks/autoconfiguration.md` | RFC4862-GLOB-22, GLOB-23 |
| [Groups joined before Duplicate Address Detection](checks/multicast.md#groups-joined-before-duplicate-address-detection) | `checks/multicast.md` | RFC4862-DAD-11, RFC4861-AR-4, RFC4861-AR-6; covers RFC4862-DAD-16 |
| [All-routers group of a router](checks/multicast.md#all-routers-group-of-a-router) | `checks/multicast.md` | RFC4861-ADV-3 |

## Statements this pass wrote no check for

Level 2 asks for a core check of every mandatory feature, and the checks above give one to every
mandatory feature except message validation, whose statements all need a crafted message. The
statements below have no check in this pass. This section says **what a check would need**. It
says nothing about the simulation model: whether the absence of a check is acceptable is a
judgment about what the model claims, and that judgment lives in the coverage ledger.

**A crafted message**, the toolset of level 3: every validity check of a received message —
RFC4861-RVAL-1 to RVAL-10, RVAL-12 to RVAL-18, RDVAL-1 and RDVAL-3 to RDVAL-14, the receiver
halves of the reserved fields and the unknown options (RS-10, RS-13, RA-14, RA-28, NS-11, NS-17,
NA-20, NA-26, RDM-10, OPT-6, OPT-13, OPT-23, OPT-30, OPT-33, OPT-35, OPT-36, OPT-41, OPT-43,
OPT-48, OPT-50), and RFC4862-DAD-10; the receiver half of RFC 6980, RFC6980-FRAG-2 to FRAG-6,
which needs a fragmented ND message; RFC5942-ONLINK-9 and ONLINK-10, which need a Neighbor
Advertisement or another ND message from an address outside every on-link prefix; the Neighbor
Cache rules of an advertisement with a changed link-layer address, with the Override flag clear,
or for a target with no entry (RFC4861-NA-15, NA-16, AR-42, AR-43, AR-47 to AR-52, AR-63,
AR-64); a Router Solicitation from the unspecified address with an option (RFC4861-ADV-37 to
ADV-41); a Router Lifetime above the limit that senders obey (RFC4861-RA-16); a Prefix
Information option for the link-local prefix, which a router does not send (RFC4861-HOST-29,
RFC4862-GLOB-6); a Prefix Information option with a preferred lifetime above its valid lifetime
or a length that leaves no room for the interface identifier (RFC4862-GLOB-7, GLOB-9); a Redirect
to a host from a router that is not its first hop, and a Redirect to a router
(RFC4861-RDR-10); a packet to a tentative or an invalid address (RFC4862-DAD-8, GLOB-24).

**A distribution**, a statistical test of level 4: the random halves of the delays and intervals
of the standard. The checks above test the bounds of the delay of a solicited advertisement and of
the interval of unsolicited advertisements. The delays with no check are the delay before the
first Router Solicitation (RFC4861-HOST-47), the random ReachableTime (HOST-6, HOST-17, HOST-18),
the anycast delay (ANY-2, AR-40), and the delay of the join before Duplicate Address Detection
(RFC4862-DAD-14, DAD-15).

**State inside a node**, the toolset of level 4. The Neighbor Cache states and the IsRouter flag
(RFC4861-AR-3, AR-25 to AR-29, AR-45, AR-46, AR-55, HOST-20 to HOST-23, RDH-3 to RDH-5, RDH-7,
RDH-8), the timers and lists that no message shows until they expire (HOST-5, HOST-13;
RFC4862-CONF-6, LL-7, CONS-3), a log entry (RFC4862-DAD-28), the join of the all-nodes address, which MLD
never reports (RFC4861-HOST-8), and the error report of a host without a route, which goes to the
application inside the host (RFC5942-ONLINK-7). Neighbor Unreachability Detection, RFC 4861 §7.3,
enters at level 4 as well, and with it the unicast Neighbor Solicitations and their answers
(RFC4861-NS-16, NA-11, NA-13, NA-25, AR-13, AR-33).

**Management, other documents and other links.** Statements about the management interface and
about documents for other link layers (RFC4861-RCFG-1 to RCFG-3, HOST-2; RFC4862-CONF-2): the
checks above set router variables, but a check of these statements needs every variable. The
ICMPv6 checksum (RFC4861-RS-8, RA-8, NS-9, NA-9, RDM-8) is the checksum of ICMPv6 for every
message type, RFC 4443 §2.3, and not a rule of Neighbor Discovery. Links with a variable MTU or
without multicast (RFC4861-RA-25, RDM-16), and interface identifiers of a length other than 64
bits (RFC4862-LL-6, GLOB-10), are not in the mockups.

**A permission that no observation can fail**: a router may leave out the Source Link-Layer
Address option or some options (RFC4861-RA-24, ADV-18), a host may skip the delay before its
first Router Solicitation (HOST-48, HOST-49), and a node may keep IP on an interface whose
duplicate link-local address does not come from the hardware address (RFC4862-DAD-30).

**A larger mockup or a second message path**, work for a later level 2 pass:

- a Router Advertisement larger than the MTU, which a router splits and does not fragment
  (RFC4861-ADV-20);
- a router variable of zero, which leaves the host value as it is (RFC4861-RA-10, RA-20, RA-22,
  HOST-11), and a lifetime of all one bits, which is infinity (OPT-25, OPT-27);
- a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or
  refreshes it (RFC4861-OPT-20, HOST-27, HOST-31 to HOST-33; RFC5942-ONLINK-4); a prefix length
  other than 64 (RFC4861-HOST-35); a router whose lifetime runs out (RFC4861-HOST-37, HOST-38);
- a router with Router Lifetime zero that alone advertises a prefix (RFC4861-RA-18);
- three or more routers, for default router selection (RFC4861-HOST-40, HOST-41, HOST-15), and two
  routers that advertise different values of one parameter (HOST-9, HOST-10; RFC4862-CONS-2);
- an interface that does not advertise (RFC4861-RCFG-4, ADV-1);
- a change of the link-layer address of a node, for unsolicited advertisements (RFC4861-NA-5,
  NA-22, AR-54, AR-57 to AR-68);
- an anycast address or a proxy (RFC4861-NA-17, AR-36, ANY-1 to ANY-10; RFC4862-DAD-4);
- a router that stops advertising, changes its link-local address or becomes a host
  (RFC4861-ADV-2, ADV-24 to ADV-28, ADV-47, HOST-14, AR-56), and two routers with inconsistent
  advertisements (ADV-42 to ADV-45);
- a burst of packets to one neighbor that is not resolved yet (RFC4861-AR-16, AR-17), a stream of
  packets that causes Redirects, for the rate limit (RDR-9), and two flows to one destination with
  different Flow Labels (RDH-9);
- an address with an interface identifier of its own, which has its own solicited-node group, and
  the removal of an address (RFC4861-AR-5, AR-7);
- a deprecated address with an alternative (RFC4861-OPT-26; RFC4862-GLOB-16 to GLOB-21), and the
  renewal of a known prefix under the two-hour rule (GLOB-12 to GLOB-15);
- an interface that fails and comes back, for the link-local address formed again
  (RFC4862-LL-3, LL-4);
- other values of DupAddrDetectTransmits (RFC4862-DAD-3);
- two nodes that test one address at the same time, or a node that sees its own solicitation
  (RFC4862-DAD-17, DAD-19 to DAD-24), and a duplicate global address;
- the M and O flags with DHCPv6 (RFC4861-RA-11, RA-12; RFC4862-CONS-1).

The statements that no check targets because a check above shows them are in the ledger as
`covered`, with the check that shows them.
