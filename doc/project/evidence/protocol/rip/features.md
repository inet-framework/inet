# RIP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RIP-F-*` · **Stands on:** [standards.md](standards.md), [rfc2453/catalog.md](../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../standard/rfc2080/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/rip/coverage.md), and step 8 compares it with the claims of
  the model in [`conformance.md`](../../model/rip/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/rip/coverage.md).

## One map for two documents

RFC 2453 defines RIP version 2 for IPv4 and RFC 2080 defines RIPng for IPv6. The two texts
state the same mechanisms in parallel sections, often in the same words; the table in
[`standards.md`](standards.md#document-list) maps the sections. So each feature below names
the statements of both documents, and a feature is one mechanism of the protocol, not of one
address family. Where the two documents differ, the feature says so:

- the port (520 and 521), the multicast group (224.0.0.9 and FF02::9) and the destination
  (an address with a subnet mask, and a prefix with a prefix length);
- the entry limit (25 entries, and as many as the MTU allows);
- the next hop (a field in each entry, and a separate entry for the entries after it);
- the source of an update (any address on the network, and a link-local address) and the hop
  limit of 255, which only RFC 2080 demands;
- the triggered update for a new route (a permission in RFC 2453, and a plain rule in
  RFC 2080: "Whenever the metric for a route is changed, an update is triggered");
- the random offset of the periodic update (up to 5 seconds, and up to 15 seconds);
- the host routes, the interworking with version 1 and the queries of version 1, which exist
  in RFC 2453 only.

## How this map decides the level

The rule of the guide: `mandatory` when any core statement says `must` or `shall`, or when
the mechanism is the only path the document gives to a state or an outcome; `optional` when
every core statement says `may` or `should`; `unstated` otherwise. Neither document uses the
keywords of RFC 2119; the rule reads the lowercase words.

One refinement, which the DHCP map states first: **a conditional keyword does not raise the
level of a feature.** RFC 2453 lets an implementation leave out host routes ("may choose not
to support host routes") and then says that one which does "must ignore any received host
routes". The `must` holds only for an implementation that has used the permission. The level
comes from the statement that decides whether the mechanism has to exist at all, so
[RIP-F-HOST-ROUTES](#rip-f-host-routes) is `optional`. Two other features have a `must`
that is conditional in the same way: [RIP-F-NEXT-HOP](#rip-f-next-hop) is `optional` and
[RIP-F-TABLE-REQUEST](#rip-f-table-request) is `unstated`, and each names the statement that
its level rests on.

## Index

| ID | Feature |
| --- | --- |
| [RIP-F-MESSAGE-FORMAT](#rip-f-message-format) | Every message is a header — command, version, two zero octets — and a list of 20-octet route entries. |
| [RIP-F-TRANSPORT](#rip-f-transport) | RIP runs over UDP: port 520 for version 2 and port 521 for RIPng, with fixed rules for the source and the destination port. |
| [RIP-F-UPDATE-ADDRESSING](#rip-f-update-addressing) | A regular or triggered update goes to the RIP multicast group of each directly-connected network; a RIPng update leaves from a link-local address with hop limit 255. |
| [RIP-F-METRIC](#rip-f-metric) | A route has a metric from 1 to 15, 16 means unreachable, and each hop adds the cost of the network it crosses. |
| [RIP-F-DESTINATION-PREFIX](#rip-f-destination-prefix) | Each route names its destination with a subnet mask or a prefix length, and the entry carries it. |
| [RIP-F-PERIODIC-UPDATE](#rip-f-periodic-update) | Every 30 seconds, with an offset against synchronization, a router sends its whole table to every neighbor. |
| [RIP-F-ROUTE-LEARNING](#rip-f-route-learning) | A router adds a new route from a response, adopts a better one or the news of its current next hop, and ignores the rest. |
| [RIP-F-SPLIT-HORIZON](#rip-f-split-horizon) | A route is not advertised back with a finite metric onto the network it was learned from; poisoned reverse sends it there with metric 16. |
| [RIP-F-TRIGGERED-UPDATE](#rip-f-triggered-update) | A change of a route is sent at once, without waiting for the next periodic update, holding at least the changed routes, and at a limited rate. |
| [RIP-F-ROUTE-EXPIRY](#rip-f-route-expiry) | A route that is not refreshed for 180 seconds, or that its next hop withdraws, is advertised with metric 16 for 120 seconds and then removed; a new route in that time replaces it. |
| [RIP-F-RESPONSE-CONTENTS](#rip-f-response-contents) | A response holds every route that split horizon lets through, those with metric 16 included, split into messages of at most 25 entries or of the MTU, and never a route to a link-local address. |
| [RIP-F-RESPONSE-VALIDATION](#rip-f-response-validation) | A router ignores a response that is not from its RIP port, not from a neighbor, or from itself, and each entry whose destination or metric is invalid. |
| [RIP-F-TABLE-REQUEST](#rip-f-table-request) | A router that comes up asks its neighbors for their whole tables, and a router answers such a request directly to the requester, after split horizon. |
| [RIP-F-SPECIFIC-QUERY](#rip-f-specific-query) | A request that names destinations gets their metrics back as they are, without split horizon; an empty request gets nothing. |
| [RIP-F-NEXT-HOP](#rip-f-next-hop) | An update can name a better next hop than its sender; the next hop must be directly reachable, and zero means the sender. |
| [RIP-F-ROUTE-TAG](#rip-f-route-tag) | A route keeps the route tag it arrives with, and the tag is advertised with it. |
| [RIP-F-DEFAULT-ROUTE](#rip-f-default-route) | A default route — 0.0.0.0, or a prefix of length zero — travels through RIP like any other route. |
| [RIP-F-HOST-ROUTES](#rip-f-host-routes) | A router supports host routes, or it ignores the host routes it receives. |
| [RIP-F-VERSION-1-INTERWORKING](#rip-f-version-1-interworking) | Where a version 1 router takes part, the version 2 router hides what version 1 would misread, and it answers a version 1 request in version 1. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [RIP-F-MESSAGE-FORMAT](#rip-f-message-format) | mandatory | RFC 2453 §3.6, §3.10.2, §4; RFC 2080 §2.1, §2.5.2 | RFC2453-MSG-9, MSG-10, MSG-12, GEN-2; RFC2080-MSG-8, GEN-3 |
| [RIP-F-TRANSPORT](#rip-f-transport) | mandatory | RFC 2453 §3.6; RFC 2080 §2.1 | RFC2453-MSG-1, MSG-2, MSG-3, MSG-4; RFC2080-MSG-1, MSG-2, MSG-3, MSG-4 |
| [RIP-F-UPDATE-ADDRESSING](#rip-f-update-addressing) | mandatory | RFC 2453 §3.10, §4.5; RFC 2080 §2.4.2, §2.5, §2.5.2 | RFC2453-MCAST-1, OUT-2; RFC2080-OUT-3, GEN-1, RESP-5 |
| [RIP-F-METRIC](#rip-f-metric) | mandatory | RFC 2453 §3.5, §3.6, §3.9.2; RFC 2080 §2, §2.1, §2.4.2 | RFC2453-MET-3, MET-4, MET-5; RFC2080-MET-3, MET-4, MET-5 |
| [RIP-F-DESTINATION-PREFIX](#rip-f-destination-prefix) | mandatory | RFC 2453 §3.5, §4.3; RFC 2080 §2.1 | RFC2453-TABLE-2, MASK-1; RFC2080-MSG-10 |
| [RIP-F-PERIODIC-UPDATE](#rip-f-periodic-update) | mandatory | RFC 2453 §3.8; RFC 2080 §2.3, §2.5 | RFC2453-TIMER-1, TIMER-2; RFC2080-TIMER-1, TIMER-2, OUT-2 |
| [RIP-F-ROUTE-LEARNING](#rip-f-route-learning) | mandatory | RFC 2453 §3.9.2; RFC 2080 §2.4.2 | RFC2453-RESP-6, RESP-7, RESP-9, RESP-12, TABLE-3; RFC2080-RESP-8, RESP-9, RESP-11, RESP-14, TABLE-2 |
| [RIP-F-SPLIT-HORIZON](#rip-f-split-horizon) | mandatory | RFC 2453 §3.4.3; RFC 2080 §2.6, §2.4.1 | RFC2453-SH-1, SH-2; RFC2080-SH-1, SH-2 |
| [RIP-F-TRIGGERED-UPDATE](#rip-f-triggered-update) | mandatory | RFC 2453 §3.4.4, §3.10.1; RFC 2080 §2.5, §2.5.1 | RFC2453-TRIG-1, TRIG-2, TRIG-5, TRIG-9; RFC2080-OUT-2, TRIG-1, TRIG-4, TRIG-8 |
| [RIP-F-ROUTE-EXPIRY](#rip-f-route-expiry) | mandatory | RFC 2453 §3.8; RFC 2080 §2.3 | RFC2453-TIMER-4, TIMER-5, TIMER-6, TIMER-7; RFC2080-TIMER-4, TIMER-5, TIMER-6, TIMER-7 |
| [RIP-F-RESPONSE-CONTENTS](#rip-f-response-contents) | mandatory | RFC 2453 §3.6, §3.10.2; RFC 2080 §2.1, §2.5.2 | RFC2453-GEN-3, GEN-5, MSG-7; RFC2080-GEN-4, GEN-5, GEN-7, MSG-11 |
| [RIP-F-RESPONSE-VALIDATION](#rip-f-response-validation) | mandatory | RFC 2453 §3.9.2; RFC 2080 §2.4.2 | RFC2453-RESP-2, RESP-3, RESP-4, RESP-5; RFC2080-RESP-2, RESP-3, RESP-4, RESP-6, RESP-7 |
| [RIP-F-TABLE-REQUEST](#rip-f-table-request) | unstated | RFC 2453 §3.9.1, §3.10; RFC 2080 §2.4.1, §2.5 | RFC2453-REQ-4, REQ-6, OUT-1; RFC2080-REQ-4, REQ-6, OUT-1 |
| [RIP-F-SPECIFIC-QUERY](#rip-f-specific-query) | unstated | RFC 2453 §3.9.1; RFC 2080 §2.4.1 | RFC2453-REQ-2, REQ-3, REQ-5, REQ-7; RFC2080-REQ-2, REQ-3, REQ-5, REQ-7 |
| [RIP-F-NEXT-HOP](#rip-f-next-hop) | optional | RFC 2453 §4.4; RFC 2080 §2.1.1 | RFC2453-NH-1, NH-2; RFC2080-NH-4, NH-5 |
| [RIP-F-ROUTE-TAG](#rip-f-route-tag) | mandatory | RFC 2453 §4.2; RFC 2080 §2.1 | RFC2453-TAG-1; RFC2080-TAG-1 |
| [RIP-F-DEFAULT-ROUTE](#rip-f-default-route) | unstated | RFC 2453 §3.7; RFC 2080 §2.2 | RFC2453-ADDR-6; RFC2080-ADDR-1, ADDR-2 |
| [RIP-F-HOST-ROUTES](#rip-f-host-routes) | optional | RFC 2453 §3.7 | RFC2453-ADDR-1, ADDR-5 |
| [RIP-F-VERSION-1-INTERWORKING](#rip-f-version-1-interworking) | mandatory | RFC 2453 §3.7, §4.3, §4.6 | RFC2453-ADDR-2, ADDR-4, MASK-2, QRY-1 |

## RIP-F-MESSAGE-FORMAT

**Every message is a header — command, version, two zero octets — and a list of 20-octet route entries.**

- **Sources** — RFC 2453 §3.6, `rfc2453.txt:1096-1173`; §3.10.2, `rfc2453.txt:1646-1650`;
  §4, `rfc2453.txt:1693-1714`. RFC 2080 §2.1, `rfc2080.txt:258-324`; §2.5.2,
  `rfc2080.txt:856-858`.
- **Level** — mandatory (reason: only path). Each document gives one message format and no
  other; a router that does not use it exchanges nothing with a router that does.
- **Description** — the command is 1 for a request and 2 for a response. The version is 2 in
  RIP version 2 when the entries use the version 2 fields, and 1 in RIPng. The two octets
  after the version are zero. An entry of RIP version 2 holds an address family identifier
  of 2, a route tag, an IPv4 address, a subnet mask, a next hop and a metric; an entry of
  RIPng holds an IPv6 prefix, a route tag, a prefix length and a metric.
- **Checks** — core: RFC2453-MSG-9 (the command), RFC2453-MSG-10 (the address family 2),
  RFC2453-MSG-12 (version 2), RFC2453-GEN-2 (command 2 and the zero octets of a response),
  RFC2080-MSG-8 (the command), RFC2080-GEN-3 (version 1, command 2 and the zero octets).
  Supporting: RFC2453-MSG-6, MSG-8, MSG-11 and RFC2080-MSG-6, MSG-7, MSG-9 (the bit layout;
  their category is a serializer test).

## RIP-F-TRANSPORT

**RIP runs over UDP: port 520 for version 2 and port 521 for RIPng, with fixed rules for the source and the destination port.**

- **Sources** — RFC 2453 §3.6, `rfc2453.txt:1085-1094`. RFC 2080 §2.1,
  `rfc2080.txt:247-256`.
- **Level** — mandatory (reason: only path). Each document names one transport and one port.
- **Description** — a router sends and receives on its RIP port. A message for another router
  goes to that port; an update leaves from it; an unsolicited update has the RIP port as both
  ports; the answer to a request goes to the port the request came from.
- **Checks** — core: RFC2453-MSG-1, MSG-2, MSG-3, MSG-4; RFC2080-MSG-1, MSG-2, MSG-3, MSG-4.
  Supporting: RFC2453-MSG-5 and RFC2080-MSG-5 (a query from another port still goes to the
  RIP port).

## RIP-F-UPDATE-ADDRESSING

**A regular or triggered update goes to the RIP multicast group of each directly-connected network; a RIPng update leaves from a link-local address with hop limit 255.**

- **Sources** — RFC 2453 §3.10, `rfc2453.txt:1575-1581`; §4.5, `rfc2453.txt:1824-1839`.
  RFC 2080 §2.5, `rfc2080.txt:757-764`; §2.5.2, `rfc2080.txt:826-850`; §2.4.2,
  `rfc2080.txt:625-627`.
- **Level** — mandatory (reason: keyword). "The IPv6 source address must be a link-local
  address", `rfc2080.txt:826`, and "periodic advertisements must have their hop counts set to
  255", `rfc2080.txt:626-627`. For RIP version 2 the multicast group is the only address the
  document gives for a periodic update on a network with broadcast, `rfc2453.txt:1826-1828`.
- **Description** — one response for each directly-connected network, to 224.0.0.9 or to
  FF02::9. RIPng adds two rules for the receiver's sake: the source is a link-local address,
  because the receiver takes it as the next hop, and the hop limit is 255, because a receiver
  checks it to know that the message did not cross a router.
- **Checks** — core: RFC2453-MCAST-1 (224.0.0.9), RFC2453-OUT-2 (one response on each
  network), RFC2080-OUT-3 (FF02::9 on each network), RFC2080-GEN-1 (a link-local source),
  RFC2080-RESP-5 (hop limit 255). Supporting: RFC2453-MCAST-2 (the multicast stays on its
  link), RFC2453-MCAST-3 (a network without broadcast), RFC2453-MCAST-4 (multicast on every
  interface), RFC2080-GEN-2 (one designated link-local source).

## RIP-F-METRIC

**A route has a metric from 1 to 15, 16 means unreachable, and each hop adds the cost of the network it crosses.**

- **Sources** — RFC 2453 §3.5, `rfc2453.txt:980-984`, `rfc2453.txt:1046-1049`; §3.6,
  `rfc2453.txt:1171-1173`; §3.9.2, `rfc2453.txt:1477-1481`. RFC 2080 §2,
  `rfc2080.txt:184-188`, `rfc2080.txt:216-219`; §2.1, `rfc2080.txt:356-358`; §2.4.2,
  `rfc2080.txt:652-656`.
- **Level** — mandatory (reason: only path). The metric is the only quantity by which a
  router compares routes, and `MIN(metric + cost, infinity)` is the only rule for it.
- **Description** — a directly-connected network has the cost of that network, usually 1.
  A received metric gets the cost of the arrival network added, and the result stops at 16.
- **Checks** — core: RFC2453-MET-3 (the cost of a directly-connected network),
  RFC2453-MET-4 (the range of the field), RFC2453-MET-5 (the addition), and RFC2080-MET-3,
  MET-4, MET-5. Supporting: RFC2453-MET-1, MET-2, RFC2080-MET-1, MET-2 (the range of a
  configured cost, and a cost that the administrator sets).

## RIP-F-DESTINATION-PREFIX

**Each route names its destination with a subnet mask or a prefix length, and the entry carries it.**

- **Sources** — RFC 2453 §3.5, `rfc2453.txt:1055-1059`; §4.3, `rfc2453.txt:1781-1783`.
  RFC 2080 §2.1, `rfc2080.txt:352-354`.
- **Level** — mandatory (reason: keyword). "each entry must additionally contain a subnet
  mask", `rfc2453.txt:1055-1056`.
- **Description** — version 2 of RIP adds the subnet mask to each entry, so that the subnets
  of one network and the masks of remote networks can be told apart. RIPng carries a prefix
  length from 0 to 128.
- **Checks** — core: RFC2453-TABLE-2 (the mask is part of the route), RFC2453-MASK-1 (the
  mask in the entry), RFC2080-MSG-10 (the prefix length in the entry).

## RIP-F-PERIODIC-UPDATE

**Every 30 seconds, with an offset against synchronization, a router sends its whole table to every neighbor.**

- **Sources** — RFC 2453 §3.8, `rfc2453.txt:1303-1320`. RFC 2080 §2.3,
  `rfc2080.txt:464-482`; §2.5, `rfc2080.txt:747-749`.
- **Level** — mandatory (reason: keyword and only path). "implementations are required to
  take one of two precautions", `rfc2453.txt:1311-1312`, `rfc2080.txt:473-474`; and the
  periodic update is the only path by which a neighbor learns that a route is still alive.
- **Description** — the update is an unsolicited response with every route that split horizon
  lets through. The 30-second timer runs on a clock that load does not affect, or it gets a
  random offset each time it is set: up to 5 seconds in RFC 2453, up to 15 seconds in
  RFC 2080.
- **Checks** — core: RFC2453-TIMER-1, TIMER-2; RFC2080-TIMER-1, TIMER-2, OUT-2 (the regular
  update half).

## RIP-F-ROUTE-LEARNING

**A router adds a new route from a response, adopts a better one or the news of its current next hop, and ignores the rest.**

- **Sources** — RFC 2453 §3.9.2, `rfc2453.txt:1463-1552`; §3.5, `rfc2453.txt:1024-1044`.
  RFC 2080 §2.4.2, `rfc2080.txt:635-735`; §2, `rfc2080.txt:194-214`.
- **Level** — mandatory (reason: only path). The processing of a response is the only way a
  route that is not directly connected enters the table.
- **Description** — a new destination with a metric below 16 is added, with the sender as
  next hop. An existing route takes the metric of the datagram when the datagram comes from
  its current next hop, whatever the metric, or when the metric is lower. Anything else is
  ignored. A new destination that arrives with metric 16 is not added.
- **Checks** — core: RFC2453-RESP-6 (add), RFC2453-RESP-7 (not at 16), RFC2453-RESP-9
  (adopt), RFC2453-RESP-12 (ignore the rest), RFC2453-TABLE-3 (the sender is the next hop),
  and RFC2080-RESP-8, RESP-9, RESP-11, RESP-14, TABLE-2. Supporting: RFC2453-RESP-1,
  RESP-8, RESP-11, TABLE-1 and RFC2080-RESP-1, RESP-10, RESP-13, TABLE-1 (the same processing
  for every cause, the refresh of the timeout, the switch at half the timeout, the content of
  a route).

## RIP-F-SPLIT-HORIZON

**A route is not advertised back with a finite metric onto the network it was learned from; poisoned reverse sends it there with metric 16.**

- **Sources** — RFC 2453 §3.4.3, `rfc2453.txt:810-889`. RFC 2080 §2.6,
  `rfc2080.txt:873-887`; §2.4.1, `rfc2080.txt:581-592`.
- **Level** — mandatory (reason: keyword). "all implementation of RIP must use split
  horizon", `rfc2453.txt:886-887`; RFC 2080 has "Split Horizon must be done",
  `rfc2080.txt:591-592`, and applies split horizon to every update.
- **Description** — simple split horizon leaves the route out of the updates on that network;
  poisoned reverse includes it with metric 16. RFC 2453 says a router should use poisoned
  reverse, and RFC 2080 calls it the preferred method. On a broadcast network the rule covers
  every router of the network, not only the next hop.
- **Checks** — core: RFC2453-SH-1, SH-2; RFC2080-SH-1, SH-2. Supporting: RFC2453-SH-3 (the
  permitted variants), RFC2453-SH-4 (the broadcast network), RFC2080-SH-3 (the control per
  interface).

## RIP-F-TRIGGERED-UPDATE

**A change of a route is sent at once, without waiting for the next periodic update, holding at least the changed routes, and at a limited rate.**

- **Sources** — RFC 2453 §3.4.4, `rfc2453.txt:903-970`; §3.10.1, `rfc2453.txt:1590-1639`.
  RFC 2080 §2.5, `rfc2080.txt:751-752`; §2.5.1, `rfc2080.txt:773-819`.
- **Level** — mandatory (reason: keyword). "must implement triggered update for deleted
  routes", `rfc2453.txt:967`; "must also limit the rate", `rfc2453.txt:969`; "must include at
  least those routes that have their route change flag set", `rfc2453.txt:1607-1608`,
  `rfc2080.txt:798-799`.
- **Description** — RFC 2453 demands a triggered update for a deleted route and permits one
  for a new or changed route; RFC 2080 triggers one "whenever the metric for a route is
  changed". A timer of 1 to 5 seconds after each triggered update collects further changes
  into one message. The update goes to every directly-connected network, after split horizon,
  and follows every rule of a response.
- **Checks** — core: RFC2453-TRIG-1 (a deleted route), RFC2453-TRIG-2 (the rate limit),
  RFC2453-TRIG-5 (the changed routes), RFC2453-TRIG-9 (the rules of a response), and
  RFC2080-OUT-2 (the triggered half), RFC2080-TRIG-1, TRIG-4, TRIG-8. Supporting:
  RFC2453-TRIG-3, TRIG-4, TRIG-6, TRIG-7, TRIG-8, GEN-4 and RFC2080-TRIG-2, TRIG-3, TRIG-5,
  TRIG-6, TRIG-7, GEN-6 (the timer, the suppression, every network, the permitted omissions,
  the clearing of the flags, the content of an entry).

## RIP-F-ROUTE-EXPIRY

**A route that is not refreshed for 180 seconds, or that its next hop withdraws, is advertised with metric 16 for 120 seconds and then removed; a new route in that time replaces it.**

- **Sources** — RFC 2453 §3.8, `rfc2453.txt:1322-1368`; §3.9.2, `rfc2453.txt:1531-1535`.
  RFC 2080 §2.3, `rfc2080.txt:484-533`; §2.4.2, `rfc2080.txt:707-711`.
- **Level** — mandatory (reason: keyword and only path). "the garbage-collection timer must be
  cleared", `rfc2453.txt:1367-1368`, `rfc2080.txt:532-533`; and the timeout is the only path
  by which the route through a router that has failed leaves the table.
- **Description** — the timeout restarts on each update for the route. When it expires, or
  when the next hop sends metric 16, the deletion starts: metric 16, the change flag, a
  triggered update, and a garbage-collection timer of 120 seconds. Until that timer expires
  the route stays in every update with metric 16; then it is removed. The deletion starts
  only once.
- **Checks** — core: RFC2453-TIMER-4, TIMER-5, TIMER-6, TIMER-7; RFC2080-TIMER-4, TIMER-5,
  TIMER-6, TIMER-7. Supporting: RFC2453-TIMER-3, RESP-8, RESP-10 and RFC2080-TIMER-3, RESP-10,
  RESP-12.

## RIP-F-RESPONSE-CONTENTS

**A response holds every route that split horizon lets through, those with metric 16 included, split into messages of at most 25 entries or of the MTU, and never a route to a link-local address.**

- **Sources** — RFC 2453 §3.6, `rfc2453.txt:1127`; §3.10.2, `rfc2453.txt:1641-1661`.
  RFC 2080 §2.1, `rfc2080.txt:360-372`; §2.5.2, `rfc2080.txt:856-871`.
- **Level** — mandatory (reason: keyword). "Routes must be included in the datagram even if
  their metrics are infinite", `rfc2453.txt:1660-1661`, `rfc2080.txt:870-871`; "Routes to
  link-local addresses must never be included in an RTE", `rfc2080.txt:863-864`.
- **Description** — RIP version 2 puts at most 25 entries into one message; RIPng fills a
  message up to the MTU. More routes go into more messages, and there is no limit on their
  number.
- **Checks** — core: RFC2453-GEN-3 (the limit of 25), RFC2453-GEN-5 (metric 16 included),
  RFC2453-MSG-7 (1 to 25 entries), RFC2080-GEN-4 (the MTU), RFC2080-GEN-5 (no link-local
  route), RFC2080-GEN-7 (metric 16 included), RFC2080-MSG-11 (the number of entries).
  Supporting: RFC2453-GEN-1 (the version of an answer), RFC2453-GEN-4 and RFC2080-GEN-6 (the
  content of an entry).

## RIP-F-RESPONSE-VALIDATION

**A router ignores a response that is not from its RIP port, not from a neighbor, or from itself, and each entry whose destination or metric is invalid.**

- **Sources** — RFC 2453 §3.9.2, `rfc2453.txt:1443-1475`. RFC 2080 §2.4.2,
  `rfc2080.txt:608-650`.
- **Level** — mandatory (reason: keyword). "The Response must be ignored if it is not from the
  RIP port", `rfc2453.txt:1444-1445`, `rfc2080.txt:609-610`, and the other `must` of the
  datagram checks.
- **Description** — the datagram as a whole must come from the RIP port and from a
  directly-connected network (RFC 2453) or a link-local address (RFC 2080), and not from the
  router itself. RFC 2080 adds the hop limit of 255 for a multicast from the RIPng port. Then
  each entry is checked: a valid destination, a prefix length up to 128 in RIPng, a metric
  from 1 to 16. A failed entry is skipped and the next one is processed.
- **Checks** — core: RFC2453-RESP-2, RESP-3, RESP-4, RESP-5; RFC2080-RESP-2, RESP-3, RESP-4,
  RESP-6, RESP-7. Every one needs a crafted response, so every check of this feature is
  level 3.

## RIP-F-TABLE-REQUEST

**A router that comes up asks its neighbors for their whole tables, and a router answers such a request directly to the requester, after split horizon.**

- **Sources** — RFC 2453 §3.9.1, `rfc2453.txt:1383-1431`; §3.10, `rfc2453.txt:1560-1561`;
  §3.10.2, `rfc2453.txt:1646-1649`. RFC 2080 §2.4.1, `rfc2080.txt:546-596`; §2.5,
  `rfc2080.txt:743-745`.
- **Level** — unstated. The whole-table request and its answer are described, not demanded:
  "Normally, Requests are sent", `rfc2453.txt:1384`, `rfc2080.txt:547`. The one `must`, "Split
  Horizon must be done", `rfc2453.txt:1427`, `rfc2080.txt:592`, holds for the answer once a
  request has arrived: a conditional keyword, which does not raise the level.
- **Description** — the request holds one entry of address family 0 (RIPng: prefix zero and
  prefix length zero) and metric 16, and it goes to the multicast group on every network. The
  answer is the output of the normal update, with split horizon, sent to the address and the
  port of the requester, before the next periodic update.
- **Checks** — core: RFC2453-REQ-4 (the form and the answer), RFC2453-REQ-6 (split horizon in
  the answer), RFC2453-OUT-1 (unicast to the requester), and RFC2080-REQ-4, REQ-6, OUT-1.
  Supporting: RFC2453-REQ-1, REQ-8, GEN-1 and RFC2080-REQ-1, REQ-8 (from the RIP port, on
  every network, in the version of the request).

## RIP-F-SPECIFIC-QUERY

**A request that names destinations gets their metrics back as they are, without split horizon; an empty request gets nothing.**

- **Sources** — RFC 2453 §3.9.1, `rfc2453.txt:1387-1431`. RFC 2080 §2.4.1,
  `rfc2080.txt:550-596`.
- **Level** — unstated. Every core statement is a description or a `should` of the requester.
- **Description** — the query usually comes from diagnostic software, from a port that is not
  the RIP port. The router fills in the metric of each entry, 16 for a destination it has no
  route to, turns the request into a response and sends it back, without split horizon. A
  RIPng router answers from a global address, because the requester may be on another
  network.
- **Checks** — core: RFC2453-REQ-2, REQ-3, REQ-5, REQ-7; RFC2080-REQ-2, REQ-3, REQ-5, REQ-7.
  Every check needs a querier that sends a crafted request, so every check of this feature
  is level 3.

## RIP-F-NEXT-HOP

**An update can name a better next hop than its sender; the next hop must be directly reachable, and zero means the sender.**

- **Sources** — RFC 2453 §4.4, `rfc2453.txt:1806-1822`. RFC 2080 §2.1.1,
  `rfc2080.txt:374-421`.
- **Level** — optional (reason: keyword). The field is "advisory", `rfc2453.txt:1818-1821`,
  `rfc2080.txt:417-420`, and a sender that names itself with zero meets every rule. The
  `must` of a directly reachable (RFC 2453) or link-local (RFC 2080) next hop holds when a
  sender names one: a conditional keyword.
- **Description** — in RIP version 2 each entry has a next hop field; in RIPng a next hop
  entry with metric 0xFF applies to the entries that follow it. Zero means the sender of the
  update. A receiver that gets an unusable next hop treats it as zero.
- **Checks** — core: RFC2453-NH-1, NH-2; RFC2080-NH-4, NH-5. Supporting: RFC2453-NH-3,
  NH-4 and RFC2080-NH-1, NH-2, NH-3, NH-6, NH-7.

## RIP-F-ROUTE-TAG

**A route keeps the route tag it arrives with, and the tag is advertised with it.**

- **Sources** — RFC 2453 §4.2, `rfc2453.txt:1758-1777`. RFC 2080 §2.1,
  `rfc2080.txt:326-350`.
- **Level** — mandatory (reason: keyword). "must be preserved and readvertised with a route",
  `rfc2453.txt:1761`, `rfc2080.txt:327`.
- **Description** — the tag separates the routes of the RIP domain from routes imported from
  other protocols. A router that imports routes should let the administrator set their tag.
- **Checks** — core: RFC2453-TAG-1, RFC2080-TAG-1. Supporting: RFC2453-TAG-2, RFC2080-TAG-2.
  A tagged route reaches a router only from a response that carries the tag, or from an
  import with a configured tag.

## RIP-F-DEFAULT-ROUTE

**A default route — 0.0.0.0, or a prefix of length zero — travels through RIP like any other route.**

- **Sources** — RFC 2453 §3.7, `rfc2453.txt:1267-1297`. RFC 2080 §2.2,
  `rfc2080.txt:428-458`.
- **Level** — unstated. The handling is described, "in exactly the same manner",
  `rfc2453.txt:1282-1284`, `rfc2080.txt:444-445`; whether a router originates one is a
  `should` of RFC 2453.
- **Description** — a router that can carry the traffic for networks not listed advertises a
  default route; the administrator may choose its metric. Receivers handle it as a network.
- **Checks** — core: RFC2453-ADDR-6, RFC2080-ADDR-1, ADDR-2. Supporting: RFC2453-ADDR-7,
  RFC2080-ADDR-3.

## RIP-F-HOST-ROUTES

**A router supports host routes, or it ignores the host routes it receives.**

- **Sources** — RFC 2453 §3.7, `rfc2453.txt:1183-1265`.
- **Level** — optional (reason: keyword). "some implementations may choose not to support
  host routes", `rfc2453.txt:1198`, and they "should support host routes",
  `rfc2453.txt:1264`. The `must ignore` holds only for an implementation that does not
  support them.
- **Description** — a host route is an entry for one address. A router that supports them
  advertises them on like any route; one that does not drops them on receipt.
- **Checks** — core: RFC2453-ADDR-1, ADDR-5. RFC 2080 has no counterpart: "The distinction
  between network, subnet and host routes does not need to be made for RIPng",
  `rfc2080.txt:425-426`.

## RIP-F-VERSION-1-INTERWORKING

**Where a version 1 router takes part, the version 2 router hides what version 1 would misread, and it answers a version 1 request in version 1.**

- **Sources** — RFC 2453 §3.7, `rfc2453.txt:1215-1261`; §4.3, `rfc2453.txt:1785-1804`;
  §4.6, `rfc2453.txt:1841-1845`.
- **Level** — mandatory (reason: keyword), where version 1 is in use. "nodes must not send
  subnet routes to nodes that cannot be expected to know the appropriate subnet mask",
  `rfc2453.txt:1227-1228`; information internal to one network "must never be advertised
  into another network", `rfc2453.txt:1788-1789`.
- **Description** — version 1 has no subnet mask, so a subnet route, a host route or a
  supernet route can be misread by a version 1 router. A border router therefore sends one
  entry for a subnetted network, and no host routes, into another network.
- **Checks** — core: RFC2453-ADDR-2, ADDR-4, MASK-2, QRY-1. Supporting: RFC2453-ADDR-3. Every
  check needs a version 1 router or a version 1 request in the network; the standards map
  puts the interworking with version 1 at level 5, and the query of version 1 needs a crafted
  request, level 3.

## Coverage of the catalog areas

Every area of the two catalogs appears in a feature:

| Catalog area | Features |
| --- | --- |
| RFC 2453, Message format and transport | RIP-F-MESSAGE-FORMAT, RIP-F-TRANSPORT, RIP-F-RESPONSE-CONTENTS |
| RFC 2453, Metric | RIP-F-METRIC |
| RFC 2453, Routing table | RIP-F-ROUTE-LEARNING, RIP-F-DESTINATION-PREFIX |
| RFC 2453, Addressing | RIP-F-HOST-ROUTES, RIP-F-VERSION-1-INTERWORKING, RIP-F-DEFAULT-ROUTE |
| RFC 2453, Split horizon | RIP-F-SPLIT-HORIZON |
| RFC 2453, Timers | RIP-F-PERIODIC-UPDATE, RIP-F-ROUTE-EXPIRY |
| RFC 2453, Request messages | RIP-F-TABLE-REQUEST, RIP-F-SPECIFIC-QUERY |
| RFC 2453, Response messages | RIP-F-ROUTE-LEARNING, RIP-F-RESPONSE-VALIDATION, RIP-F-ROUTE-EXPIRY |
| RFC 2453, Output processing | RIP-F-UPDATE-ADDRESSING, RIP-F-TABLE-REQUEST |
| RFC 2453, Triggered updates | RIP-F-TRIGGERED-UPDATE |
| RFC 2453, Generating a response | RIP-F-MESSAGE-FORMAT, RIP-F-RESPONSE-CONTENTS, RIP-F-TRIGGERED-UPDATE, RIP-F-TABLE-REQUEST |
| RFC 2453, Route tag | RIP-F-ROUTE-TAG |
| RFC 2453, Subnet mask | RIP-F-DESTINATION-PREFIX, RIP-F-VERSION-1-INTERWORKING |
| RFC 2453, Next hop | RIP-F-NEXT-HOP |
| RFC 2453, Multicast | RIP-F-UPDATE-ADDRESSING |
| RFC 2453, Queries of version 1 | RIP-F-VERSION-1-INTERWORKING |
| RFC 2080, Message format and transport | RIP-F-MESSAGE-FORMAT, RIP-F-TRANSPORT, RIP-F-DESTINATION-PREFIX, RIP-F-RESPONSE-CONTENTS |
| RFC 2080, Metric | RIP-F-METRIC |
| RFC 2080, Routing table | RIP-F-ROUTE-LEARNING |
| RFC 2080, Route tag | RIP-F-ROUTE-TAG |
| RFC 2080, Next hop | RIP-F-NEXT-HOP |
| RFC 2080, Addressing | RIP-F-DEFAULT-ROUTE |
| RFC 2080, Split horizon | RIP-F-SPLIT-HORIZON |
| RFC 2080, Timers | RIP-F-PERIODIC-UPDATE, RIP-F-ROUTE-EXPIRY |
| RFC 2080, Request messages | RIP-F-TABLE-REQUEST, RIP-F-SPECIFIC-QUERY |
| RFC 2080, Response messages | RIP-F-ROUTE-LEARNING, RIP-F-RESPONSE-VALIDATION, RIP-F-UPDATE-ADDRESSING, RIP-F-ROUTE-EXPIRY |
| RFC 2080, Output processing | RIP-F-UPDATE-ADDRESSING, RIP-F-TABLE-REQUEST, RIP-F-PERIODIC-UPDATE, RIP-F-TRIGGERED-UPDATE |
| RFC 2080, Triggered updates | RIP-F-TRIGGERED-UPDATE |
| RFC 2080, Generating a response | RIP-F-UPDATE-ADDRESSING, RIP-F-MESSAGE-FORMAT, RIP-F-RESPONSE-CONTENTS, RIP-F-TRIGGERED-UPDATE |

One entry is in no feature: [RFC2453-ADDR-8](../../standard/rfc2453/catalog.md#rfc2453-addr-8),
the most specific match when a datagram is routed. It is the forwarding rule of IPv4, and a
check of it belongs to the forwarding of IPv4, not to the exchange of routes.
