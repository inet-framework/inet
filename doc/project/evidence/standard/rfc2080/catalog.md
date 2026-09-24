# RFC 2080 (RIPng for IPv6) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC2080-*` · **Stands on:** [standards.md](../../protocol/rip/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for the IPv6 base
document of the in-scope set: RFC 2080, RIPng for IPv6, of January 1997. The catalog comes
from the RFC text only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc2080.txt`](../../../../../../standards/RFC/rfc2080.txt) —
  RIPng for IPv6, January 1997. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc2080.txt>.

The IPv4 twin of this document is RFC 2453, in [`rfc2453/catalog.md`](../rfc2453/catalog.md).
RFC 2080 is the older of the two, and it takes the theory of the protocol from RFC 1058
without restating it; §2 of RFC 2080 holds every rule. The family of documents and the
in-scope set are in [`standards.md`](../../protocol/rip/standards.md). The features these
statements build are in [`features.md`](../../protocol/rip/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`rip/coverage.md`](../../model/rip/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc2080.txt:247-249` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

## How to read an entry

- **ID** — `RFC2080-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 2080 does not use the keywords of RFC 2119:
  its "must", "should" and "may" are all in lower case. The entry records the word as
  written: `must`, `must not`, `should`, `should not`, `may`, or `description` for a
  normative sentence with no keyword. Where the text says "required", "preferred" or
  "strongly discouraged", the entry names the word.
- **Class** — how a test can observe the statement: `wire` (fields of messages on a link),
  `end-to-end` (what a router accepts and acts on, seen in what it sends or forwards
  afterwards), `error-signal` (a report message), `internal` (state inside a router),
  `encoding` (the exact bit layout).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC2080-MSG-1](#rfc2080-msg-1) | A router sends and receives RIPng messages on UDP port 521. |
| [RFC2080-MSG-2](#rfc2080-msg-2) | A message for the RIPng process of another router goes to UDP port 521. |
| [RFC2080-MSG-3](#rfc2080-msg-3) | Every routing update leaves the RIPng port, and an unsolicited update has 521 as both ports. |
| [RFC2080-MSG-4](#rfc2080-msg-4) | A response to a request goes to the port that the request came from. |
| [RFC2080-MSG-5](#rfc2080-msg-5) | A specific query may come from another port, and it goes to the RIPng port of the target. |
| [RFC2080-MSG-6](#rfc2080-msg-6) | A RIPng message is a 4-octet header — command, version, two octets that must be zero — and a list of 20-octet entries: a 16-octet prefix, a 2-octet route tag, a 1-octet prefix length and a 1-octet metric. |
| [RFC2080-MSG-7](#rfc2080-msg-7) | The fields are binary integers in network byte order. |
| [RFC2080-MSG-8](#rfc2080-msg-8) | Command 1 is a request and command 2 is a response. |
| [RFC2080-MSG-9](#rfc2080-msg-9) | The destination prefix is a 128-bit IPv6 prefix in 16 octets. |
| [RFC2080-MSG-10](#rfc2080-msg-10) | The prefix length is the number of significant bits of the prefix, from 0 to 128. |
| [RFC2080-MSG-11](#rfc2080-msg-11) | The MTU of the medium limits the size of a message, and so the number of entries in it. |
| [RFC2080-MET-1](#rfc2080-met-1) | The metric of a network is an integer from 1 to 15. |
| [RFC2080-MET-2](#rfc2080-met-2) | The administrator can set the metric of each network. |
| [RFC2080-MET-3](#rfc2080-met-3) | The metric of a directly-connected network is the cost of that network. |
| [RFC2080-MET-4](#rfc2080-met-4) | The metric field holds 1 to 15, or 16, which means that the destination is not reachable. |
| [RFC2080-MET-5](#rfc2080-met-5) | A received metric gets the cost of the arrival network added, and the result stops at 16. |
| [RFC2080-TABLE-1](#rfc2080-table-1) | Each route holds the destination prefix, the metric, the next hop, a route change flag and its timers. |
| [RFC2080-TABLE-2](#rfc2080-table-2) | The next hop of a learned route is the router that sent the update, or the address of a next hop entry. |
| [RFC2080-TAG-1](#rfc2080-tag-1) | A route keeps its route tag, and the tag is advertised with it. |
| [RFC2080-TAG-2](#rfc2080-tag-2) | A router that imports routes from other protocols can set the route tag of the imported routes. |
| [RFC2080-NH-1](#rfc2080-nh-1) | A next hop entry applies to the route entries that follow it, up to the end of the message or the next next hop entry. |
| [RFC2080-NH-2](#rfc2080-nh-2) | A next hop entry has 0xFF in its metric field and the next hop address in its prefix field. |
| [RFC2080-NH-3](#rfc2080-nh-3) | The route tag and the prefix length of a next hop entry are zero on sending and ignored on receipt. |
| [RFC2080-NH-4](#rfc2080-nh-4) | A next hop of 0:0:0:0:0:0:0:0 means routing through the router that sent the advertisement. |
| [RFC2080-NH-5](#rfc2080-nh-5) | A next hop is a link-local address. |
| [RFC2080-NH-6](#rfc2080-nh-6) | The next hop is advisory: a receiver that ignores it still has a valid route. |
| [RFC2080-NH-7](#rfc2080-nh-7) | A received next hop that is not link-local is treated as 0:0:0:0:0:0:0:0. |
| [RFC2080-ADDR-1](#rfc2080-addr-1) | A prefix of length zero designates a default route. |
| [RFC2080-ADDR-2](#rfc2080-addr-2) | RIPng handles a default route like any other prefix. |
| [RFC2080-ADDR-3](#rfc2080-addr-3) | The administrator can choose the metric of an advertised default route. |
| [RFC2080-SH-1](#rfc2080-sh-1) | Split horizon omits a route in the updates to the neighbor it came from; on a broadcast network, it omits every route learned from that network in the updates sent on it. |
| [RFC2080-SH-2](#rfc2080-sh-2) | Poisoned reverse sends such routes with metric 16, and it is the preferred method. |
| [RFC2080-SH-3](#rfc2080-sh-3) | Each interface can be set to no split horizon, split horizon, or poisoned reverse. |
| [RFC2080-TIMER-1](#rfc2080-timer-1) | Every 30 seconds a router sends an unsolicited response with its whole table to every neighbor. |
| [RFC2080-TIMER-2](#rfc2080-timer-2) | The 30-second updates are kept from synchronizing: by a clock that load does not change, or by a random offset of up to 15 seconds. |
| [RFC2080-TIMER-3](#rfc2080-timer-3) | A route has a timeout and a garbage-collection timer; after the timeout the route is invalid but stays in the table for a short time. |
| [RFC2080-TIMER-4](#rfc2080-timer-4) | The timeout starts when the route is set up and again on each update for it; after 180 seconds the route expires. |
| [RFC2080-TIMER-5](#rfc2080-timer-5) | On deletion — by timeout, or by metric 16 from the current next hop — the route gets metric 16, the garbage-collection timer of 120 seconds, the change flag, and a triggered update. |
| [RFC2080-TIMER-6](#rfc2080-timer-6) | Until the garbage-collection timer expires, the route is in every update; then it is removed. |
| [RFC2080-TIMER-7](#rfc2080-timer-7) | A new route that arrives during garbage collection replaces the old one and clears the garbage-collection timer. |
| [RFC2080-REQ-1](#rfc2080-req-1) | A router that has just come up sends requests from the RIPng port, by multicast. |
| [RFC2080-REQ-2](#rfc2080-req-2) | A request for the table of one router should come from another port, and the router answers to the address and port of the requester, from a global address. |
| [RFC2080-REQ-3](#rfc2080-req-3) | A request with no entries gets no response. |
| [RFC2080-REQ-4](#rfc2080-req-4) | A request with one entry of prefix zero, prefix length zero and metric 16 asks for the whole table, which goes to the requesting address and port. |
| [RFC2080-REQ-5](#rfc2080-req-5) | For a specific request, each entry gets the metric of the route, or 16 if there is none, and the message goes back as a response. |
| [RFC2080-REQ-6](#rfc2080-req-6) | The answer to a whole-table request goes through normal output processing, split horizon included. |
| [RFC2080-REQ-7](#rfc2080-req-7) | The answer to a specific request is the table as it is, without split horizon. |
| [RFC2080-REQ-8](#rfc2080-req-8) | A router that comes up multicasts a request for the complete table on every connected network. |
| [RFC2080-RESP-1](#rfc2080-resp-1) | A response is processed the same way whatever caused it. |
| [RFC2080-RESP-2](#rfc2080-resp-2) | A response that does not come from port 521 is ignored. |
| [RFC2080-RESP-3](#rfc2080-resp-3) | The source of a response must be a link-local address. |
| [RFC2080-RESP-4](#rfc2080-resp-4) | A router ignores its own messages. |
| [RFC2080-RESP-5](#rfc2080-resp-5) | A periodic advertisement carries hop limit 255. |
| [RFC2080-RESP-6](#rfc2080-resp-6) | An inbound multicast from the RIPng port with a hop limit other than 255 is not from a neighbor; queries and their answers need no such test. |
| [RFC2080-RESP-7](#rfc2080-resp-7) | An entry with a multicast or link-local prefix, a prefix length above 128, or a metric outside 1 to 16 is ignored, and the next entry is processed. |
| [RFC2080-RESP-8](#rfc2080-resp-8) | A new destination with a metric below 16 is added, with a fresh timeout, the change flag and a triggered update. |
| [RFC2080-RESP-9](#rfc2080-resp-9) | A new destination that arrives with metric 16 is not added. |
| [RFC2080-RESP-10](#rfc2080-resp-10) | An update for an existing route from its current next hop restarts the timeout. |
| [RFC2080-RESP-11](#rfc2080-resp-11) | The route from the datagram is adopted when it comes from the current next hop with a different metric, or when its metric is lower. |
| [RFC2080-RESP-12](#rfc2080-resp-12) | The deletion starts only when the metric first becomes 16. |
| [RFC2080-RESP-13](#rfc2080-resp-13) | With an equal metric, a router may switch to the new route when the current one is at least halfway to its timeout. |
| [RFC2080-RESP-14](#rfc2080-resp-14) | Any other entry is ignored: an equal or worse route from a router that is not the current next hop changes nothing. |
| [RFC2080-OUT-1](#rfc2080-out-1) | The response to a request is unicast to the requester only. |
| [RFC2080-OUT-2](#rfc2080-out-2) | Every 30 seconds the whole table goes to every neighbor, and every change of a metric triggers an update. |
| [RFC2080-OUT-3](#rfc2080-out-3) | Regular and triggered updates are multicast to FF02::9, one response for each directly-connected network. |
| [RFC2080-TRIG-1](#rfc2080-trig-1) | A router limits the rate of its triggered updates. |
| [RFC2080-TRIG-2](#rfc2080-trig-2) | After a triggered update, a random timer of 1 to 5 seconds collects further changes into one update. |
| [RFC2080-TRIG-3](#rfc2080-trig-3) | A triggered update may be suppressed when a regular update is due by the time it would be sent. |
| [RFC2080-TRIG-4](#rfc2080-trig-4) | A triggered update holds at least the routes whose change flag is set; a complete table is strongly discouraged. |
| [RFC2080-TRIG-5](#rfc2080-trig-5) | A triggered update goes out on every directly-connected network, after split horizon. |
| [RFC2080-TRIG-6](#rfc2080-trig-6) | A changed route that split horizon leaves unchanged on a network need not be sent there, and an empty update may be left out. |
| [RFC2080-TRIG-7](#rfc2080-trig-7) | After the triggered updates are generated, the change flags are cleared. |
| [RFC2080-TRIG-8](#rfc2080-trig-8) | The rules of the next section apply to triggered updates as to all other updates. |
| [RFC2080-GEN-1](#rfc2080-gen-1) | A response leaves from a link-local address of the interface; the answer to a unicast request from another port leaves from a global address. |
| [RFC2080-GEN-2](#rfc2080-gen-2) | With several link-local addresses on an interface, a router sends from one designated address and keeps it while it is valid. |
| [RFC2080-GEN-3](#rfc2080-gen-3) | A response carries version 1, command 2, and zero in the octets that must be zero. |
| [RFC2080-GEN-4](#rfc2080-gen-4) | When a message is full for the MTU, the router sends it and starts another. |
| [RFC2080-GEN-5](#rfc2080-gen-5) | A route to a link-local address is never advertised. |
| [RFC2080-GEN-6](#rfc2080-gen-6) | A triggered update need hold only the changed routes; a route that split horizon excludes is skipped; an included route puts its prefix, prefix length, metric and route tag into an entry. |
| [RFC2080-GEN-7](#rfc2080-gen-7) | A route with metric 16 is included in the updates too. |

## Message format and transport

### RFC2080-MSG-1

**A router sends and receives RIPng messages on UDP port 521.**

> "Each router that uses RIPng has a
> routing process that sends and receives datagrams on UDP port number
> 521, the RIPng port." — §2.1, `rfc2080.txt:247-249`

- Strength: description. Class: wire.
- Check idea: every RIPng message that a router sends leaves UDP port 521.

### RFC2080-MSG-2

**A message for the RIPng process of another router goes to UDP port 521.**

> "All communications intended for another
> router's RIPng process are sent to the RIPng port." — §2.1, `rfc2080.txt:249-250`

- Strength: description. Class: wire.
- Check idea: a request and a periodic update of a router both carry destination port 521.

### RFC2080-MSG-3

**Every routing update leaves the RIPng port, and an unsolicited update has 521 as both ports.**

> "All routing
> update messages are sent from the RIPng port.  Unsolicited routing
> update messages have both the source and destination port equal to
> the RIPng port." — §2.1, `rfc2080.txt:250-253`

- Strength: description. Class: wire.
- Check idea: a periodic update and a triggered update carry source port 521 and
  destination port 521.

### RFC2080-MSG-4

**A response to a request goes to the port that the request came from.**

> "Those sent in response to a request are sent to the
> port from which the request came." — §2.1, `rfc2080.txt:253-254`

- Strength: description. Class: wire.
- Check idea: the response to a whole-table request that came from port 521 goes to port
  521.

### RFC2080-MSG-5

**A specific query may come from another port, and it goes to the RIPng port of the target.**

> "Specific queries may be sent from
> ports other than the RIPng port, but they must be directed to the
> RIPng port on the target machine." — §2.1, `rfc2080.txt:254-256`

- Strength: may (the source port), must (the destination port). Class: wire.
- Check idea: a request that a router sends carries destination port 521.

### RFC2080-MSG-6

**A RIPng message is a 4-octet header — command, version, two octets that must be zero — and a list of 20-octet entries: a 16-octet prefix, a 2-octet route tag, a 1-octet prefix length and a 1-octet metric.**

> "|  command (1)  |  version (1)  |       must be zero (2)        |" — §2.1,
> `rfc2080.txt:263`; "~                        IPv6 prefix (16)                       ~" and
> "|         route tag (2)         | prefix len (1)|  metric (1)   |" — `rfc2080.txt:292`,
> `rfc2080.txt:295`

- Strength: description. Class: encoding.
- Check idea: the octets of a message: one octet of command, one of version, two zero
  octets, then entries of 20 octets each, with the metric in the last octet.

### RFC2080-MSG-7

**The fields are binary integers in network byte order.**

> "Unless otherwise specified, fields
> contain binary integers, in network byte order, with the most-
> significant octet first (big-endian)." — §2.1, `rfc2080.txt:300-302`

- Strength: description. Class: encoding.
- Check idea: the two octets of a route tag of 7 are `00 07`.

### RFC2080-MSG-8

**Command 1 is a request and command 2 is a response.**

> "1 - request    A request for the responding system to send all or
> part of its routing table." and "2 - response   A message containing all or part of the
> sender's routing table." — §2.1, `rfc2080.txt:310-314`

- Strength: description. Class: wire.
- Check idea: the request at startup carries command 1, and every update carries command 2.

### RFC2080-MSG-9

**The destination prefix is a 128-bit IPv6 prefix in 16 octets.**

> "The destination prefix is the usual 128-bit, IPv6 address prefix
> stored as 16 octets in network byte order." — §2.1, `rfc2080.txt:323-324`

- Strength: description. Class: encoding.
- Check idea: the first 16 octets of an entry are the prefix, most significant octet first.

### RFC2080-MSG-10

**The prefix length is the number of significant bits of the prefix, from 0 to 128.**

> "The prefix length field is the length in bits of the significant part
> of the prefix (a value between 0 and 128 inclusive) starting from the
> left of the prefix." — §2.1, `rfc2080.txt:352-354`

- Strength: description. Class: wire.
- Check idea: the entry of a /64 network carries prefix length 64.

### RFC2080-MSG-11

**The MTU of the medium limits the size of a message, and so the number of entries in it.**

> "The maximum datagram size is limited by the MTU of the medium over
> which the protocol is being used." — §2.1, `rfc2080.txt:360-361`; "The determination of
> the number of RTEs which may be put
> into a given message is a function of the medium's MTU, the number of
> octets of header information preceeding the RIPng message, the size
> of the RIPng header, and the size of an RTE." — `rfc2080.txt:363-366`

- Strength: description. Class: wire.
- Check idea: a router whose table does not fit into one message of the link MTU sends its
  update in more than one message, and no message is larger than the MTU allows.

## Metric

### RFC2080-MET-1

**The metric of a network is an integer from 1 to 15.**

> "The RIPng metric of a network
> is an integer between 1 and 15, inclusive." — §2, `rfc2080.txt:184-185`

- Strength: description. Class: wire.
- Check idea: the metric that a router advertises for a network it reaches is between 1
  and 15.

### RFC2080-MET-2

**The administrator can set the metric of each network.**

> "Implementations should allow
> the system administrator to set the metric of each network." — §2, `rfc2080.txt:187-188`

- Strength: should. Class: wire.
- Check idea: a network with a configured cost of 3 is advertised with metric 3 by the
  router that is directly connected to it.

### RFC2080-MET-3

**The metric of a directly-connected network is the cost of that network.**

> "The metric for a directly-connected network is set to the
> cost of that network." — §2, `rfc2080.txt:218-219`

- Strength: description. Class: wire.
- Check idea: a router advertises each network it is attached to with the cost of that
  network, 1 when no cost is configured.

### RFC2080-MET-4

**The metric field holds 1 to 15, or 16, which means that the destination is not reachable.**

> "The metric field contains a value between 1 and 15 inclusive,
> specifying the current metric for the destination; or the value 16
> (infinity), which indicates that the destination is not reachable." — §2.1,
> `rfc2080.txt:356-358`

- Strength: description. Class: wire.
- Check idea: no route entry of any update carries a metric of 0, or a metric above 16 other
  than the 0xFF of a next hop entry.

### RFC2080-MET-5

**A received metric gets the cost of the arrival network added, and the result stops at 16.**

> "Once the entry has been validated, update the metric by adding the
> cost of the network on which the message arrived.  If the result is
> greater than infinity, use infinity." — §2.4.2, `rfc2080.txt:652-654`

- Strength: description. Class: wire.
- Check idea: in a chain of routers, each router advertises a remote prefix with a metric
  one higher than the router before it.

## Routing table

### RFC2080-TABLE-1

**Each route holds the destination prefix, the metric, the next hop, a route change flag and its timers.**

> "Each entry contains at least the following information:" — §2,
> `rfc2080.txt:196-197`, followed by the five items at `rfc2080.txt:199-214`

- Strength: description. Class: internal.
- Check idea: none of its own. Each item shows in the messages that other entries cover.

### RFC2080-TABLE-2

**The next hop of a learned route is the router that sent the update, or the address of a next hop entry.**

> "- Set the next hop address to be the address of the router from which
> the datagram came or the next hop address specified by a next hop
> RTE." — §2.4.2, `rfc2080.txt:678-680`

- Strength: description. Class: end-to-end.
- Check idea: a host behind a router reaches a network two routers away, which is possible
  only when each router forwards to the router it learned the route from.

## Route tag

### RFC2080-TAG-1

**A route keeps its route tag, and the tag is advertised with it.**

> "The route tag field is an attribute assigned to a route which must be
> preserved and readvertised with a route." — §2.1, `rfc2080.txt:326-327`

- Strength: must. Class: wire.
- Check idea: a response offers a prefix with tag 7; the receiver advertises the prefix on
  with tag 7.

### RFC2080-TAG-2

**A router that imports routes from other protocols can set the route tag of the imported routes.**

> "Routers supporting protocols other than RIPng should be configurable
> to allow the route tag to be configured for routes imported from
> different sources." — §2.1, `rfc2080.txt:342-344`

- Strength: should. Class: wire.
- Check idea: a static route imported with a configured tag is advertised with that tag.

## Next hop

### RFC2080-NH-1

**A next hop entry applies to the route entries that follow it, up to the end of the message or the next next hop entry.**

> "Therefore, in RIPng, the next hop is specified by a special
> RTE and applies to all of the address RTEs following the next hop RTE
> until the end of the message or until another next hop RTE is
> encountered." — §2.1.1, `rfc2080.txt:381-384`

- Strength: description. Class: encoding.
- Check idea: a message with two next hop entries; the receiver uses the first next hop for
  the routes between them and the second for the rest.

### RFC2080-NH-2

**A next hop entry has 0xFF in its metric field and the next hop address in its prefix field.**

> "A next hop RTE is identified by a value of 0xFF in the metric field
> of an RTE.  The prefix field specifies the IPv6 address of the next
> hop." — §2.1.1, `rfc2080.txt:386-388`

- Strength: description. Class: encoding.
- Check idea: the octets of a next hop entry end in `FF`.

### RFC2080-NH-3

**The route tag and the prefix length of a next hop entry are zero on sending and ignored on receipt.**

> "The route tag and prefix length in the next hop RTE must be set
> to zero on sending and ignored on receiption." — §2.1.1, `rfc2080.txt:388-389`

- Strength: must. Class: encoding.
- Check idea: the route tag and the prefix length of a next hop entry are zero.

### RFC2080-NH-4

**A next hop of 0:0:0:0:0:0:0:0 means routing through the router that sent the advertisement.**

> "Specifying a value of 0:0:0:0:0:0:0:0 in the prefix field of a next
> hop RTE indicates that the next hop address should be the originator
> of the RIPng advertisement." — §2.1.1, `rfc2080.txt:410-412`

- Strength: description. Class: end-to-end.
- Check idea: a next hop entry of all zeros; the receiver forwards to the sender of the
  update.

### RFC2080-NH-5

**A next hop is a link-local address.**

> "An address specified as a next hop must
> be a link-local address." — §2.1.1, `rfc2080.txt:412-413`

- Strength: must. Class: wire.
- Check idea: every next hop that a router advertises is a link-local address.

### RFC2080-NH-6

**The next hop is advisory: a receiver that ignores it still has a valid route.**

> "Note that
> next hop RTE is "advisory".  That is, if the provided information is
> ignored, a possibly sub-optimal, but absolutely valid, route may be
> taken." — §2.1.1, `rfc2080.txt:417-420`

- Strength: description. Class: end-to-end.
- Check idea: a permission; it lets a receiver forward through the sender of an update even
  when a next hop entry names another router.

### RFC2080-NH-7

**A received next hop that is not link-local is treated as 0:0:0:0:0:0:0:0.**

> "If the received next hop address is not a link-local address,
> it should be treated as 0:0:0:0:0:0:0:0." — §2.1.1, `rfc2080.txt:420-421`

- Strength: should. Class: end-to-end.
- Check idea: a next hop entry names a global address; the receiver forwards to the sender
  of the update.

## Addressing

### RFC2080-ADDR-1

**A prefix of length zero designates a default route.**

> "Any prefix with a prefix length of zero is used to designate a
> default route.  It is suggested that the prefix 0:0:0:0:0:0:0:0 be
> used when specifying the default route, though the prefix is
> essentially ignored." — §2.2, `rfc2080.txt:428-431`

- Strength: description. Class: wire.
- Check idea: a default route is advertised with prefix length 0, and its prefix is all
  zeros.

### RFC2080-ADDR-2

**RIPng handles a default route like any other prefix.**

> "The default route entries are handled by RIPng in exactly
> the same manner as any other destination prefix." — §2.2, `rfc2080.txt:444-445`

- Strength: description. Class: wire.
- Check idea: a default route that one router originates reaches the next router and is
  advertised on with the metric increased.

### RFC2080-ADDR-3

**The administrator can choose the metric of an advertised default route.**

> "If this mechanism is used, the
> implementation should allow the network administrator to choose the
> metric associated with the default route advertisement." — §2.2, `rfc2080.txt:440-442`

- Strength: should. Class: wire.
- Check idea: a default route with a configured metric of 5 is advertised with metric 5.

## Split horizon

### RFC2080-SH-1

**Split horizon omits a route in the updates to the neighbor it came from; on a broadcast network, it omits every route learned from that network in the updates sent on it.**

> "The basic split horizon algorithm omits routes learned from
> one neighbor in updates sent to that neighbor.  In the case of a
> broadcast network, all routes learned from any neighbor on that
> network are omitted from updates sent on that network." — §2.6, `rfc2080.txt:877-880`

- Strength: description. Class: wire.
- Check idea: a route learned on a network does not appear with a finite metric in the
  updates that the router sends on that network.

### RFC2080-SH-2

**Poisoned reverse sends such routes with metric 16, and it is the preferred method.**

> "Split Horizon with Poisoned Reverse (more simply, Poison Reverse)
> does include such routes in updates, but sets their metrics to
> infinity.  In effect, advertising the fact that there routes are not
> reachable.  This is the preferred method of operation;" — §2.6, `rfc2080.txt:882-885`

- Strength: description ("the preferred method"). Class: wire.
- Check idea: the update on the network where a route was learned holds that route with
  metric 16.

### RFC2080-SH-3

**Each interface can be set to no split horizon, split horizon, or poisoned reverse.**

> "however,
> implementations should provide a per-interface control allowing no
> horizoning, split horizoning, and poisoned reverse to be selected." — §2.6,
> `rfc2080.txt:885-887`

- Strength: should. Class: wire.
- Check idea: the same network run three times, once with each setting on the interface;
  the reverse route is sent with its metric, left out, and sent with metric 16.

## Timers

### RFC2080-TIMER-1

**Every 30 seconds a router sends an unsolicited response with its whole table to every neighbor.**

> "Every 30 seconds, the RIPng process is awakened to send an
> unsolicited Response message, containing the complete routing table
> (see section 2.6 on Split Horizon), to every neighboring router." — §2.3,
> `rfc2080.txt:464-466`

- Strength: description. Class: wire.
- Check idea: the unsolicited responses of a router on a link come about 30 seconds apart,
  and each one holds every route of its table that split horizon lets through.

### RFC2080-TIMER-2

**The 30-second updates are kept from synchronizing: by a clock that load does not change, or by a random offset of up to 15 seconds.**

> "Therefore, implementations are required to take
> one of two precautions:" — §2.3, `rfc2080.txt:473-474`, followed by "The 30-second timer
> is offset by a small random time (+/- 0 to 15
> seconds) each time it is set.  The offset is derived from: 0.5 *
> the update period (i.e. 30)." — `rfc2080.txt:480-482`

- Strength: must ("required"). Class: wire.
- Check idea: the intervals between the periodic updates of a router stay between 15 and 45
  seconds. Whether the offset is random is a distribution.

### RFC2080-TIMER-3

**A route has a timeout and a garbage-collection timer; after the timeout the route is invalid but stays in the table for a short time.**

> "Upon expiration of the timeout, the route
> is no longer valid; however, it is retained in the routing table for
> a short time so that neighbors can be notified that the route has
> been dropped.  Upon expiration of the garbage-collection timer, the
> route is finally removed from the routing table." — §2.3, `rfc2080.txt:485-489`

- Strength: description. Class: wire.
- Check idea: after its neighbor goes silent, a route first appears with metric 16 in the
  updates and later disappears from them.

### RFC2080-TIMER-4

**The timeout starts when the route is set up and again on each update for it; after 180 seconds the route expires.**

> "The timeout is initialized when a route is established, and any time
> an update message is received for the route.  If 180 seconds elapse
> from the last time the timeout was initialized, the route is
> considered to have expired, and the deletion process described below
> begins for that route." — §2.3, `rfc2080.txt:491-495`

- Strength: description. Class: wire.
- Check idea: a neighbor stops without notice; 180 seconds after its last update, the route
  through it is withdrawn with metric 16, and not earlier.

### RFC2080-TIMER-5

**On deletion — by timeout, or by metric 16 from the current next hop — the route gets metric 16, the garbage-collection timer of 120 seconds, the change flag, and a triggered update.**

> "Deletions can occur for one of two reasons: the timeout expires, or
> the metric is set to 16 because of an update received from the
> current router" — §2.3, `rfc2080.txt:510-512`; "- The garbage-collection timer is set
> for 120 seconds." — `rfc2080.txt:516`; "- The metric for the route is set to 16
> (infinity)." — `rfc2080.txt:518`; "- The output process is signalled to trigger a
> response." — `rfc2080.txt:524`

- Strength: description. Class: wire.
- Check idea: when a route is deleted, the router sends an update with the route at metric
  16 soon after, without waiting for the next periodic update.

### RFC2080-TIMER-6

**Until the garbage-collection timer expires, the route is in every update; then it is removed.**

> "Until the garbage-collection timer expires, the route is included in
> all updates sent by this router.  When the garbage-collection timer
> expires, the route is deleted from the routing table." — §2.3, `rfc2080.txt:526-528`

- Strength: description. Class: wire.
- Check idea: after a deletion, every periodic update for 120 seconds holds the route at
  metric 16, and the updates after that do not hold it.

### RFC2080-TIMER-7

**A new route that arrives during garbage collection replaces the old one and clears the garbage-collection timer.**

> "Should a new route to this network be established while the garbage-
> collection timer is running, the new route will replace the one that
> is about to be deleted.  In this case the garbage-collection timer
> must be cleared." — §2.3, `rfc2080.txt:530-533`

- Strength: must. Class: wire.
- Check idea: a route is deleted and comes back before 120 seconds pass; the router still
  advertises it with a finite metric after the instant when the old garbage collection
  would have expired.

## Request messages

### RFC2080-REQ-1

**A router that has just come up sends requests from the RIPng port, by multicast.**

> "Normally, Requests are sent as multicasts,
> from the RIPng port, by routers which have just come up and are
> seeking to fill in their routing tables as quickly as possible." — §2.4.1,
> `rfc2080.txt:547-549`

- Strength: description. Class: wire.
- Check idea: a router that starts sends a request from port 521 to FF02::9.

### RFC2080-REQ-2

**A request for the table of one router should come from another port, and the router answers to the address and port of the requester, from a global address.**

> "In this case, the
> Request should be sent directly to that router from a UDP port other
> than the RIPng port.  If such a Request is received, the router
> responds directly to the requestor's address and port with a globally
> valid source address since the requestor may not reside on the
> directly attached network." — §2.4.1, `rfc2080.txt:551-556`

- Strength: should (the requester), description (the answer). Class: wire.
- Check idea: a request from port 5000 of a host; the answer goes to that host and port 5000,
  from a global source address.

### RFC2080-REQ-3

**A request with no entries gets no response.**

> "If there are no entries, no
> response is given." — §2.4.1, `rfc2080.txt:566-567`

- Strength: description. Class: wire.
- Check idea: a request with an empty list of entries; no response follows.

### RFC2080-REQ-4

**A request with one entry of prefix zero, prefix length zero and metric 16 asks for the whole table, which goes to the requesting address and port.**

> "If there is exactly one entry in the request, and it has a destination prefix of zero,
> a prefix length of zero, and a metric of infinity (i.e., 16), then this
> is a request to send the entire routing table.  In that case, a call
> is made to the output process to send the routing table to the
> requesting address/port." — §2.4.1, `rfc2080.txt:567-572`

- Strength: description. Class: wire.
- Check idea: the request of a router that starts has this form, and its neighbor answers
  with its table, sent to the address and port of the requester.

### RFC2080-REQ-5

**For a specific request, each entry gets the metric of the route, or 16 if there is none, and the message goes back as a response.**

> "Examine the list of RTEs in the Request one by one.
> For each entry, look up the destination in the router's routing
> database and, if there is a route, put that route's metric in the
> metric field of the RTE.  If there is no explicit route to the
> specified destination, put infinity in the metric field.  Once all
> the entries have been filled in, change the command from Request to
> Response and send the datagram back to the requestor." — §2.4.1, `rfc2080.txt:573-579`

- Strength: description. Class: wire.
- Check idea: a request for two prefixes, one known and one unknown; the response holds the
  known metric and 16, in the same order.

### RFC2080-REQ-6

**The answer to a whole-table request goes through normal output processing, split horizon included.**

> "If the request is for a complete routing
> table, normal output processing is done, including Split Horizon" — §2.4.1,
> `rfc2080.txt:582-584`; "For
> this reason, Split Horizon must be done." — `rfc2080.txt:591-592`

- Strength: must. Class: wire.
- Check idea: the answer to the request of a router that starts does not advertise, with a
  finite metric, a route learned from that router.

### RFC2080-REQ-7

**The answer to a specific request is the table as it is, without split horizon.**

> "If the request is for specific
> entries, they are looked up in the routing table and the information
> is returned as is; no Split Horizon processing is done." — §2.4.1,
> `rfc2080.txt:584-586`

- Strength: description. Class: wire.
- Check idea: a specific request from a neighbor for a route that the answering router
  learned from that neighbor; the answer holds the finite metric.

### RFC2080-REQ-8

**A router that comes up multicasts a request for the complete table on every connected network.**

> "When a router first comes up, it multicasts a Request on every connected network asking
> for a complete routing table." — §2.4.1, `rfc2080.txt:588-590`

- Strength: description. Class: wire.
- Check idea: a router with two networks sends a whole-table request on each of them when
  it starts.

## Response messages

### RFC2080-RESP-1

**A response is processed the same way whatever caused it.**

> "Processing is the same no matter why the Response was generated." — §2.4.2,
> `rfc2080.txt:606`

- Strength: description. Class: end-to-end.
- Check idea: none of its own; the checks of periodic, triggered and requested responses
  each show one cause.

### RFC2080-RESP-2

**A response that does not come from port 521 is ignored.**

> "The Response must be ignored if it is not from the RIPng port." — §2.4.2,
> `rfc2080.txt:609-610`

- Strength: must. Class: end-to-end.
- Check idea: a response from port 5000 that offers a new prefix; the receiver does not
  advertise the prefix.

### RFC2080-RESP-3

**The source of a response must be a link-local address.**

> "The
> datagram's IPv6 source address should be checked to see whether the
> datagram is from a valid neighbor; the source of the datagram must be
> a link-local address." — §2.4.2, `rfc2080.txt:610-613`

- Strength: should (check), must (the condition). Class: end-to-end.
- Check idea: a response from a global source address; the receiver does not take its
  routes.

### RFC2080-RESP-4

**A router ignores its own messages.**

> "If a router processes its own output as new input, confusion is likely, and such
> datagrams must be ignored." — §2.4.2, `rfc2080.txt:624-625`

- Strength: must. Class: end-to-end.
- Check idea: a response with the source address of the receiver and a changed metric;
  the receiver does not act on it.

### RFC2080-RESP-5

**A periodic advertisement carries hop limit 255.**

> "As an additional check, periodic advertisements must have their hop counts
> set to 255" — §2.4.2, `rfc2080.txt:625-627`

- Strength: must. Class: wire.
- Check idea: every periodic update and every triggered update carries hop limit 255 in its
  IPv6 header.

### RFC2080-RESP-6

**An inbound multicast from the RIPng port with a hop limit other than 255 is not from a neighbor; queries and their answers need no such test.**

> "and inbound, multicast packets sent from the RIPng port
> (i.e. periodic advertisement or triggered update packets) must be
> examined to ensure that the hop count is 255." — §2.4.2, `rfc2080.txt:627-629`;
> "Queries and their responses may still cross intermediate nodes and therefore do not
> require the hop count test to be done." — `rfc2080.txt:631-633`

- Strength: must. Class: end-to-end.
- Check idea: a multicast update from port 521 with hop limit 254 offers a new prefix; the
  receiver does not take it.

### RFC2080-RESP-7

**An entry with a multicast or link-local prefix, a prefix length above 128, or a metric outside 1 to 16 is ignored, and the next entry is processed.**

> "- is the destination prefix valid (e.g., not a multicast prefix and
> not a link-local address)  A link-local address should never be
> present in an RTE.
> - is the prefix length valid (i.e., between 0 and 128, inclusive)
> - is the metric valid (i.e., between 1 and 16, inclusive)
>
> If any check fails, ignore that entry and proceed to the next." — §2.4.2,
> `rfc2080.txt:643-649`

- Strength: description, should (no link-local prefix). Class: end-to-end.
- Check idea: a response with four entries: a multicast prefix, a link-local prefix, a
  metric of 17, and a valid prefix; only the valid prefix is learned.

### RFC2080-RESP-8

**A new destination with a metric below 16 is added, with a fresh timeout, the change flag and a triggered update.**

> "If there is no such route, add this route to the
> routing table, unless the metric is infinity" — §2.4.2, `rfc2080.txt:659-661`, and the
> six items of "Adding a route to the routing table consists of:", `rfc2080.txt:661-688`

- Strength: description. Class: wire.
- Check idea: a router learns a new prefix from a neighbor and advertises it on its other
  links with the metric increased by the cost.

### RFC2080-RESP-9

**A new destination that arrives with metric 16 is not added.**

> "add this route to the
> routing table, unless the metric is infinity (there is no point in
> adding a route which unusable)." — §2.4.2, `rfc2080.txt:659-661`

- Strength: description. Class: wire.
- Check idea: a prefix offered only with metric 16 never appears in the updates of the
  receiver.

### RFC2080-RESP-10

**An update for an existing route from its current next hop restarts the timeout.**

> "If this datagram
> is from the same router as the existing route, reinitialize the
> timeout." — §2.4.2, `rfc2080.txt:691-693`

- Strength: description. Class: wire.
- Check idea: a route whose next hop keeps sending updates never times out.

### RFC2080-RESP-11

**The route from the datagram is adopted when it comes from the current next hop with a different metric, or when its metric is lower.**

> "If the datagram is from the
> same router as the existing route, and the new metric is different
> than the old one; or, if the new metric is lower than the old one; do
> the following actions:" — §2.4.2, `rfc2080.txt:693-696`; then "Adopt the route from
> the datagram", set the change flag, signal a triggered update, and start the deletion at
> infinity or restart the timeout — `rfc2080.txt:698-705`

- Strength: description. Class: wire.
- Check idea: a route gets worse at its next hop, and the receiver takes the worse metric;
  a better path appears through another neighbor, and the receiver takes it.

### RFC2080-RESP-12

**The deletion starts only when the metric first becomes 16.**

> "Note that the
> deletion process is started only when the metric is first set to
> infinity.  If the metric was already infinity, then a new deletion
> process is not started." — §2.4.2, `rfc2080.txt:708-711`

- Strength: description. Class: wire.
- Check idea: a withdrawn route that keeps arriving with metric 16 still leaves the updates
  120 seconds after the first withdrawal.

### RFC2080-RESP-13

**With an equal metric, a router may switch to the new route when the current one is at least halfway to its timeout.**

> "Therefore, if the new metric is the same as the old one,
> examine the timeout for the existing route.  If it is at least
> halfway to the expiration point, switch to the new route.  This
> heuristic is optional, but highly recommended." — §2.4.2, `rfc2080.txt:722-725`

- Strength: should ("optional, but highly recommended"). Class: wire.
- Check idea: two equal paths; the current next hop goes silent; the router switches to the
  other path before the timeout expires.

### RFC2080-RESP-14

**Any other entry is ignored: an equal or worse route from a router that is not the current next hop changes nothing.**

> "Any entry that fails these tests is ignored, as it is no better than
> the current route." — §2.4.2, `rfc2080.txt:734-735`

- Strength: description. Class: wire.
- Check idea: a router with two paths of different length keeps the shorter one when the
  longer one is advertised.

## Output processing

### RFC2080-OUT-1

**The response to a request is unicast to the requester only.**

> "- By input processing, when a Request is received.  In this case, the
> Response is sent to only one destination (i.e. the unicast address
> of the requestor)." — §2.5, `rfc2080.txt:743-745`

- Strength: description. Class: wire.
- Check idea: the answer to the request of a router that starts goes to the unicast address
  of that router.

### RFC2080-OUT-2

**Every 30 seconds the whole table goes to every neighbor, and every change of a metric triggers an update.**

> "- By the regular routing update.  Every 30 seconds, a Response
> containing the whole routing table is sent to every neighboring
> router." and "- By triggered updates.  Whenever the metric for a route is changed,
> an update is triggered." — §2.5, `rfc2080.txt:747-752`

- Strength: description. Class: wire.
- Check idea: a new prefix, a changed metric and a deleted prefix each give a triggered
  update before the next periodic one.

### RFC2080-OUT-3

**Regular and triggered updates are multicast to FF02::9, one response for each directly-connected network.**

> "When a Response is to be sent to all neighbors (i.e., a regular or
> triggered update), a Response message is multicast to the multicast
> group FF02::9, the all-rip-routers multicast group, on all connected
> networks that support broadcasting or are point-to-point links." — §2.5,
> `rfc2080.txt:757-760`; "Thus, one
> Response is prepared for each directly-connected network, and sent to
> the all-rip-routers multicast group." — `rfc2080.txt:762-764`

- Strength: description. Class: wire.
- Check idea: a router with two networks sends each periodic update on both of them, to
  FF02::9.

## Triggered updates

### RFC2080-TRIG-1

**A router limits the rate of its triggered updates.**

> "Therefore, the protocol requires that implementors include provisions
> to limit the frequency of triggered updates." — §2.5.1, `rfc2080.txt:778-779`

- Strength: must ("requires"). Class: wire.
- Check idea: many route changes in a short time give fewer triggered updates than changes,
  and two triggered updates on one network are at least one second apart.

### RFC2080-TRIG-2

**After a triggered update, a random timer of 1 to 5 seconds collects further changes into one update.**

> "After a triggered
> update is sent, a timer should be set for a random interval between 1
> and 5 seconds.  If other changes that would trigger updates occur" "before the timer
> expires, a single update is triggered when the timer
> expires.  The timer is then reset to another random value between 1
> and 5 seconds." — §2.5.1, `rfc2080.txt:779-781`, `rfc2080.txt:790-792`

- Strength: should. Class: wire.
- Check idea: the intervals between consecutive triggered updates lie between 1 and 5
  seconds. That the value is random is a distribution.

### RFC2080-TRIG-3

**A triggered update may be suppressed when a regular update is due by the time it would be sent.**

> "Triggered updates may be suppressed if a regular
> update is due by the time the triggered update would be sent." — §2.5.1,
> `rfc2080.txt:792-793`

- Strength: may. Class: wire.
- Check idea: a permission; a change just before a periodic update may give no separate
  triggered update.

### RFC2080-TRIG-4

**A triggered update holds at least the routes whose change flag is set; a complete table is strongly discouraged.**

> "Therefore messages generated as part of a triggered update
> must include at least those routes that have their route change flag
> set.  They may include additional routes, at the discretion of the
> implementor; however, sending complete routing updates is strongly
> discouraged." — §2.5.1, `rfc2080.txt:797-801`

- Strength: must (the changed routes), may (more), "strongly discouraged" (the whole table).
  Class: wire.
- Check idea: when one route changes in a table of several, the triggered update holds that
  route and not the whole table.

### RFC2080-TRIG-5

**A triggered update goes out on every directly-connected network, after split horizon.**

> "When a triggered update is processed, messages should
> be generated for every directly-connected network.  Split Horizon
> processing is done when generating triggered updates as well as
> normal updates" — §2.5.1, `rfc2080.txt:801-804`

- Strength: should (every network), description (split horizon). Class: wire.
- Check idea: a deleted prefix appears in triggered updates on each network of the router,
  subject to split horizon.

### RFC2080-TRIG-6

**A changed route that split horizon leaves unchanged on a network need not be sent there, and an empty update may be left out.**

> "If, after Split Horizon processing
> for a given network, a changed route will appear unchanged on that
> network (e.g., it appears with an infinite metric), the route need
> not be sent.  If no routes need be sent on that network, the update
> may be omitted." — §2.5.1, `rfc2080.txt:804-808`

- Strength: may. Class: wire.
- Check idea: a permission; it widens the observation of RFC2080-TRIG-5.

### RFC2080-TRIG-7

**After the triggered updates are generated, the change flags are cleared.**

> "Once all of the triggered updates have been
> generated, the route change flags should be cleared." — §2.5.1, `rfc2080.txt:808-809`

- Strength: should. Class: wire.
- Check idea: two changes some seconds apart; the second triggered update does not repeat
  the route of the first one.

### RFC2080-TRIG-8

**The rules of the next section apply to triggered updates as to all other updates.**

> "The only difference between a triggered update and other update
> messages is the possible omission of routes that have not changed.
> The remaining mechanisms, described in the next section, must be
> applied to all updates." — §2.5.1, `rfc2080.txt:816-819`

- Strength: must. Class: wire.
- Check idea: a triggered update has the same header, ports, addresses, hop limit and entry
  rules as a periodic one.

## Generating a response

### RFC2080-GEN-1

**A response leaves from a link-local address of the interface; the answer to a unicast request from another port leaves from a global address.**

> "The IPv6 source address must be a link-local address of the possible
> addresses of the sending router's interface, except when replying to
> a unicast Request Message from a port other than the RIPng port.  In
> the latter case, the source address must be a globaly valid address." — §2.5.2,
> `rfc2080.txt:826-829`

- Strength: must. Class: wire.
- Check idea: every periodic and triggered update carries a link-local source address of
  the interface it leaves.

### RFC2080-GEN-2

**With several link-local addresses on an interface, a router sends from one designated address and keeps it while it is valid.**

> "In this case, the router must only originate a
> single Response message with a source address of the designated
> link-local address for a given interface.  The choice of which link-
> local address to use should only change when the current choice is no
> longer valid." — §2.5.2, `rfc2080.txt:846-850`

- Strength: must (one source), should (a stable choice). Class: wire.
- Check idea: an interface with two link-local addresses; all updates on it carry the same
  one of them as source.

### RFC2080-GEN-3

**A response carries version 1, command 2, and zero in the octets that must be zero.**

> "Set the version number to the current version of RIPng.  The version
> described in this document is version 1.  Set the command to
> Response.  Set the bytes labeled "must be zero" to zero." — §2.5.2,
> `rfc2080.txt:856-858`

- Strength: description. Class: wire.
- Check idea: every update carries version 1, command 2 and zero in the two unused octets
  of the header.

### RFC2080-GEN-4

**When a message is full for the MTU, the router sends it and starts another.**

> "Recall that the maximum datagram size is limited by the
> network's MTU.  When there is no more space in the datagram, send
> the current Response and start a new one." — §2.5.2, `rfc2080.txt:859-861`

- Strength: description. Class: wire.
- Check idea: a router with more routes than one message of the link MTU holds sends them
  in several messages, and together they hold every route.

### RFC2080-GEN-5

**A route to a link-local address is never advertised.**

> "Routes to link-local addresses must never be included in an RTE." — §2.5.2,
> `rfc2080.txt:863-864`

- Strength: must not ("must never"). Class: wire.
- Check idea: no entry of any update carries a link-local prefix.

### RFC2080-GEN-6

**A triggered update need hold only the changed routes; a route that split horizon excludes is skipped; an included route puts its prefix, prefix length, metric and route tag into an entry.**

> "If a triggered update is being generated, only entries whose route change
> flags are set need be included.  If, after Split Horizon processing,
> the route should not be included, skip it.  If the route is to be
> included, then the destination prefix, prefix length, and metric are
> put into the RTE.  The route tag is filled in as defined in section
> 2.1." — §2.5.2, `rfc2080.txt:864-870`

- Strength: description. Class: wire.
- Check idea: the entry of a route carries its prefix, its prefix length and its current
  metric.

### RFC2080-GEN-7

**A route with metric 16 is included in the updates too.**

> "Routes must be included in the datagram even if their metrics
> are infinite." — §2.5.2, `rfc2080.txt:870-871`

- Strength: must. Class: wire.
- Check idea: a deleted prefix, and a prefix that poisoned reverse sets to 16, both appear in
  the updates with metric 16.

## Out of scope in this catalog

The entries above hold every "must", "should" and "may" of §2 that states a behavior of a
router, and every field of the message. What is left out, and why:

- **§1**, `rfc2080.txt:71-164`: the introduction, the pointer to the theory of RFC 1058, and
  the limits of the protocol. They demand nothing.
- **The deployment statements**: RIPng "should be implemented only in routers",
  `rfc2080.txt:178-179`; every router of an AS must take part, and a router must leak routes
  between IGPs, `rfc2080.txt:240-243`; default routes should not leave the AS,
  `rfc2080.txt:454-458`; a list of neighbors for networks without broadcast is left to the
  implementor, `rfc2080.txt:765-771`; the uses of the route tag, `rfc2080.txt:349-350`. They
  say how a network is set up, not what a message holds.
- **The interlocking of input and output**, `rfc2080.txt:811-814`. It is a condition on
  concurrent input and output processing inside one router, and no message exchange can
  observe it.
- **§3, the control functions**, `rfc2080.txt:902-942`. The text calls them "not part of the
  protocol per se" and optional.
- **§4, the security considerations**, `rfc2080.txt:958-963`. They delegate to the IPv6
  security documents and demand nothing of RIPng.
