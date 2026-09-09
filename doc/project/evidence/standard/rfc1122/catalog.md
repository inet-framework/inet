# RFC 1122 (host requirements) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC1122-*` · **Stands on:** [ipv4/standards.md](../../protocol/ipv4/standards.md), [udp/standards.md](../../protocol/udp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 1122. Two protocols share it, and each one pins its own sections. The
sections of §3 come from the IPv4 standards map
([`ipv4/standards.md`](../../protocol/ipv4/standards.md#in-scope-set)); §4.1, the host
requirements for UDP, comes from the UDP standards map
([`udp/standards.md`](../../protocol/udp/standards.md#in-scope-set)). The catalog comes from
the RFC text only. It contains no simulation model names and no code references.

The entries of §3 carry the identifiers of the internet layer and the entries of §4.1 carry
identifiers that start with `U`. One document holds both, because one document owns the
`RFC1122-*` identifiers; a reader of either protocol reads the same text.

Source, cached in this folder:

- `rfc1122.txt` — Requirements for Internet Hosts — Communication Layers, October 1989.
  Downloaded 2026-09-09 from <https://www.rfc-editor.org/rfc/rfc1122.txt>.

RFC 1122 speaks for hosts. It restates the rules of RFC 791 with the keywords of its §1.3.2
(MUST, SHOULD, MAY and their negations), and it adds the rules that RFC 791 left implicit:
what a host does with a datagram it cannot process, and when a host must stay silent. Where
an entry restates an RFC 791 statement, the RFC 791 entry carries an `Overridden by` field
that points here; the test targets the entry that governs.

Quotes are verbatim. A reference such as `rfc1122.txt:1686` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledgers,
[`ipv4/coverage.md`](../../model/ipv4/coverage.md) and
[`udp/coverage.md`](../../model/udp/coverage.md). Keeping it out is deliberate: this
catalog states what the standard says, so a new test or a new run must never force an edit
here.

## Index

| ID | Statement |
| --- | --- |
| [RFC1122-VER-1](#rfc1122-ver-1) | A datagram whose version is not 4 is silently discarded. |
| [RFC1122-CKSUM-1](#rfc1122-cksum-1) | A host verifies the header checksum of every received datagram and silently discards a bad one. |
| [RFC1122-ADDR-1](#rfc1122-addr-1) | The source address of a sent datagram is one of the host's own addresses. |
| [RFC1122-ADDR-2](#rfc1122-addr-2) | A host silently discards a datagram that is not destined for it. |
| [RFC1122-ADDR-3](#rfc1122-addr-3) | A host silently discards a datagram with an invalid source address. |
| [RFC1122-ADDR-4](#rfc1122-addr-4) | Broadcast, zero, and loopback forms are not used as a source address. |
| [RFC1122-ID-1](#rfc1122-id-1) | A retransmitted copy may keep the identification (superseded). |
| [RFC1122-TTL-1](#rfc1122-ttl-1) | A host does not send a datagram with TTL zero. |
| [RFC1122-TTL-2](#rfc1122-ttl-2) | A host does not discard a received datagram because its TTL is below 2. |
| [RFC1122-TTL-3](#rfc1122-ttl-3) | The transport layer can set the TTL of every datagram. |
| [RFC1122-TTL-4](#rfc1122-ttl-4) | A fixed TTL is configurable. |
| [RFC1122-REASM-1](#rfc1122-reasm-1) | The IP layer implements reassembly. |
| [RFC1122-REASM-2](#rfc1122-reasm-2) | The reassembly size EMTU_R is at least 576 octets. |
| [RFC1122-REASM-3](#rfc1122-reasm-3) | The transport layer can learn MMS_R. |
| [RFC1122-REASM-4](#rfc1122-reasm-4) | There is a reassembly timeout, fixed, 60 to 120 seconds. |
| [RFC1122-REASM-5](#rfc1122-reasm-5) | On timeout the partial datagram is discarded and a Time Exceeded is sent if fragment zero arrived. |
| [RFC1122-FRAG-1](#rfc1122-frag-1) | The IP layer may fragment outgoing datagrams itself. |
| [RFC1122-FRAG-2](#rfc1122-frag-2) | The transport layer can learn MMS_S. |
| [RFC1122-FRAG-3](#rfc1122-frag-3) | Without local fragmentation, no datagram exceeds MMS_S. |
| [RFC1122-FRAG-4](#rfc1122-frag-4) | Off-net, the IP layer should send at most 576 octets. |
| [RFC1122-FRAG-5](#rfc1122-frag-5) | The MTU of each interface is configurable. |
| [RFC1122-ICMP-1](#rfc1122-icmp-1) | An ICMP message of unknown type is silently discarded. |
| [RFC1122-ICMP-2](#rfc1122-icmp-2) | An ICMP error quotes the header and at least 8 data octets, unchanged. |
| [RFC1122-ICMP-3](#rfc1122-icmp-3) | The quoted protocol number selects the transport that handles the error. |
| [RFC1122-ICMP-4](#rfc1122-icmp-4) | An ICMP error should carry TOS zero. |
| [RFC1122-ICMP-5](#rfc1122-icmp-5) | No ICMP error about an ICMP error. |
| [RFC1122-ICMP-6](#rfc1122-icmp-6) | No ICMP error about a datagram to an IP broadcast or multicast address. |
| [RFC1122-ICMP-7](#rfc1122-icmp-7) | No ICMP error about a datagram sent as a link-layer broadcast. |
| [RFC1122-ICMP-8](#rfc1122-icmp-8) | No ICMP error about a non-initial fragment. |
| [RFC1122-ICMP-9](#rfc1122-icmp-9) | No ICMP error about a datagram whose source is not a single host. |
| [RFC1122-DU-1](#rfc1122-du-1) | A host should send Destination Unreachable code 2 or 3. |
| [RFC1122-DU-2](#rfc1122-du-2) | A received Destination Unreachable is reported to the transport layer. |
| [RFC1122-DU-3](#rfc1122-du-3) | Codes 0, 1 and 5 are only a hint. |
| [RFC1122-TE-1](#rfc1122-te-1) | A received Time Exceeded is passed to the transport layer. |
| [RFC1122-PP-1](#rfc1122-pp-1) | A host should generate Parameter Problem messages. |
| [RFC1122-PP-2](#rfc1122-pp-2) | A received Parameter Problem is passed to the transport layer. |
| [RFC1122-ERR-1](#rfc1122-err-1) | Wherever practical, a host returns an ICMP error on an error. |
| [RFC1122-UCK-1](#rfc1122-uck-1) | A host implements the generation and the check of the UDP checksum. |
| [RFC1122-UCK-2](#rfc1122-uck-2) | An application may be able to control whether a checksum is generated. |
| [RFC1122-UCK-3](#rfc1122-uck-3) | The generation of the checksum is on by default. |
| [RFC1122-UCK-4](#rfc1122-uck-4) | A datagram whose checksum is non-zero and wrong is silently discarded. |
| [RFC1122-UCK-5](#rfc1122-uck-5) | An application may be able to control what happens to a datagram without a checksum. |
| [RFC1122-UCK-6](#rfc1122-uck-6) | A computed checksum of zero is transmitted as all ones. |
| [RFC1122-UPORT-1](#rfc1122-uport-1) | A datagram for a port with no listener should draw an ICMP Port Unreachable message. |
| [RFC1122-UERR-1](#rfc1122-uerr-1) | UDP passes every ICMP error message it receives up to the application. |
| [RFC1122-UOPT-1](#rfc1122-uopt-1) | UDP passes a received IP option to the application unchanged. |
| [RFC1122-UOPT-2](#rfc1122-uopt-2) | An application can specify the IP options of a sent datagram. |
| [RFC1122-UMH-1](#rfc1122-umh-1) | The specific destination address of a received datagram goes up to the application. |
| [RFC1122-UMH-2](#rfc1122-umh-2) | An application can choose the source address of a datagram. |
| [RFC1122-UMH-3](#rfc1122-umh-3) | There should be a way to tell the application which source address was chosen. |
| [RFC1122-UADDR-1](#rfc1122-uaddr-1) | A datagram whose IP source address is invalid is discarded, by UDP or by the IP layer. |
| [RFC1122-UADDR-2](#rfc1122-uaddr-2) | The source address of a datagram a host sends is an address of that host. |
| [RFC1122-UAPI-1](#rfc1122-uapi-1) | An application can set the TTL, the TOS and the IP options of a datagram it sends. |
| [RFC1122-UAPI-2](#rfc1122-uapi-2) | UDP may pass the received TOS up to the application. |
| [RFC1122-UAPI-3](#rfc1122-uapi-3) | The application interface of UDP gives the full service of the IP transport interface. |

The rows above the line hold §3, the internet layer. The rows below hold §4.1, UDP.

## How to read an entry

- **Strength** — the keyword the RFC uses: `must`, `must not`, `should`, `should not`,
  `may`, or `description` for normative prose without a keyword. RFC 1122 §1.3.2 defines
  the keywords; a MUST NOT is a `must not` here, not a `must`.
- **Class** — how a test can observe the statement:
  - `wire` — fields of datagrams on a link between two nodes;
  - `end-to-end` — what the destination accepts and delivers upward, or does not;
  - `error-signal` — an ICMP message that reports a failure, or its absence;
  - `internal` — state or an interface inside a module; not visible from outside;
  - `encoding` — the exact bit layout of a field; a serializer concern.
- **Governs** — the entry of a base document that this entry restates with a keyword, or
  the entry of another layer that states the same rule. That entry carries the matching
  `Overridden by` field when the strength changes.
- **Overridden by** — present only when a later in-scope document changes this statement.
  One entry carries it: RFC 6864 replaces the identification permission of §3.2.1.5.

## Version

### RFC1122-VER-1

**A datagram whose version is not 4 is silently discarded.**

> "A datagram whose version number is not 4 MUST be silently discarded." — §3.2.1.1,
> `rfc1122.txt:1686-1687`

- Strength: must. Class: end-to-end (absence) plus error-signal (absence).
- Check idea: deliver to a host a datagram whose header carries version 5 and an otherwise
  valid header. The host delivers nothing upward and sends nothing back.

## Header checksum

### RFC1122-CKSUM-1

**A host verifies the header checksum of every received datagram and silently discards a
datagram whose checksum is bad.**

> "A host MUST verify the IP header checksum on every received datagram and silently
> discard every datagram that has a bad checksum." — §3.2.1.2, `rfc1122.txt:1691-1693`

- Strength: must. Class: end-to-end (absence) plus error-signal (absence).
- Governs: [RFC791-CKSUM-2](../rfc791/catalog.md#rfc791-cksum-2).
- Check idea: corrupt the checksum field of a datagram in flight and leave the rest of the
  header intact. The host delivers nothing upward and sends nothing back.

## Addressing

### RFC1122-ADDR-1

**The source address of every datagram a host sends is one of the host's own addresses,
and never a broadcast or multicast address.**

> "When a host sends any datagram, the IP source address MUST be one of its own IP
> addresses (but not a broadcast or multicast address)." — §3.2.1.3,
> `rfc1122.txt:1805-1807`

- Strength: must. Class: wire.
- Check idea: every datagram that leaves a host carries an address of that host as source.

### RFC1122-ADDR-2

**A host silently discards an incoming datagram that is not destined for it.**

> "A host MUST silently discard an incoming datagram that is not destined for the host.
> An incoming datagram is destined for the host if the datagram's destination address
> field is: (1) (one of) the host's IP address(es); or (2) an IP broadcast address valid
> for the connected network; or (3) the address for a multicast group of which the host is
> a member on the incoming physical interface." — §3.2.1.3, `rfc1122.txt:1809-1819`

- Strength: must. Class: end-to-end (absence) plus error-signal (absence).
- Check idea: deliver to a host, on its own link, a datagram whose destination address is
  another unicast address. The host delivers nothing upward and sends nothing back.

### RFC1122-ADDR-3

**A host silently discards an incoming datagram whose source address is invalid.**

> "A host MUST silently discard an incoming datagram containing an IP source address that
> is invalid by the rules of this section. This validation could be done in either the IP
> layer or by each protocol in the transport layer." — §3.2.1.3, `rfc1122.txt:1843-1846`

- Strength: must. Class: end-to-end (absence) plus error-signal (absence).
- Check idea: deliver to a host a datagram whose source address is the limited broadcast
  address. Nothing reaches the program, whichever layer discards it.
- Note: the invalid forms are the ones of [RFC1122-ADDR-4](#rfc1122-addr-4).

### RFC1122-ADDR-4

**The broadcast forms, the zero forms, and the loopback form are not used as a source
address, and a loopback address never appears outside a host.**

> "(c) { -1, -1 } Limited broadcast. It MUST NOT be used as a source address." —
> §3.2.1.3, `rfc1122.txt:1748-1751`; the same sentence for the directed broadcast forms (d),
> (e) and (f), `rfc1122.txt:1757-1781`; "(g) { 127, <any> } Internal host loopback address.
> Addresses of this form MUST NOT appear outside a host." — `rfc1122.txt:1783-1786`; the
> forms (a) { 0, 0 } and (b) { 0, <Host-number> } "MUST NOT be sent, except as a source
> address as part of an initialization procedure" — `rfc1122.txt:1734-1746`

- Strength: must not. Class: wire.
- Check idea: over a whole run, no datagram on any link carries one of these forms as its
  source address. This is a standing negative rule, not a step in a sequence.
- Note: the same section defines which of these forms a host must recognize as a
  destination (the broadcast family). That side is out of scope; see the standards map.

## Identification

### RFC1122-ID-1

**A retransmitted copy of a datagram may keep the identification of the original.**

> "When sending an identical copy of an earlier datagram, a host MAY optionally retain the
> same Identification field in the copy." — §3.2.1.5, `rfc1122.txt:1878-1880`

- Strength: may. Class: wire.
- Overridden by: [RFC6864-ID-4](../rfc6864/catalog.md#rfc6864-id-4). RFC 6864 §6.2 states
  the change: a retransmitted non-atomic datagram is no longer permitted to reuse the value.
  The entry stays for the record; no test targets it.

## Time to live

### RFC1122-TTL-1

**A host does not send a datagram with a TTL of zero.**

> "A host MUST NOT send a datagram with a Time-to-Live (TTL) value of zero." — §3.2.1.7,
> `rfc1122.txt:1977-1978`

- Strength: must not. Class: wire.
- Check idea: ask the host to send with a TTL of zero. No datagram with TTL zero appears on
  the host's link; the host refuses the request or substitutes a positive value.

### RFC1122-TTL-2

**A host does not discard a received datagram only because its TTL is below 2.**

> "A host MUST NOT discard a datagram just because it was received with TTL less than 2."
> — §3.2.1.7, `rfc1122.txt:1980-1981`

- Strength: must not. Class: end-to-end.
- Check idea: send a datagram whose TTL is exactly the hop count of the path, so that it
  arrives at the destination with TTL 1. The destination delivers it upward.

### RFC1122-TTL-3

**The IP layer lets the transport layer set the TTL of every datagram it sends.**

> "The IP layer MUST provide a means for the transport layer to set the TTL field of every
> datagram that is sent." — §3.2.1.7, `rfc1122.txt:1983-1984`

- Strength: must. Class: wire (the requested value appears on the first link) plus
  internal (the interface).
- Check idea: a program that asks for a TTL value sees that value on the first link.

### RFC1122-TTL-4

**When a host uses a fixed TTL, the value is configurable.**

> "When a fixed TTL value is used, it MUST be configurable." — §3.2.1.7,
> `rfc1122.txt:1984-1985`

- Strength: must. Class: internal (a configuration property) with a wire effect.
- Check idea: change the configured default and observe the new value on the first link
  for a datagram whose sender set no TTL.

## Reassembly

### RFC1122-REASM-1

**The IP layer implements reassembly.**

> "The IP layer MUST implement reassembly of IP datagrams." — §3.3.2, `rfc1122.txt:3281`
>
> "The Internet model requires that every host support reassembly." — §3.2.1.4,
> `rfc1122.txt:1872-1873`

- Strength: must. Class: end-to-end.
- Governs: [RFC791-REASM-1](../rfc791/catalog.md#rfc791-reasm-1), which describes the
  procedure without an obligation.

### RFC1122-REASM-2

**The largest datagram a host can reassemble, EMTU_R, is at least 576 octets.**

> "We designate the largest datagram size that can be reassembled by EMTU_R ("Effective
> MTU to receive"); this is sometimes called the "reassembly buffer size". EMTU_R MUST be
> greater than or equal to 576, SHOULD be either configurable or indefinite, and SHOULD be
> greater than or equal to the MTU of the connected network(s)." — §3.3.2,
> `rfc1122.txt:3283-3288`

- Strength: must (576); should (configurable or indefinite; at least the MTU). Class:
  end-to-end.
- Governs: [RFC791-REASM-2](../rfc791/catalog.md#rfc791-reasm-2).
- Check idea: a 576-octet datagram that arrives in fragments is delivered whole. A datagram
  as large as the MTU of the connected link, fragmented on the path, is a check of the
  should half.

### RFC1122-REASM-3

**The transport layer can learn MMS_R, the largest message it can receive.**

> "There MUST be a mechanism by which the transport layer can learn MMS_R, the maximum
> message size that can be received and reassembled in an IP datagram" — §3.3.2,
> `rfc1122.txt:3327-3330`

- Strength: must. Class: internal (an interface between layers).
- Note: not observable on the wire. A module test can ask the interface.

### RFC1122-REASM-4

**There is a reassembly timeout; it should be a fixed value between 60 and 120 seconds.**

> "There MUST be a reassembly timeout. The reassembly timeout value SHOULD be a fixed
> value, not set from the remaining TTL. It is recommended that the value lie between 60
> seconds and 120 seconds." — §3.3.2, `rfc1122.txt:3337-3340`

- Strength: must (a timeout exists); should (fixed); description (the range). Class:
  internal with an error-signal effect ([RFC1122-REASM-5](#rfc1122-reasm-5)).
- Note: a timer is level 4 work by the level table of the guide.

### RFC1122-REASM-5

**When the timeout expires, the partial datagram is discarded, and a Time Exceeded message
goes to the source if fragment zero arrived.**

> "If this timeout expires, the partially-reassembled datagram MUST be discarded and an
> ICMP Time Exceeded message sent to the source host (if fragment zero has been
> received)." — §3.3.2, `rfc1122.txt:3340-3342`

- Strength: must. Class: end-to-end (absence) plus error-signal.
- Check idea: withhold one fragment. After the timeout, nothing is delivered, and the
  source receives Time Exceeded code 1 when fragment zero was among the received ones,
  and nothing when it was not.
- Note: level 4, with [RFC1122-REASM-4](#rfc1122-reasm-4).

## Fragmentation

### RFC1122-FRAG-1

**The IP layer may fragment outgoing datagrams itself.**

> "Optionally, the IP layer MAY implement a mechanism to fragment outgoing datagrams
> intentionally." — §3.3.3, `rfc1122.txt:3385-3386`

- Strength: may. Class: wire.
- Check idea: a datagram larger than the MTU of the first link leaves the host in
  fragments. Absence of the mechanism is not a violation; then
  [RFC1122-FRAG-3](#rfc1122-frag-3) applies.

### RFC1122-FRAG-2

**The transport layer can learn MMS_S, the largest message it may send.**

> "A host MUST implement a mechanism to allow the transport layer to learn MMS_S, the
> maximum transport-layer message size that may be sent for a given {source, destination,
> TOS} triplet" — §3.3.3, `rfc1122.txt:3393-3396`

- Strength: must. Class: internal (an interface between layers).

### RFC1122-FRAG-3

**A host without local fragmentation never sends a datagram larger than MMS_S.**

> "A host that does not implement local fragmentation MUST ensure that the transport layer
> (for TCP) or the application layer (for UDP) obtains MMS_S from the IP layer and does
> not send a datagram exceeding MMS_S in size." — §3.3.3, `rfc1122.txt:3407-3410`

- Strength: must, conditional on the absence of [RFC1122-FRAG-1](#rfc1122-frag-1). Class:
  wire.
- Check idea: no datagram on the first link is larger than the MTU of that link.

### RFC1122-FRAG-4

**To a destination that is not on a connected network, the IP layer should send datagrams
of at most 576 octets.**

> "In the absence of actual knowledge of the minimum MTU along the path, the IP layer
> SHOULD use EMTU_S <= 576 whenever the destination address is not on a connected network,
> and otherwise use the connected network's MTU." — §3.3.3, `rfc1122.txt:3414-3429`

- Strength: should. Class: wire.
- Check idea: a program hands the IP layer more than 556 octets for an off-net
  destination; the datagrams on the first link are at most 576 octets long. A host that
  sends the larger datagram whole declines a should, which is not a violation.

### RFC1122-FRAG-5

**The MTU of each interface is configurable.**

> "The MTU of each physical interface MUST be configurable." — §3.3.3,
> `rfc1122.txt:3431`

- Strength: must. Class: internal (a configuration property) with a wire effect.
- Check idea: set the MTU of an interface below the size of a datagram and observe the
  effect on that link.

## ICMP, general rules

### RFC1122-ICMP-1

**An ICMP message of unknown type is silently discarded.**

> "If an ICMP message of unknown type is received, it MUST be silently discarded." —
> §3.2.2, `rfc1122.txt:2222-2223`

- Strength: must. Class: end-to-end (absence) plus error-signal (absence).
- Check idea: deliver to a host an ICMP message whose type number is assigned to no
  message. The host sends nothing back and delivers nothing upward.

### RFC1122-ICMP-2

**An ICMP error message quotes the internet header and at least the first 8 data octets of
the datagram that caused it, unchanged.**

> "Every ICMP error message includes the Internet header and at least the first 8 data
> octets of the datagram that triggered the error; more than 8 octets MAY be sent; this
> header and data MUST be unchanged from the received datagram." — §3.2.2,
> `rfc1122.txt:2225-2228`

- Strength: description (the quote exists); may (more than 8 octets); must (unchanged).
  Class: error-signal (the content of the message).
- Check idea: cause an error report and compare the quoted header and the first 8 data
  octets with the datagram that was sent.

### RFC1122-ICMP-3

**The protocol number of the quoted header selects the transport entity that handles a
received ICMP error.**

> "In those cases where the Internet layer is required to pass an ICMP error message to
> the transport layer, the IP protocol number MUST be extracted from the original header
> and used to select the appropriate transport protocol entity to handle the error." —
> §3.2.2, `rfc1122.txt:2230-2234`

- Strength: must. Class: internal (the handoff between layers).

### RFC1122-ICMP-4

**An ICMP error message should carry zero TOS bits.**

> "An ICMP error message SHOULD be sent with normal (i.e., zero) TOS bits." — §3.2.2,
> `rfc1122.txt:2236-2237`

- Strength: should. Class: wire.
- Check idea: the type of service byte of every error report is zero.

The five entries below share one lead sentence. Each is a case of its own, because each
needs a different scenario.

> "An ICMP error message MUST NOT be sent as the result of receiving:" — §3.2.2,
> `rfc1122.txt:2249-2250`; and the note that closes the list: "NOTE: THESE RESTRICTIONS
> TAKE PRECEDENCE OVER ANY REQUIREMENT ELSEWHERE IN THIS DOCUMENT FOR SENDING ICMP ERROR
> MESSAGES." — `rfc1122.txt:2266-2267`

### RFC1122-ICMP-5

**No ICMP error message is sent about an ICMP error message.**

> "* an ICMP error message, or" — §3.2.2, `rfc1122.txt:2252`

- Strength: must not. Class: error-signal (absence).
- Check idea: let an ICMP error message run into an error of its own on the way, for
  example a TTL that expires. The node that discards it sends no report.

### RFC1122-ICMP-6

**No ICMP error message is sent about a datagram destined to an IP broadcast or IP
multicast address.**

> "* a datagram destined to an IP broadcast or IP multicast address, or" — §3.2.2,
> `rfc1122.txt:2254-2255`

- Strength: must not. Class: error-signal (absence).
- Check idea: send a datagram to the limited broadcast address and to a port that no
  program opened. No host on the link answers with a Destination Unreachable.

### RFC1122-ICMP-7

**No ICMP error message is sent about a datagram that arrived as a link-layer broadcast.**

> "* a datagram sent as a link-layer broadcast, or" — §3.2.2, `rfc1122.txt:2257`

- Strength: must not. Class: error-signal (absence).
- Check idea: deliver a unicast datagram to a closed port inside a broadcast frame of the
  link layer. The host sends no Destination Unreachable.

### RFC1122-ICMP-8

**No ICMP error message is sent about a non-initial fragment.**

> "* a non-initial fragment, or" — §3.2.2, `rfc1122.txt:2259`

- Strength: must not. Class: error-signal (absence).
- Check idea: let the fragments of one datagram all fail at one node, for example by a TTL
  that expires there. At most the first fragment produces a report.

### RFC1122-ICMP-9

**No ICMP error message is sent about a datagram whose source address does not name a
single host.**

> "* a datagram whose source address does not define a single host -- e.g., a zero
> address, a loopback address, a broadcast address, a multicast address, or a Class E
> address." — §3.2.2, `rfc1122.txt:2261-2264`

- Strength: must not. Class: error-signal (absence).
- Check idea: deliver a datagram with the limited broadcast address as source to a closed
  port. The host sends no Destination Unreachable.

## Destination Unreachable

### RFC1122-DU-1

**A host should send Destination Unreachable with code 2 when the protocol is not
supported, and with code 3 when the port is closed.**

> "A host SHOULD generate Destination Unreachable messages with code: 2 (Protocol
> Unreachable), when the designated transport protocol is not supported; or 3 (Port
> Unreachable), when the designated transport protocol (e.g., UDP) is unable to
> demultiplex the datagram but has no protocol mechanism to inform the sender." —
> §3.2.2.1, `rfc1122.txt:2322-2331`

- Strength: should. Class: error-signal.
- Check idea: send a UDP datagram to a port that no program opened. The host returns type
  3, code 3.

### RFC1122-DU-2

**A received Destination Unreachable is reported to the transport layer.**

> "A Destination Unreachable message that is received MUST be reported to the transport
> layer." — §3.2.2.1, `rfc1122.txt:2333-2334`

- Strength: must. Class: internal (the handoff between layers).

### RFC1122-DU-3

**Destination Unreachable codes 0, 1 and 5 are only a hint.**

> "A Destination Unreachable message that is received with code 0 (Net), 1 (Host), or 5
> (Bad Source Route) may result from a routing transient and MUST therefore be
> interpreted as only a hint, not proof, that the specified destination is unreachable" —
> §3.2.2.1, `rfc1122.txt:2342-2346`

- Strength: must. Class: internal (a rule for the consumer of the report).

## Time Exceeded

### RFC1122-TE-1

**A received Time Exceeded is passed to the transport layer.**

> "An incoming Time Exceeded message MUST be passed to the transport layer." — §3.2.2.4,
> `rfc1122.txt:2413-2414`

- Strength: must. Class: internal (the handoff between layers).

## Parameter Problem

### RFC1122-PP-1

**A host should generate Parameter Problem messages.**

> "A host SHOULD generate Parameter Problem messages." — §3.2.2.5, `rfc1122.txt:2442`

- Strength: should. Class: error-signal.
- Check idea: deliver a datagram with a malformed header field that no other message
  covers. The host returns type 12.

### RFC1122-PP-2

**A received Parameter Problem is passed to the transport layer and may be reported to the
user.**

> "An incoming Parameter Problem message MUST be passed to the transport layer, and it MAY
> be reported to the user." — §3.2.2.5, `rfc1122.txt:2442-2444`

- Strength: must (transport); may (user). Class: internal.

## Error reporting

### RFC1122-ERR-1

**Wherever practical, a host returns an ICMP error message when it detects an error, unless
a rule prohibits the message.**

> "Wherever practical, hosts MUST return ICMP error datagrams on detection of an error,
> except in those cases where returning an ICMP error message is specifically
> prohibited." — §3.3.8, `rfc1122.txt:4021-4023`

- Strength: must, with the hedge "wherever practical". Class: error-signal.
- Note: this is the host-side counterpart of the "may" of every RFC 792 error message. It
  governs a host; a gateway stays under RFC 792 until RFC 1812 enters the in-scope set.
  The prohibitions are [RFC1122-ICMP-5](#rfc1122-icmp-5) to
  [RFC1122-ICMP-9](#rfc1122-icmp-9).

## UDP checksums

### RFC1122-UCK-1

**A host implements the generation and the check of the UDP checksum.**

> "A host MUST implement the facility to generate and validate UDP checksums." — §4.1.3.4,
> `rfc1122.txt:4584-4585`

- Strength: must. Class: wire plus end-to-end.
- Governs: [RFC768-CKSUM-1](../rfc768/catalog.md#rfc768-cksum-1). RFC 768 defines the
  checksum but demands nothing; this entry makes both halves mandatory for a host.
- Check idea: a datagram that a host sends carries the checksum that RFC 768 defines, and a
  datagram whose checksum is wrong does not reach the program at the receiver.

### RFC1122-UCK-2

**An application may be able to control whether a checksum is generated.**

> "An application MAY optionally be able to control whether a UDP checksum will be
> generated" — §4.1.3.4, `rfc1122.txt:4585-4586`

- Strength: may. Class: internal.
- Check idea: none from the wire. The statement permits an interface; it demands no
  behaviour that two nodes can show each other.

### RFC1122-UCK-3

**The generation of the checksum is on by default.**

> "but it MUST default to checksumming on." — §4.1.3.4, `rfc1122.txt:4586-4587`

- Strength: must. Class: wire.
- Check idea: a host sends a datagram, and no program and no configuration says anything
  about the checksum. The datagram on the wire carries the checksum that RFC 768 defines:
  the one's complement sum over the pseudo header, the UDP header and the data.

### RFC1122-UCK-4

**A datagram whose checksum is non-zero and wrong is silently discarded.**

> "If a UDP datagram is received with a checksum that is non-zero and invalid, UDP MUST
> silently discard the datagram." — §4.1.3.4, `rfc1122.txt:4589-4590`

- Strength: must. Class: end-to-end (absence) plus error-signal (absence).
- Check idea: change a datagram in flight, so that its checksum no longer matches what it
  covers, and leave the checksum non-zero. The receiver hands nothing to the program and
  sends nothing back. The change can be to the checksum itself, to the data, or to a field
  of the pseudo header; the three are the same statement seen from three sides.

### RFC1122-UCK-5

**An application may be able to control what happens to a datagram without a checksum.**

> "An application MAY optionally be able to control whether UDP datagrams without checksums
> should be discarded or passed to the application." — §4.1.3.4, `rfc1122.txt:4591-4593`

- Strength: may. Class: internal, with an end-to-end consequence.
- Check idea: the permission itself is an interface. What a test can see is the behaviour
  when no application uses the permission: a datagram whose checksum is all zero reaches
  the program, because RFC 768 says an all-zero checksum means that the sender generated
  none.

### RFC1122-UCK-6

**A computed checksum of zero is transmitted as all ones.**

> "If the transmitter really calculates a UDP checksum of zero, it must transmit the
> checksum as all 1's (65535)." — §4.1.3.4, IMPLEMENTATION, `rfc1122.txt:4618-4620`

- Strength: description. The sentence is in an IMPLEMENTATION note and its "must" is not
  one of the keywords of §1.3.2.
- Governs: [RFC768-CKSUM-2](../rfc768/catalog.md#rfc768-cksum-2).
- Check idea: a sender whose data gives a sum of zero puts 65535 on the wire. A test cannot
  choose the data that gives this sum without knowledge of the addresses and the ports that
  the run assigns.

## UDP ports

### RFC1122-UPORT-1

**A datagram for a port with no listener should draw an ICMP Port Unreachable message.**

> "If a datagram arrives addressed to a UDP port for which there is no pending LISTEN call,
> UDP SHOULD send an ICMP Port Unreachable message." — §4.1.3.1, `rfc1122.txt:4527-4529`

- Strength: should. Class: error-signal.
- Governs: [RFC792-DU-3](../rfc792/catalog.md#rfc792-du-3). RFC 792 says a host **may**
  send the message; for a host that follows RFC 1122 the strength is **should**.
- Check idea: send a datagram to a port on which no program listens. The receiver answers
  with Destination Unreachable, code 3.

## UDP and ICMP errors

### RFC1122-UERR-1

**UDP passes every ICMP error message it receives up to the application.**

> "UDP MUST pass to the application layer all ICMP error messages that it receives from the
> IP layer." — §4.1.3.3, `rfc1122.txt:4565-4566`

- Strength: must. Class: internal.
- Check idea: the observation is at the interface between UDP and the program, not on a
  link. A test that watches the traffic between two nodes cannot see it.

## UDP and IP options

### RFC1122-UOPT-1

**UDP passes a received IP option to the application unchanged.**

> "UDP MUST pass any IP option that it receives from the IP layer transparently to the
> application layer." — §4.1.3.2, `rfc1122.txt:4533-4534`

- Strength: must. Class: internal.
- Check idea: the observation is at the interface between UDP and the program.

### RFC1122-UOPT-2

**An application can specify the IP options of a sent datagram, and UDP passes them down.**

> "An application MUST be able to specify IP options to be sent in its UDP datagrams, and
> UDP MUST pass these options to the IP layer." — §4.1.3.2, `rfc1122.txt:4536-4538`

- Strength: must. Class: internal, with a wire consequence.
- Check idea: a program asks for an IP option, and the datagram on the wire carries it.

## UDP multihoming

### RFC1122-UMH-1

**The specific destination address of a received datagram goes up to the application.**

> "When a UDP datagram is received, its specific-destination address MUST be passed up to
> the application layer." — §4.1.3.5, `rfc1122.txt:4626-4627`

- Strength: must. Class: internal.
- Check idea: the observation is at the interface between UDP and the program.

### RFC1122-UMH-2

**An application can choose the source address of a datagram, or leave the choice open.**

> "An application program MUST be able to specify the IP source address to be used for
> sending a UDP datagram or to leave it unspecified (in which case the networking software
> will choose an appropriate source address)." — §4.1.3.5, `rfc1122.txt:4629-4632`

- Strength: must. Class: internal, with a wire consequence.
- Check idea: a program on a host with two interfaces names one of the two addresses, and
  the datagram on the wire carries the address that the program named.

### RFC1122-UMH-3

**There should be a way to tell the application which source address was chosen.**

> "There SHOULD be a way to communicate the chosen source address up to the application
> layer" — §4.1.3.5, `rfc1122.txt:4632-4634`

- Strength: should. Class: internal.
- Check idea: the observation is at the interface between UDP and the program.

## UDP addresses

### RFC1122-UADDR-1

**A datagram whose IP source address is invalid is discarded, by UDP or by the IP layer.**

> "A UDP datagram received with an invalid IP source address (e.g., a broadcast or multicast
> address) must be discarded by UDP or by the IP layer" — §4.1.3.6, `rfc1122.txt:4646-4648`

The requirements summary of §4.1.5 lists the statement in the MUST column:

> "Bad IP src addr silently discarded by UDP/IP |4.1.3.6 |x| | | | |" — §4.1.5,
> `rfc1122.txt:4731`

- Strength: must. Class: end-to-end (absence).
- Governs: [RFC1122-ADDR-3](#rfc1122-addr-3), for a datagram that carries UDP. The two
  entries state one rule for two layers: §4.1.3.6 lets either layer do the discard, and it
  points at §3.2.1.3 for the address forms that are invalid.
- Check idea: put a broadcast or multicast address in the source field of a datagram in
  flight and keep every checksum valid. The receiver hands nothing to the program.

### RFC1122-UADDR-2

**The source address of a datagram a host sends is an address of that host.**

> "When a host sends a UDP datagram, the source address MUST be (one of) the IP address(es)
> of the host." — §4.1.3.6, `rfc1122.txt:4650-4651`

- Strength: must. Class: wire.
- Governs: [RFC1122-ADDR-1](#rfc1122-addr-1), for a datagram that carries UDP.
- Check idea: read the source address of every datagram that a host sends and compare it
  with the addresses of the interfaces of that host.

## UDP and the application interface

### RFC1122-UAPI-1

**An application can set the TTL, the TOS and the IP options of a datagram it sends, and
these values reach the IP layer unchanged.**

> "An application-layer program MUST be able to set the TTL and TOS values as well as IP
> options for sending a UDP datagram, and these values must be passed transparently to the
> IP layer." — §4.1.4, `rfc1122.txt:4675-4677`

- Strength: must. Class: internal, with a wire consequence.
- Check idea: a program names a TTL, and the datagram that leaves the host carries that
  TTL.

### RFC1122-UAPI-2

**UDP may pass the received TOS up to the application.**

> "UDP MAY pass the received TOS up to the application layer." — §4.1.4, `rfc1122.txt:4678`

- Strength: may. Class: internal.
- Check idea: the observation is at the interface between UDP and the program.

### RFC1122-UAPI-3

**The application interface of UDP gives the full service of the IP transport interface.**

> "The application interface to UDP MUST provide the full services of the IP/transport
> interface described in Section 3.4" — §4.1.4, `rfc1122.txt:4655-4656`

- Strength: must. Class: internal.
- Check idea: the statement names the calls of §3.4, which are an interface and not a
  behaviour on a link. The standards map leaves §3.4 out of the in-scope set for the same
  reason.

## Out of scope in this catalog

Three statements of the in-scope sections of §3 are left out on purpose: the
All-Subnets-MTU configuration flag of §3.3.3 (a `may` about a host with several subnets),
and the two sentences of §3.2.2.1 and §3.2.2.3 that tell the **transport** layer how to act
on a received report.

§4.1 is here in full. Its two other parts hold no checkable statement: §4.1.1 introduces
UDP, and §4.1.2 says that the specification of UDP has no known error.

The parts of §3 that the standards map leaves out of the IPv4 set: type of service
(§3.2.1.6), the options (§3.2.1.8) and source route forwarding (§3.3.5), the subnet
requirement and the broadcast address forms as destinations (§3.2.1.3, §3.3.6), redirect
and host routing (§3.2.2.2, §3.3.1, §3.3.4), source quench (§3.2.2.3), the ICMP query
messages (§3.2.2.6 to §3.2.2.9), IGMP and multicasting (§3.2.3, §3.3.7), and the layer
interface of §3.4. The reasons are in
[`ipv4/standards.md`](../../protocol/ipv4/standards.md#in-scope-set). §4.2 of the document,
the host requirements for TCP, is not here; the TCP pass works from RFC 9293, which is the
current specification of TCP and holds the RFC 1122 rules already.
