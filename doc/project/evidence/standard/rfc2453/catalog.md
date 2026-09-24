# RFC 2453 (RIP version 2) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC2453-*` · **Stands on:** [standards.md](../../protocol/rip/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for the IPv4 base
document of the in-scope set: RFC 2453, RIP Version 2, of November 1998. The catalog comes
from the RFC text only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc2453.txt`](../../../../../../standards/RFC/rfc2453.txt) —
  RIP Version 2, November 1998. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc2453.txt>.

The IPv6 twin of this document is RFC 2080, in [`rfc2080/catalog.md`](../rfc2080/catalog.md).
The two documents state the same mechanisms in parallel sections; the table in
[`standards.md`](../../protocol/rip/standards.md#document-list) maps the sections onto each
other. The family of documents and the in-scope set are in
[`standards.md`](../../protocol/rip/standards.md). The features these statements build are
in [`features.md`](../../protocol/rip/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`rip/coverage.md`](../../model/rip/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc2453.txt:1085-1087` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

## How to read an entry

- **ID** — `RFC2453-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 2453 does not use the keywords of RFC 2119:
  its "must", "should" and "may" are all in lower case. The entry records the word as
  written: `must`, `must not`, `should`, `should not`, `may`, or `description` for a
  normative sentence with no keyword. Where the text says "required", "highly recommended"
  or "strongly discouraged", the entry names the word.
- **Class** — how a test can observe the statement: `wire` (fields of messages on a link),
  `end-to-end` (what a router accepts and acts on, seen in what it sends or forwards
  afterwards), `error-signal` (a report message), `internal` (state inside a router),
  `encoding` (the exact bit layout).
- **Check idea** — one or two sentences, still without implementation names.

Two sentences of §3.4 quote "the router requirements RFC [11]", RFC 1812. They are
statements of RFC 2453 all the same: RFC 2453 repeats the rule in its own text, and the
catalog records it where it stands.

Section 3.9 of this document refers to "section 3.7.1", "section 3.7.2" and "section 3.8.1"
for material that stands in §3.9.1, §3.9.2 and §3.10.1. The references are errors of the
text; the catalog cites the section where the material stands.

## Index

| ID | Statement |
| --- | --- |
| [RFC2453-MSG-1](#rfc2453-msg-1) | A router sends and receives RIP messages on UDP port 520. |
| [RFC2453-MSG-2](#rfc2453-msg-2) | A message for the RIP process of another router goes to UDP port 520. |
| [RFC2453-MSG-3](#rfc2453-msg-3) | Every routing update leaves the RIP port, and an unsolicited update has 520 as both ports. |
| [RFC2453-MSG-4](#rfc2453-msg-4) | A response to a request goes to the port that the request came from. |
| [RFC2453-MSG-5](#rfc2453-msg-5) | A specific query may come from another port, and it goes to the RIP port of the target. |
| [RFC2453-MSG-6](#rfc2453-msg-6) | A RIP message is a 4-octet header — command, version, two octets that must be zero — and a list of 20-octet entries. |
| [RFC2453-MSG-7](#rfc2453-msg-7) | A message holds between 1 and 25 entries. |
| [RFC2453-MSG-8](#rfc2453-msg-8) | The fields are binary integers in network byte order. |
| [RFC2453-MSG-9](#rfc2453-msg-9) | Command 1 is a request and command 2 is a response. |
| [RFC2453-MSG-10](#rfc2453-msg-10) | The address family identifier of an IPv4 entry is 2. |
| [RFC2453-MSG-11](#rfc2453-msg-11) | A version 2 entry holds the address family identifier, a route tag, the address, a subnet mask, a next hop and the metric. |
| [RFC2453-MSG-12](#rfc2453-msg-12) | A message that carries information in the version 2 fields has version number 2. |
| [RFC2453-MET-1](#rfc2453-met-1) | The metric of a network is an integer from 1 to 15. |
| [RFC2453-MET-2](#rfc2453-met-2) | The administrator can set the metric of each network. |
| [RFC2453-MET-3](#rfc2453-met-3) | The metric of a directly-connected network is the cost of that network. |
| [RFC2453-MET-4](#rfc2453-met-4) | The metric field holds 1 to 15, or 16, which means that the destination is not reachable. |
| [RFC2453-MET-5](#rfc2453-met-5) | A received metric gets the cost of the arrival network added, and the result stops at 16. |
| [RFC2453-TABLE-1](#rfc2453-table-1) | Each route holds the destination, the metric, the next hop, a route change flag and its timers. |
| [RFC2453-TABLE-2](#rfc2453-table-2) | Each route also holds a subnet mask. |
| [RFC2453-TABLE-3](#rfc2453-table-3) | The next hop of a learned route is the router that sent the update. |
| [RFC2453-ADDR-1](#rfc2453-addr-1) | An implementation may leave out host routes, and then it drops the host routes it receives. |
| [RFC2453-ADDR-2](#rfc2453-addr-2) | A version 1 router does not send a subnet route where the subnet mask is not known, and not outside the network of the subnet. |
| [RFC2453-ADDR-3](#rfc2453-addr-3) | A border router sends one entry for a subnetted network as a whole to the neighbors in other networks. |
| [RFC2453-ADDR-4](#rfc2453-addr-4) | A border router does not advertise host routes of its directly-connected networks into other networks. |
| [RFC2453-ADDR-5](#rfc2453-addr-5) | A router should support host routes, and one that does not ignores the host routes it receives. |
| [RFC2453-ADDR-6](#rfc2453-addr-6) | The address 0.0.0.0 describes a default route, and RIP handles it like any other network. |
| [RFC2453-ADDR-7](#rfc2453-addr-7) | A router that is prepared to carry the traffic for unlisted networks creates an entry for 0.0.0.0. |
| [RFC2453-ADDR-8](#rfc2453-addr-8) | A datagram is routed by the most specific route: a host route first, then a subnet or network, then the default. |
| [RFC2453-SH-1](#rfc2453-sh-1) | A router uses split horizon, and it should use split horizon with poisoned reverse. |
| [RFC2453-SH-2](#rfc2453-sh-2) | Simple split horizon omits a route in the updates to the neighbor it came from; poisoned reverse sends it there with metric 16. |
| [RFC2453-SH-3](#rfc2453-sh-3) | An implementation may choose simple split horizon, poisoned reverse, a switch between them, or a mix. |
| [RFC2453-SH-4](#rfc2453-sh-4) | On a broadcast network, a route through one router of the network is poisoned towards every other router of that network too. |
| [RFC2453-TIMER-1](#rfc2453-timer-1) | Every 30 seconds a router sends an unsolicited response with its whole table to every neighbor. |
| [RFC2453-TIMER-2](#rfc2453-timer-2) | The 30-second updates are kept from synchronizing: by a clock that load does not change, or by a random offset of up to 5 seconds. |
| [RFC2453-TIMER-3](#rfc2453-timer-3) | A route has a timeout and a garbage-collection timer; after the timeout the route is invalid but stays in the table for a short time. |
| [RFC2453-TIMER-4](#rfc2453-timer-4) | The timeout starts when the route is set up and again on each update for it; after 180 seconds the route expires. |
| [RFC2453-TIMER-5](#rfc2453-timer-5) | On deletion — by timeout, or by metric 16 from the current next hop — the route gets metric 16, the garbage-collection timer of 120 seconds, the change flag, and a triggered update. |
| [RFC2453-TIMER-6](#rfc2453-timer-6) | Until the garbage-collection timer expires, the route is in every update; then it is removed. |
| [RFC2453-TIMER-7](#rfc2453-timer-7) | A new route that arrives during garbage collection replaces the old one and clears the garbage-collection timer. |
| [RFC2453-REQ-1](#rfc2453-req-1) | A router that has just come up sends requests from the RIP port, by broadcast or multicast. |
| [RFC2453-REQ-2](#rfc2453-req-2) | A request for the table of one router should come from another port, and the router answers to the address and port of the requester. |
| [RFC2453-REQ-3](#rfc2453-req-3) | A request with no entries gets no response. |
| [RFC2453-REQ-4](#rfc2453-req-4) | A request with one entry of address family 0 and metric 16 asks for the whole table, which goes to the requesting address and port. |
| [RFC2453-REQ-5](#rfc2453-req-5) | For a specific request, each entry gets the metric of the route, or 16 if there is none, and the message goes back as a response. |
| [RFC2453-REQ-6](#rfc2453-req-6) | The answer to a whole-table request goes through normal output processing, split horizon included. |
| [RFC2453-REQ-7](#rfc2453-req-7) | The answer to a specific request is the table as it is, without split horizon. |
| [RFC2453-REQ-8](#rfc2453-req-8) | A router that comes up multicasts a request for the complete table on every connected network. |
| [RFC2453-RESP-1](#rfc2453-resp-1) | A response is processed the same way whatever caused it. |
| [RFC2453-RESP-2](#rfc2453-resp-2) | A response that does not come from port 520 is ignored. |
| [RFC2453-RESP-3](#rfc2453-resp-3) | The source of a response must be on a directly-connected network. |
| [RFC2453-RESP-4](#rfc2453-resp-4) | A router ignores its own messages. |
| [RFC2453-RESP-5](#rfc2453-resp-5) | An entry with an invalid destination or a metric outside 1 to 16 is ignored, and the next entry is processed. |
| [RFC2453-RESP-6](#rfc2453-resp-6) | A new destination with a metric below 16 is added, with the sender as next hop, a fresh timeout, the change flag and a triggered update. |
| [RFC2453-RESP-7](#rfc2453-resp-7) | A new destination that arrives with metric 16 is not added. |
| [RFC2453-RESP-8](#rfc2453-resp-8) | An update for an existing route from its current next hop restarts the timeout. |
| [RFC2453-RESP-9](#rfc2453-resp-9) | The route from the datagram is adopted when it comes from the current next hop with a different metric, or when its metric is lower. |
| [RFC2453-RESP-10](#rfc2453-resp-10) | The deletion starts only when the metric first becomes 16. |
| [RFC2453-RESP-11](#rfc2453-resp-11) | With an equal metric, a router may switch to the new route when the current one is at least halfway to its timeout. |
| [RFC2453-RESP-12](#rfc2453-resp-12) | Any other entry is ignored: an equal or worse route from a router that is not the current next hop changes nothing. |
| [RFC2453-OUT-1](#rfc2453-out-1) | The response to a request is unicast to the requester. |
| [RFC2453-OUT-2](#rfc2453-out-2) | Regular and triggered updates go to all neighbors: one response for each directly-connected network, multicast for version 2. |
| [RFC2453-TRIG-1](#rfc2453-trig-1) | A router sends a triggered update for a deleted route, and it may send one for a new or changed route. |
| [RFC2453-TRIG-2](#rfc2453-trig-2) | A router limits the rate of its triggered updates. |
| [RFC2453-TRIG-3](#rfc2453-trig-3) | After a triggered update, a random timer of 1 to 5 seconds collects further changes into one update. |
| [RFC2453-TRIG-4](#rfc2453-trig-4) | A triggered update is suppressed when a regular update is due by the time it would be sent. |
| [RFC2453-TRIG-5](#rfc2453-trig-5) | A triggered update holds at least the routes whose change flag is set; a complete table is strongly discouraged. |
| [RFC2453-TRIG-6](#rfc2453-trig-6) | A triggered update goes out on every directly-connected network, after split horizon. |
| [RFC2453-TRIG-7](#rfc2453-trig-7) | A changed route that split horizon leaves unchanged on a network need not be sent there, and an empty update may be left out. |
| [RFC2453-TRIG-8](#rfc2453-trig-8) | After the triggered updates are generated, the change flags are cleared. |
| [RFC2453-TRIG-9](#rfc2453-trig-9) | The rules of the next section apply to triggered updates as to all other updates. |
| [RFC2453-GEN-1](#rfc2453-gen-1) | The version of a response is 1 or 2, and the response to a request should carry the version of the request. |
| [RFC2453-GEN-2](#rfc2453-gen-2) | A response carries command 2 and zero in the octets that must be zero. |
| [RFC2453-GEN-3](#rfc2453-gen-3) | A response holds at most 25 entries; more routes go into further messages. |
| [RFC2453-GEN-4](#rfc2453-gen-4) | A triggered update need hold only the changed routes; a route that split horizon excludes is skipped; an included route puts its destination and metric into an entry. |
| [RFC2453-GEN-5](#rfc2453-gen-5) | A route with metric 16 is included in the updates too. |
| [RFC2453-TAG-1](#rfc2453-tag-1) | A route keeps its route tag, and the tag is advertised with it. |
| [RFC2453-TAG-2](#rfc2453-tag-2) | A router that imports routes from other protocols can set the route tag of the imported routes. |
| [RFC2453-MASK-1](#rfc2453-mask-1) | The subnet mask field holds the mask of the entry, and zero means that no mask is included. |
| [RFC2453-MASK-2](#rfc2453-mask-2) | Where a version 1 router can hear version 2 entries, three rules limit what is advertised. |
| [RFC2453-NH-1](#rfc2453-nh-1) | A next hop of 0.0.0.0 means routing through the router that sent the advertisement. |
| [RFC2453-NH-2](#rfc2453-nh-2) | A next hop that an entry names is directly reachable on the subnet where the entry is advertised. |
| [RFC2453-NH-3](#rfc2453-nh-3) | The next hop is advisory: a receiver that ignores it still has a valid route. |
| [RFC2453-NH-4](#rfc2453-nh-4) | A received next hop that is not directly reachable is treated as 0.0.0.0. |
| [RFC2453-MCAST-1](#rfc2453-mcast-1) | Periodic updates go to the IP multicast address 224.0.0.9. |
| [RFC2453-MCAST-2](#rfc2453-mcast-2) | RIP multicasts stay on their link. |
| [RFC2453-MCAST-3](#rfc2453-mcast-3) | On a network without broadcast, unicast may be used, and a response to the multicast address is accepted. |
| [RFC2453-MCAST-4](#rfc2453-mcast-4) | Multicast is a configuration choice, and when used it is used on every interface that can. |
| [RFC2453-QRY-1](#rfc2453-qry-1) | A version 2 router answers a version 1 request with a version 1 response, unless it is set to send version 2 only. |

## Message format and transport

### RFC2453-MSG-1

**A router sends and receives RIP messages on UDP port 520.**

> "Each router that uses RIP has a routing
> process that sends and receives datagrams on UDP port number 520, the
> RIP-1/RIP-2 port." — §3.6, `rfc2453.txt:1085-1087`

- Strength: description. Class: wire.
- Check idea: every RIP message that a router sends leaves UDP port 520.

### RFC2453-MSG-2

**A message for the RIP process of another router goes to UDP port 520.**

> "All communications intended for another routers's RIP process are sent to the RIP
> port." — §3.6, `rfc2453.txt:1087-1088`

- Strength: description. Class: wire.
- Check idea: a request and a periodic update of a router both carry destination port 520.

### RFC2453-MSG-3

**Every routing update leaves the RIP port, and an unsolicited update has 520 as both ports.**

> "All routing update messages
> are sent from the RIP port.  Unsolicited routing update messages have
> both the source and destination port equal to the RIP port." — §3.6, `rfc2453.txt:1088-1090`

- Strength: description. Class: wire.
- Check idea: a periodic update and a triggered update carry source port 520 and
  destination port 520.

### RFC2453-MSG-4

**A response to a request goes to the port that the request came from.**

> "Update
> messages sent in response to a request are sent to the port from
> which the request came." — §3.6, `rfc2453.txt:1090-1092`

- Strength: description. Class: wire.
- Check idea: the response to a whole-table request that came from port 520 goes to port
  520; the response to a request from another port goes to that port.

### RFC2453-MSG-5

**A specific query may come from another port, and it goes to the RIP port of the target.**

> "Specific queries may be sent from ports
> other than the RIP port, but they must be directed to the RIP port on
> the target machine." — §3.6, `rfc2453.txt:1092-1094`

- Strength: may (the source port), must (the destination port). Class: wire.
- Check idea: a request that a router sends carries destination port 520.

### RFC2453-MSG-6

**A RIP message is a 4-octet header — command, version, two octets that must be zero — and a list of 20-octet entries.**

> "|  command (1)  |  version (1)  |       must be zero (2)        |" and
> "~                         RIP Entry (20)                        ~" — §3.6,
> `rfc2453.txt:1101`, `rfc2453.txt:1104`

- Strength: description. Class: encoding.
- Check idea: the octets of a message: one octet of command, one of version, two zero
  octets, then entries of 20 octets each.

### RFC2453-MSG-7

**A message holds between 1 and 25 entries.**

> "There may be between 1 and 25 (inclusive) RIP entries." — §3.6, `rfc2453.txt:1127`

- Strength: description. Class: wire.
- Check idea: a router whose table has more than 25 routes sends its update in more than
  one message, and no message holds more than 25 entries.

### RFC2453-MSG-8

**The fields are binary integers in network byte order.**

> "Unless otherwise specified, fields
> contain binary integers, in network byte order, with the most-
> significant octet first (big-endian)." — §3.6, `rfc2453.txt:1144-1146`

- Strength: description. Class: encoding.
- Check idea: the octets of a metric of 1 are `00 00 00 01`.

### RFC2453-MSG-9

**Command 1 is a request and command 2 is a response.**

> "1 - request    A request for the responding system to send all or
> part of its routing table." and "2 - response   A message containing all or part of the
> sender's routing table." — §3.6, `rfc2453.txt:1155-1159`

- Strength: description. Class: wire.
- Check idea: the request at startup carries command 1, and every update carries command 2.

### RFC2453-MSG-10

**The address family identifier of an IPv4 entry is 2.**

> "The AFI is the type of address.  For RIP-1, only AF_INET (2) is
> generally supported." — §3.6, `rfc2453.txt:1168-1169`; and for version 2, "The Address
> Family Identifier, IP Address, and Metric all have the meanings defined in section 3.4."
> — §4, `rfc2453.txt:1711-1712`

- Strength: description. Class: wire.
- Check idea: every entry of an update carries address family identifier 2.

### RFC2453-MSG-11

**A version 2 entry holds the address family identifier, a route tag, the address, a subnet mask, a next hop and the metric.**

> "| Address Family Identifier (2) |        Route Tag (2)          |", then "IP Address (4)",
> "Subnet Mask (4)", "Next Hop (4)" and "Metric (4)" — §4, `rfc2453.txt:1697-1709`

- Strength: description. Class: encoding.
- Check idea: the octets of a version 2 entry: two of address family, two of route tag, then
  four each of address, subnet mask, next hop and metric.

### RFC2453-MSG-12

**A message that carries information in the version 2 fields has version number 2.**

> "The Version field will specify
> version number 2 for RIP messages which use authentication or carry
> information in any of the newly defined fields." — §4, `rfc2453.txt:1712-1714`

- Strength: description. Class: wire.
- Check idea: an update whose entries carry a subnet mask has version 2.

## Metric

### RFC2453-MET-1

**The metric of a network is an integer from 1 to 15.**

> "The RIP metric of a network is an integer between 1
> and 15, inclusive." — §3.5, `rfc2453.txt:980-981`

- Strength: description. Class: wire.
- Check idea: the metric that a router advertises for a network it reaches is between 1
  and 15.

### RFC2453-MET-2

**The administrator can set the metric of each network.**

> "Implementations should allow the system
> administrator to set the metric of each network." — §3.5, `rfc2453.txt:983-984`

- Strength: should. Class: wire.
- Check idea: a network with a configured cost of 3 is advertised with metric 3 by the
  router that is directly connected to it.

### RFC2453-MET-3

**The metric of a directly-connected network is the cost of that network.**

> "The metric for a directly-connected network is set to the
> cost of that network." — §3.5, `rfc2453.txt:1048-1049`

- Strength: description. Class: wire.
- Check idea: a router advertises each network it is attached to with the cost of that
  network, 1 when no cost is configured.

### RFC2453-MET-4

**The metric field holds 1 to 15, or 16, which means that the destination is not reachable.**

> "The metric field contains a value between 1 and 15 (inclusive) which
> specifies the current metric for the destination; or the value 16
> (infinity), which indicates that the destination is not reachable." — §3.6,
> `rfc2453.txt:1171-1173`

- Strength: description. Class: wire.
- Check idea: no entry of any update carries a metric of 0 or above 16.

### RFC2453-MET-5

**A received metric gets the cost of the arrival network added, and the result stops at 16.**

> "Once the entry has been validated, update the metric by adding the
> cost of the network on which the message arrived.  If the result is
> greater than infinity, use infinity." — §3.9.2, `rfc2453.txt:1477-1479`

- Strength: description. Class: wire.
- Check idea: in a chain of routers, each router advertises a remote network with a metric
  one higher than the router before it; a route that arrives with metric 15 over a network
  of cost 1 or more is not advertised with a finite metric.

## Routing table

### RFC2453-TABLE-1

**Each route holds the destination, the metric, the next hop, a route change flag and its timers.**

> "Each entry contains at least
> the following information:" — §3.5, `rfc2453.txt:1026-1027`, followed by the five items
> at `rfc2453.txt:1029-1044`

- Strength: description. Class: internal.
- Check idea: none of its own. Each item shows in the messages that other entries cover:
  the metric in the updates, the next hop in the forwarding, the flag and the timers in the
  triggered updates and the deletions.

### RFC2453-TABLE-2

**Each route also holds a subnet mask.**

> "To support the extensions detailed in this document, each entry must
> additionally contain a subnet mask." — §3.5, `rfc2453.txt:1055-1056`

- Strength: must. Class: wire.
- Check idea: two routes with the same network number and different mask lengths are
  advertised as two entries with their two masks.

### RFC2453-TABLE-3

**The next hop of a learned route is the router that sent the update.**

> "- Set the next hop address to be the address of the router from which
> the datagram came" — §3.9.2, `rfc2453.txt:1495-1496`

- Strength: description. Class: end-to-end.
- Check idea: a host behind a router reaches a network two routers away, which is possible
  only when each router forwards to the router it learned the route from.

## Addressing

### RFC2453-ADDR-1

**An implementation may leave out host routes, and then it drops the host routes it receives.**

> "Thus, some implementations may choose not to support host routes.  If
> host routes are not supported, they are to be dropped when they are
> received in response messages" — §3.7, `rfc2453.txt:1198-1200`

- Strength: may. Class: end-to-end.
- Check idea: a response with a host route; the receiver either advertises it on or drops it,
  and in neither case forwards to it through a wrong route.

### RFC2453-ADDR-2

**A version 1 router does not send a subnet route where the subnet mask is not known, and not outside the network of the subnet.**

> "In order to avoid this sort of
> ambiguity, when using version 1, nodes must not send subnet routes to
> nodes that cannot be expected to know the appropriate subnet mask." — §3.7,
> `rfc2453.txt:1226-1228`; and "unless special provisions have been made," "routes to a
> subnet must not be sent outside the network of which the subnet is a part." —
> `rfc2453.txt:1230`, `rfc2453.txt:1239-1240`

- Strength: must not. Class: wire.
- Check idea: a router that sends version 1 on a link to another network advertises the
  whole network, not its subnets. The statement holds for version 1 only.

### RFC2453-ADDR-3

**A border router sends one entry for a subnetted network as a whole to the neighbors in other networks.**

> "However, border routers send only a
> single entry for the network as a whole to nodes in other networks." — §3.7,
> `rfc2453.txt:1247-1248`

- Strength: description. Class: wire.
- Check idea: the version 1 counterpart of RFC2453-ADDR-2: the update on a link to another
  network holds one entry for the subnetted network.

### RFC2453-ADDR-4

**A border router does not advertise host routes of its directly-connected networks into other networks.**

> "Similarly, border routers must not mention host routes for nodes
> within one of the directly-connected networks in messages to other
> networks." — §3.7, `rfc2453.txt:1258-1260`

- Strength: must not. Class: wire.
- Check idea: a host route inside network N is absent from the updates that the border
  router of N sends into another network. The sentence continues the subnet filtering of
  version 1.

### RFC2453-ADDR-5

**A router should support host routes, and one that does not ignores the host routes it receives.**

> "The router requirements RFC [11] specifies that all implementation of
> RIP should support host routes but if they do not then they must
> ignore any received host routes." — §3.7, `rfc2453.txt:1263-1265`

- Strength: should (support), must (ignore when not supported). Class: end-to-end.
- Check idea: a host route arrives in a response; the receiver advertises it on, or leaves
  it out of every later update and out of its forwarding.

### RFC2453-ADDR-6

**The address 0.0.0.0 describes a default route, and RIP handles it like any other network.**

> "The special address 0.0.0.0 is used to describe a default route." — §3.7,
> `rfc2453.txt:1267`; "The entries for 0.0.0.0 are handled by RIP
> in exactly the same manner as if there were an actual network with
> this address." — `rfc2453.txt:1282-1284`

- Strength: description. Class: wire.
- Check idea: a default route that one router originates reaches the next router and is
  advertised on with the metric increased, like any other route.

### RFC2453-ADDR-7

**A router that is prepared to carry the traffic for unlisted networks creates an entry for 0.0.0.0.**

> "These routers should create
> RIP entries for the address 0.0.0.0, just as if it were a network to
> which they are connected." — §3.7, `rfc2453.txt:1271-1273`

- Strength: should. Class: wire.
- Check idea: a router with a default route in its configuration advertises an entry for
  0.0.0.0.

### RFC2453-ADDR-8

**A datagram is routed by the most specific route: a host route first, then a subnet or network, then the default.**

> "That is, when routing
> a datagram, its destination address must first be checked against the
> list of node addresses.  Then it must be checked to see whether it
> matches any known subnet or network number.  Finally, if none of
> these match, the default route is used." — §3.7, `rfc2453.txt:1209-1213`

- Strength: must. Class: end-to-end.
- Check idea: the rule is the forwarding rule of IPv4; a check of it belongs to the
  forwarding of IPv4 and not to the exchange of routes.

## Split horizon

### RFC2453-SH-1

**A router uses split horizon, and it should use split horizon with poisoned reverse.**

> "The router requirements RFC [11] specifies that all implementation of
> RIP must use split horizon and should also use split horizon with
> poisoned reverse, although there may be a knob to disable poisoned
> reverse." — §3.4.3, `rfc2453.txt:886-889`

- Strength: must (split horizon), should (poisoned reverse), may (the knob). Class: wire.
- Check idea: a route learned from a neighbor is either absent from the updates to that
  neighbor or present with metric 16; it never appears there with a finite metric.

### RFC2453-SH-2

**Simple split horizon omits a route in the updates to the neighbor it came from; poisoned reverse sends it there with metric 16.**

> "The "simple split
> horizon" scheme omits routes learned from one neighbor in updates
> sent to that neighbor.  "Split horizon with poisoned reverse"
> includes such routes in updates, but sets their metrics to infinity." — §3.4.3,
> `rfc2453.txt:819-822`

- Strength: description. Class: wire.
- Check idea: with poisoned reverse, the update to the neighbor holds the route with metric
  16; with simple split horizon, it does not hold the route.

### RFC2453-SH-3

**An implementation may choose simple split horizon, poisoned reverse, a switch between them, or a mix.**

> "Thus implementors may at their option implement simple split horizon
> rather than split horizon with poisoned reverse, or they may provide
> a configuration option that allows the network manager to choose
> which behavior to use.  It is also permissible to implement hybrid
> schemes that advertise some reverse routes with a metric of 16 and
> omit others." — §3.4.3, `rfc2453.txt:877-882`

- Strength: may. Class: wire.
- Check idea: a permission; it widens the observation of RFC2453-SH-1 to "absent or 16".

### RFC2453-SH-4

**On a broadcast network, a route through one router of the network is poisoned towards every other router of that network too.**

> "If A has a route through C, it should indicate that D is
> unreachable when talking to any other router on that network." — §3.4.3,
> `rfc2453.txt:833-835`

- Strength: should. Class: wire.
- Check idea: three routers on one LAN; a route that A learned from C is absent or has
  metric 16 in the update that A multicasts on the LAN, which every router of the LAN hears.

## Timers

### RFC2453-TIMER-1

**Every 30 seconds a router sends an unsolicited response with its whole table to every neighbor.**

> "Every 30 seconds, the RIP process is awakened to send an unsolicited
> Response message containing the complete routing table (see section
> 3.9 on Split Horizon) to every neighboring router." — §3.8, `rfc2453.txt:1303-1305`

- Strength: description. Class: wire.
- Check idea: the unsolicited responses of a router on a link come about 30 seconds apart,
  and each one holds every route of its table that split horizon lets through.

### RFC2453-TIMER-2

**The 30-second updates are kept from synchronizing: by a clock that load does not change, or by a random offset of up to 5 seconds.**

> "Therefore,
> implementations are required to take one of two precautions:" — §3.8,
> `rfc2453.txt:1311-1312`, followed by "The 30-second timer is offset by a small random time
> (+/- 0 to 5
> seconds) each time it is set." — `rfc2453.txt:1318-1319`

- Strength: must ("required"). Class: wire.
- Check idea: the intervals between the periodic updates of a router stay between 25 and 35
  seconds. Whether the offset is random is a distribution.

### RFC2453-TIMER-3

**A route has a timeout and a garbage-collection timer; after the timeout the route is invalid but stays in the table for a short time.**

> "Upon expiration of the timeout, the route
> is no longer valid; however, it is retained in the routing table for
> a short time so that neighbors can be notified that the route has
> been dropped.  Upon expiration of the garbage-collection timer, the
> route is finally removed from the routing table." — §3.8, `rfc2453.txt:1323-1327`

- Strength: description. Class: wire.
- Check idea: after its neighbor goes silent, a route first appears with metric 16 in the
  updates and later disappears from them.

### RFC2453-TIMER-4

**The timeout starts when the route is set up and again on each update for it; after 180 seconds the route expires.**

> "The timeout is initialized when a route is established, and any time
> an update message is received for the route.  If 180 seconds elapse
> from the last time the timeout was initialized, the route is
> considered to have expired, and the deletion process described below
> begins for that route." — §3.8, `rfc2453.txt:1329-1333`

- Strength: description. Class: wire.
- Check idea: a neighbor stops without notice; 180 seconds after its last update, the route
  through it is withdrawn with metric 16, and not earlier.

### RFC2453-TIMER-5

**On deletion — by timeout, or by metric 16 from the current next hop — the route gets metric 16, the garbage-collection timer of 120 seconds, the change flag, and a triggered update.**

> "Deletions can occur for one of two reasons: the timeout expires, or
> the metric is set to 16 because of an update received from the
> current router" — §3.8, `rfc2453.txt:1335-1337`; "- The garbage-collection timer is set
> for 120 seconds." — `rfc2453.txt:1351`; "- The metric for the route is set to 16
> (infinity)." — `rfc2453.txt:1353`; "- The route change flag is set to indicate that this
> entry has been
> changed." — `rfc2453.txt:1356-1357`; "- The output process is signalled to trigger a
> response." — `rfc2453.txt:1359`

- Strength: description. Class: wire.
- Check idea: when a route is deleted, the router sends an update with the route at metric
  16 soon after, without waiting for the next periodic update.

### RFC2453-TIMER-6

**Until the garbage-collection timer expires, the route is in every update; then it is removed.**

> "Until the garbage-collection timer expires, the route is included in
> all updates sent by this router.  When the garbage-collection timer
> expires, the route is deleted from the routing table." — §3.8, `rfc2453.txt:1361-1363`

- Strength: description. Class: wire.
- Check idea: after a deletion, every periodic update for 120 seconds holds the route at
  metric 16, and the updates after that do not hold it.

### RFC2453-TIMER-7

**A new route that arrives during garbage collection replaces the old one and clears the garbage-collection timer.**

> "Should a new route to this network be established while the garbage-
> collection timer is running, the new route will replace the one that
> is about to be deleted.  In this case the garbage-collection timer
> must be cleared." — §3.8, `rfc2453.txt:1365-1368`

- Strength: must. Class: wire.
- Check idea: a route is deleted and comes back before 120 seconds pass; the router still
  advertises it with a finite metric after the instant when the old garbage collection
  would have expired.

## Request messages

### RFC2453-REQ-1

**A router that has just come up sends requests from the RIP port, by broadcast or multicast.**

> "Normally, Requests are sent as broadcasts
> (multicasts for RIP-2), from the RIP port, by routers which have just
> come up and are seeking to fill in their routing tables as quickly as
> possible." — §3.9.1, `rfc2453.txt:1384-1387`

- Strength: description. Class: wire.
- Check idea: a router that starts sends a request from port 520 to the RIP multicast
  address.

### RFC2453-REQ-2

**A request for the table of one router should come from another port, and the router answers to the address and port of the requester.**

> "In this case, the Request should be sent directly to that router from a UDP
> port other than the RIP port.  If such a Request is received, the
> router responds directly to the requestor's address and port." — §3.9.1,
> `rfc2453.txt:1388-1391`

- Strength: should (the requester), description (the answer). Class: wire.
- Check idea: a request from port 5000 of a host; the answer goes to that host and port
  5000.

### RFC2453-REQ-3

**A request with no entries gets no response.**

> "If there are no entries, no
> response is given." — §3.9.1, `rfc2453.txt:1393-1394`

- Strength: description. Class: wire.
- Check idea: a request with an empty list of entries; no response follows.

### RFC2453-REQ-4

**A request with one entry of address family 0 and metric 16 asks for the whole table, which goes to the requesting address and port.**

> "If there is exactly
> one entry in the request, and it has an address family identifier of
> zero and a metric of infinity (i.e., 16), then this is a request to
> send the entire routing table.  In that case, a call is made to the
> output process to send the routing table to the requesting" "address/port." — §3.9.1,
> `rfc2453.txt:1394-1398`, `rfc2453.txt:1407`

- Strength: description. Class: wire.
- Check idea: the request of a router that starts has this form, and its neighbor answers
  with its table, sent to the address and port of the requester.

### RFC2453-REQ-5

**For a specific request, each entry gets the metric of the route, or 16 if there is none, and the message goes back as a response.**

> "Examine the list of RTEs in the Request one by one.  For
> each entry, look up the destination in the router's routing database
> and, if there is a route, put that route's metric in the metric field
> of the RTE.  If there is no explicit route to the specified
> destination, put infinity in the metric field.  Once all the entries
> have been filled in, change the command from Request to Response and
> send the datagram back to the requestor." — §3.9.1, `rfc2453.txt:1408-1414`

- Strength: description. Class: wire.
- Check idea: a request for two destinations, one known and one unknown; the response holds
  the known metric and 16, in the same order.

### RFC2453-REQ-6

**The answer to a whole-table request goes through normal output processing, split horizon included.**

> "If the request is for a complete routing
> table, normal output processing is done, including Split Horizon" — §3.9.1,
> `rfc2453.txt:1417-1419`; "For this reason, Split Horizon must be done." —
> `rfc2453.txt:1426-1427`

- Strength: must. Class: wire.
- Check idea: the answer to the request of a router that starts does not advertise, with a
  finite metric, a route learned from that router.

### RFC2453-REQ-7

**The answer to a specific request is the table as it is, without split horizon.**

> "If the request is for specific
> entries, they are looked up in the routing table and the information
> is returned as is; no Split Horizon processing is done." — §3.9.1,
> `rfc2453.txt:1419-1421`

- Strength: description. Class: wire.
- Check idea: a specific request from a neighbor for a route that the answering router
  learned from that neighbor; the answer holds the finite metric.

### RFC2453-REQ-8

**A router that comes up multicasts a request for the complete table on every connected network.**

> "When a router first comes
> up, it multicasts a Request on every connected network asking for a
> complete routing table." — §3.9.1, `rfc2453.txt:1423-1425`

- Strength: description. Class: wire.
- Check idea: a router with two networks sends a whole-table request on each of them when
  it starts.

## Response messages

### RFC2453-RESP-1

**A response is processed the same way whatever caused it.**

> "Processing is the same no matter why the Response was generated." — §3.9.2,
> `rfc2453.txt:1441`

- Strength: description. Class: end-to-end.
- Check idea: none of its own; the checks of periodic, triggered and requested responses
  each show one cause.

### RFC2453-RESP-2

**A response that does not come from port 520 is ignored.**

> "The Response must be ignored if it is not from the RIP port." — §3.9.2,
> `rfc2453.txt:1444-1445`

- Strength: must. Class: end-to-end.
- Check idea: a response from port 5000 that offers a new route; the receiver does not
  advertise the route.

### RFC2453-RESP-3

**The source of a response must be on a directly-connected network.**

> "The
> datagram's IPv4 source address should be checked to see whether the
> datagram is from a valid neighbor; the source of the datagram must be
> on a directly-connected network." — §3.9.2, `rfc2453.txt:1445-1448`

- Strength: should (check), must (the condition). Class: end-to-end.
- Check idea: a response whose source address is on no network of the receiver; the
  receiver does not take its routes.

### RFC2453-RESP-4

**A router ignores its own messages.**

> "If a router processes its own
> output as new input, confusion is likely so such datagrams must be
> ignored." — §3.9.2, `rfc2453.txt:1451-1453`

- Strength: must. Class: end-to-end.
- Check idea: a response with the source address of the receiver and a changed metric;
  the receiver does not act on it.

### RFC2453-RESP-5

**An entry with an invalid destination or a metric outside 1 to 16 is ignored, and the next entry is processed.**

> "- is the destination address valid (e.g., unicast; not net 0 or 127)
> - is the metric valid (i.e., between 1 and 16, inclusive)
>
> If any check fails, ignore that entry and proceed to the next." — §3.9.2,
> `rfc2453.txt:1471-1474`

- Strength: description. Class: end-to-end.
- Check idea: a response with three entries: a multicast destination, a metric of 17, and a
  valid route; only the valid route is learned.

### RFC2453-RESP-6

**A new destination with a metric below 16 is added, with the sender as next hop, a fresh timeout, the change flag and a triggered update.**

> "If there is no such route, add this route to the
> routing table, unless the metric is infinity" — §3.9.2, `rfc2453.txt:1484-1486`, and
> the six items of "Adding a route to the routing table consists of:",
> `rfc2453.txt:1486-1504`

- Strength: description. Class: wire.
- Check idea: a router learns a new network from a neighbor and advertises it on its other
  links with the metric increased by the cost.

### RFC2453-RESP-7

**A new destination that arrives with metric 16 is not added.**

> "add this route to the
> routing table, unless the metric is infinity (there is no point
> in adding a route which is unusable)." — §3.9.2, `rfc2453.txt:1484-1486`

- Strength: description. Class: wire.
- Check idea: a network offered only with metric 16 never appears in the updates of the
  receiver.

### RFC2453-RESP-8

**An update for an existing route from its current next hop restarts the timeout.**

> "If this datagram
> is from the same router as the existing route, reinitialize the
> timeout." — §3.9.2, `rfc2453.txt:1507-1509`

- Strength: description. Class: wire.
- Check idea: a route whose next hop keeps sending updates never times out.

### RFC2453-RESP-9

**The route from the datagram is adopted when it comes from the current next hop with a different metric, or when its metric is lower.**

> "If the datagram is from the
> same router as the existing route, and the new metric is different" "than the old one;
> or, if the new metric is lower than the old one; do
> the following actions:" — §3.9.2, `rfc2453.txt:1509-1511`, `rfc2453.txt:1519-1520`;
> then "Adopt the route from the datagram", set the change flag, signal a triggered update,
> and start the deletion at infinity or restart the timeout — `rfc2453.txt:1522-1529`

- Strength: description. Class: wire.
- Check idea: a route gets worse at its next hop, and the receiver takes the worse metric;
  a better path appears through another neighbor, and the receiver takes it.

### RFC2453-RESP-10

**The deletion starts only when the metric first becomes 16.**

> "Note that the
> deletion process is started only when the metric is first set to
> infinity.  If the metric was already infinity, then a new deletion
> process is not started." — §3.9.2, `rfc2453.txt:1532-1535`

- Strength: description. Class: wire.
- Check idea: a withdrawn route that keeps arriving with metric 16 still leaves the updates
  120 seconds after the first withdrawal.

### RFC2453-RESP-11

**With an equal metric, a router may switch to the new route when the current one is at least halfway to its timeout.**

> "Therefore, if the new metric is the same as the old one,
> examine the timeout for the existing route.  If it is at least
> halfway to the expiration point, switch to the new route.  This
> heuristic is optional, but highly recommended." — §3.9.2, `rfc2453.txt:1546-1549`

- Strength: should ("optional, but highly recommended"). Class: wire.
- Check idea: two equal paths; the current next hop goes silent; the router switches to the
  other path before the timeout expires.

### RFC2453-RESP-12

**Any other entry is ignored: an equal or worse route from a router that is not the current next hop changes nothing.**

> "Any entry that fails these tests is ignored, as it is no better than
> the current route." — §3.9.2, `rfc2453.txt:1551-1552`

- Strength: description. Class: wire.
- Check idea: a router with two paths of different length keeps the shorter one when the
  longer one is advertised.

## Output processing

### RFC2453-OUT-1

**The response to a request is unicast to the requester.**

> "- By input processing, when a Request is received (this Response is
> unicast to the requestor; see section 3.7.1)" — §3.10, `rfc2453.txt:1560-1561`

- Strength: description. Class: wire.
- Check idea: the answer to the request of a router that starts goes to the unicast address
  of that router.

### RFC2453-OUT-2

**Regular and triggered updates go to all neighbors: one response for each directly-connected network, multicast for version 2.**

> "When a Response is to be sent to all neighbors (i.e., a regular or
> triggered update), a Response message is directed to the router at
> the far end of each connected point-to-point link, and is broadcast
> (multicast for RIP-2) on all connected networks which support
> broadcasting.  Thus, one Response is prepared for each directly-
> connected network, and sent to the appropriate address (direct or
> broadcast/multicast)." — §3.10, `rfc2453.txt:1575-1581`

- Strength: description. Class: wire.
- Check idea: a router with two networks sends each periodic update on both of them, to the
  multicast address.

## Triggered updates

### RFC2453-TRIG-1

**A router sends a triggered update for a deleted route, and it may send one for a new or changed route.**

> "The router requirements RFC [11] specifies that all implementation of
> RIP must implement triggered update for deleted routes and may
> implement triggered updates for new routes or change of routes." — §3.4.4,
> `rfc2453.txt:966-968`

- Strength: must (deleted routes), may (new and changed routes). Class: wire.
- Check idea: a network behind a router goes away; the router sends the route with metric
  16 within a few seconds, well before the next periodic update.

### RFC2453-TRIG-2

**A router limits the rate of its triggered updates.**

> "RIP
> implementations must also limit the rate which of triggered updates
> may be trandmitted." — §3.4.4, `rfc2453.txt:968-970`; "Therefore, the protocol requires
> that implementors include provisions
> to limit the frequency of triggered updates." — §3.10.1, `rfc2453.txt:1595-1596`

- Strength: must. Class: wire.
- Check idea: many route changes in a short time give fewer triggered updates than changes,
  and two triggered updates on one network are at least one second apart.

### RFC2453-TRIG-3

**After a triggered update, a random timer of 1 to 5 seconds collects further changes into one update.**

> "After a triggered
> update is sent, a timer should be set for a random interval between 1
> and 5 seconds.  If other changes that would trigger updates occur
> before the timer expires, a single update is triggered when the timer
> expires.  The timer is then reset to another random value between 1
> and 5 seconds." — §3.10.1, `rfc2453.txt:1596-1601`

- Strength: should. Class: wire.
- Check idea: the intervals between consecutive triggered updates lie between 1 and 5
  seconds. That the value is random is a distribution.

### RFC2453-TRIG-4

**A triggered update is suppressed when a regular update is due by the time it would be sent.**

> "A triggered update should be suppressed if a regular
> update is due by the time the triggered update would be sent." — §3.10.1,
> `rfc2453.txt:1601-1602`

- Strength: should. Class: wire.
- Check idea: a change just before a periodic update gives no separate triggered update.

### RFC2453-TRIG-5

**A triggered update holds at least the routes whose change flag is set; a complete table is strongly discouraged.**

> "Therefore, messages generated as part of a triggered
> update must include at least those routes that have their route
> change flag set.  They may include additional routes, at the
> discretion of the implementor; however, sending complete routing
> updates is strongly discouraged." — §3.10.1, `rfc2453.txt:1606-1610`

- Strength: must (the changed routes), may (more), "strongly discouraged" (the whole table).
  Class: wire.
- Check idea: when one route changes in a table of several, the triggered update holds that
  route and not the whole table.

### RFC2453-TRIG-6

**A triggered update goes out on every directly-connected network, after split horizon.**

> "When a triggered update is
> processed, messages should be generated for every directly-connected
> network.  Split Horizon processing is done when generating triggered
> updates as well as normal updates" — §3.10.1, `rfc2453.txt:1610-1613`

- Strength: should (every network), description (split horizon). Class: wire.
- Check idea: a deleted route appears in triggered updates on each network of the router,
  subject to split horizon.

### RFC2453-TRIG-7

**A changed route that split horizon leaves unchanged on a network need not be sent there, and an empty update may be left out.**

> "If, after Split
> Horizon processing for a given network, a changed route will appear
> unchanged on that network (e.g., it appears with an infinite metric),
> the route need not be sent.  If no routes need be sent on that
> network, the update may be omitted." — §3.10.1, `rfc2453.txt:1613-1617`

- Strength: may. Class: wire.
- Check idea: a permission; it widens the observation of RFC2453-TRIG-6.

### RFC2453-TRIG-8

**After the triggered updates are generated, the change flags are cleared.**

> "Once all of the triggered
> updates have been generated, the route change flags should be
> cleared." — §3.10.1, `rfc2453.txt:1617-1619`

- Strength: should. Class: wire.
- Check idea: two changes some seconds apart; the second triggered update does not repeat
  the route of the first one.

### RFC2453-TRIG-9

**The rules of the next section apply to triggered updates as to all other updates.**

> "The only difference between a triggered update and other update
> messages is the possible omission of routes that have not changed.
> The remaining mechanisms, described in the next section, must be
> applied to all updates." — §3.10.1, `rfc2453.txt:1636-1639`

- Strength: must. Class: wire.
- Check idea: a triggered update has the same header, ports, address and entry rules as a
  periodic one.

## Generating a response

### RFC2453-GEN-1

**The version of a response is 1 or 2, and the response to a request should carry the version of the request.**

> "Set the version number to either 1 or 2.  The mechanism for deciding
> which version to send is implementation specific; however, if this is
> the Response to a Request, the Response version should match the
> Request version." — §3.10.2, `rfc2453.txt:1646-1649`

- Strength: should. Class: wire.
- Check idea: the answer to a version 2 request is a version 2 response.

### RFC2453-GEN-2

**A response carries command 2 and zero in the octets that must be zero.**

> "Set the command to Response.  Set the bytes labeled
> "must be zero" to zero." — §3.10.2, `rfc2453.txt:1649-1650`

- Strength: description. Class: wire.
- Check idea: every update carries command 2 and zero in the two unused octets of the header.

### RFC2453-GEN-3

**A response holds at most 25 entries; more routes go into further messages.**

> "Recall that there is
> a limit of 25 RTEs to a Response; if there are more, send the current
> Response and start a new one.  There is no defined limit to the
> number of datagrams which make up a Response." — §3.10.2, `rfc2453.txt:1650-1653`

- Strength: description. Class: wire.
- Check idea: a router with 30 routes sends its periodic update as two messages, and neither
  holds more than 25 entries; together they hold all 30.

### RFC2453-GEN-4

**A triggered update need hold only the changed routes; a route that split horizon excludes is skipped; an included route puts its destination and metric into an entry.**

> "If a
> triggered update is being generated, only entries whose route change
> flags are set need be included.  If, after Split Horizon processing,
> the route should not be included, skip it.  If the route is to be
> included, then the destination address and metric are put into the
> RTE." — §3.10.2, `rfc2453.txt:1655-1660`

- Strength: description. Class: wire.
- Check idea: the entry of a route carries its destination and its current metric.

### RFC2453-GEN-5

**A route with metric 16 is included in the updates too.**

> "Routes must be included in the datagram even if their metrics
> are infinite." — §3.10.2, `rfc2453.txt:1660-1661`

- Strength: must. Class: wire.
- Check idea: a deleted route, and a route that poisoned reverse sets to 16, both appear in
  the updates with metric 16.

## Route tag

### RFC2453-TAG-1

**A route keeps its route tag, and the tag is advertised with it.**

> "The Route Tag (RT) field is an attribute assigned to a route which
> must be preserved and readvertised with a route." — §4.2, `rfc2453.txt:1760-1761`

- Strength: must. Class: wire.
- Check idea: a response offers a route with tag 7; the receiver advertises the route on
  with tag 7.

### RFC2453-TAG-2

**A router that imports routes from other protocols can set the route tag of the imported routes.**

> "Routers supporting protocols other than RIP should be configurable to
> allow the Route Tag to be configured for routes imported from
> different sources." — §4.2, `rfc2453.txt:1767-1769`

- Strength: should. Class: wire.
- Check idea: a static route imported with a configured tag is advertised with that tag.

## Subnet mask

### RFC2453-MASK-1

**The subnet mask field holds the mask of the entry, and zero means that no mask is included.**

> "The Subnet Mask field contains the subnet mask which is applied to
> the IP address to yield the non-host portion of the address.  If this
> field is zero, then no subnet mask has been included for this entry." — §4.3,
> `rfc2453.txt:1781-1783`

- Strength: description. Class: wire.
- Check idea: each entry of a version 2 update carries the mask of its network, for example
  255.255.255.0 for a /24.

### RFC2453-MASK-2

**Where a version 1 router can hear version 2 entries, three rules limit what is advertised.**

> "On an interface where a RIP-1 router may hear and operate on the
> information in a RIP-2 routing entry the following rules apply:" — §4.3,
> `rfc2453.txt:1785-1786`, followed by the three rules at `rfc2453.txt:1788-1789`,
> `rfc2453.txt:1799-1800` and `rfc2453.txt:1802-1804`: information internal to one network
> "must never be advertised into another network", a more specific subnet "may not be
> advertised where RIP-1 routers would consider it a host route", and supernet routes "must
> not be advertised where they could be misinterpreted by RIP-1 routers".

- Strength: must not, may not. Class: wire.
- Check idea: a link with a version 1 router; the updates on it hold no subnet and no
  supernet route. The statement needs a version 1 router in the network.

## Next hop

### RFC2453-NH-1

**A next hop of 0.0.0.0 means routing through the router that sent the advertisement.**

> "Specifying a
> value of 0.0.0.0 in this field indicates that routing should be via
> the originator of the RIP advertisement." — §4.4, `rfc2453.txt:1809-1811`

- Strength: description. Class: end-to-end.
- Check idea: an update with next hop 0.0.0.0; the receiver forwards to the sender of the
  update.

### RFC2453-NH-2

**A next hop that an entry names is directly reachable on the subnet where the entry is advertised.**

> "An address specified as a
> next hop must, per force, be directly reachable on the logical subnet
> over which the advertisement is made." — §4.4, `rfc2453.txt:1811-1813`

- Strength: must. Class: wire.
- Check idea: every entry of an update carries next hop 0.0.0.0 or an address on the subnet
  of the link that carries the update.

### RFC2453-NH-3

**The next hop is advisory: a receiver that ignores it still has a valid route.**

> "Note that Next Hop is an
> "advisory" field.  That is, if the provided information is ignored, a
> possibly sub-optimal, but absolutely valid, route may be taken." — §4.4,
> `rfc2453.txt:1818-1821`

- Strength: description. Class: end-to-end.
- Check idea: a permission; it lets a receiver forward through the sender of an update even
  when the entry names another next hop.

### RFC2453-NH-4

**A received next hop that is not directly reachable is treated as 0.0.0.0.**

> "If the received Next Hop is not directly reachable, it should be treated
> as 0.0.0.0." — §4.4, `rfc2453.txt:1820-1822`

- Strength: should. Class: end-to-end.
- Check idea: a response names a next hop on a foreign subnet; the receiver forwards to the
  sender of the response.

## Multicast

### RFC2453-MCAST-1

**Periodic updates go to the IP multicast address 224.0.0.9.**

> "In order to reduce unnecessary load on those hosts which are not
> listening to RIP-2 messages, an IP multicast address will be used for
> periodic broadcasts.  The IP multicast address is 224.0.0.9." — §4.5,
> `rfc2453.txt:1826-1828`

- Strength: description. Class: wire.
- Check idea: every periodic update carries destination address 224.0.0.9.

### RFC2453-MCAST-2

**RIP multicasts stay on their link.**

> "Note
> that IGMP is not needed since these are inter-router messages which
> are not forwarded." — §4.5, `rfc2453.txt:1828-1830`

- Strength: description. Class: wire.
- Check idea: an update that one router multicasts on one network is not forwarded onto any
  other network.

### RFC2453-MCAST-3

**On a network without broadcast, unicast may be used, and a response to the multicast address is accepted.**

> "On NBMA networks, unicast addressing may be used.  However, if a
> response addressed to the RIP-2 multicast address is received, it
> should be accepted." — §4.5, `rfc2453.txt:1832-1834`

- Strength: may (unicast), should (accept). Class: end-to-end.
- Check idea: none at this level; a network without broadcast is outside the mockups of
  this pass.

### RFC2453-MCAST-4

**Multicast is a configuration choice, and when used it is used on every interface that can.**

> "In order to maintain backwards compatibility, the use of the
> multicast address will be configurable, as described in section 5.1.
> If multicasting is used, it should be used on all interfaces which
> support it." — §4.5, `rfc2453.txt:1836-1839`

- Strength: should. Class: wire.
- Check idea: a router with two broadcast networks multicasts its updates on both.

## Queries of version 1

### RFC2453-QRY-1

**A version 2 router answers a version 1 request with a version 1 response, unless it is set to send version 2 only.**

> "If a RIP-2 router receives a RIP-1 Request, it should respond with a
> RIP-1 Response.  If the router is configured to send only RIP-2
> messages, it should not respond to a RIP-1 Request." — §4.6, `rfc2453.txt:1843-1845`

- Strength: should, should not. Class: wire.
- Check idea: a version 1 request reaches a router; the answer is a version 1 response, or
  no answer when the router sends version 2 only.

## Out of scope in this catalog

The entries above hold every "must", "should" and "may" of §3 and of §4.2 to §4.6 that states
a behavior of a router, and every field of the message. What is left out, and why:

- **The design discussion of §3.1 to §3.4.2**, `rfc2453.txt:158-808`. It explains the
  distance vector algorithm and counting to infinity. The rules it motivates are stated
  again in §3.5 to §3.10, and the entries are there. Two sentences of §3.4.3 and §3.4.4 that
  state a rule of their own, the split horizon and triggered-update rules of RFC 1812, have
  entries: [RFC2453-SH-1](#rfc2453-sh-1), [RFC2453-TRIG-1](#rfc2453-trig-1) and
  [RFC2453-TRIG-2](#rfc2453-trig-2).
- **The limits of §3.2**, `rfc2453.txt:236-269`: a diameter of 15, counting to infinity,
  fixed metrics. They say what the protocol cannot do, and they demand nothing.
- **The administrative statements**: every router of an AS must take part, and a router must
  leak routes between IGPs, `rfc2453.txt:1078-1081`; routes to 0.0.0.0 should not leave the
  AS, `rfc2453.txt:1284-1296`; a list of neighbors for networks without broadcast is left to
  the implementor, `rfc2453.txt:1582-1588`; the uses of the route tag, `rfc2453.txt:1774-1777`.
  They say how a network is set up, not what a message holds.
- **The interlocking of input and output**, `rfc2453.txt:1631-1634`. It is a condition on
  concurrent input and output processing inside one router, and no message exchange can
  observe it.
- **Authentication, §4.1**, `rfc2453.txt:1716-1756`, and **the compatibility of §5**,
  `rfc2453.txt:1855-1961`: out of the in-scope set; see
  [`standards.md`](../../protocol/rip/standards.md#in-scope-set).
