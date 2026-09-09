# RFC 4443 (ICMPv6) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC4443-*` · **Stands on:** [standards.md](../../protocol/ipv6/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 4443. RFC 8200 delegates its error reports to ICMPv6, so every entry
here pairs with an entry of [`rfc8200/catalog.md`](../rfc8200/catalog.md). The catalog
comes from the RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc4443.txt` — Internet Control Message Protocol (ICMPv6) for the Internet Protocol
  Version 6 (IPv6) Specification, March 2006. Downloaded 2026-09-09 from
  <https://www.rfc-editor.org/rfc/rfc4443.txt>.

The scope of this catalog: the general rules of §2 (the source address of a message and the
processing rules of §2.4) and the error messages of §3. The informational messages of §4
(echo request and reply) belong to an ICMPv6 protocol folder and stay out.

Quotes are verbatim. A reference such as `rfc4443.txt:548` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipv6/coverage.md`](../../model/ipv6/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC4443-PTB-1](#rfc4443-ptb-1) | A router that cannot forward a packet because of the MTU sends Packet Too Big. |
| [RFC4443-PTB-2](#rfc4443-ptb-2) | The MTU field of Packet Too Big is the MTU of the next-hop link; the code is 0. |
| [RFC4443-TE-1](#rfc4443-te-1) | A router that receives or produces a hop limit of zero discards the packet and sends Time Exceeded code 0. |
| [RFC4443-DU-4](#rfc4443-du-4) | A destination should send Destination Unreachable code 4 for a port without a listener. |
| [RFC4443-ERR-1](#rfc4443-err-1) | An error message includes as much of the invoking packet as fits in 1280 octets. |
| [RFC4443-ERR-2](#rfc4443-err-2) | An error message is addressed to the source of the invoking packet. |
| [RFC4443-SRC-1](#rfc4443-src-1) | A response to a message sent to one of the node's unicast addresses carries that address as source. |
| [RFC4443-SRC-2](#rfc4443-src-2) | Otherwise the source is a unicast address of the node, chosen as for any packet. |
| [RFC4443-MPR-1](#rfc4443-mpr-1) | An error message of unknown type is passed to the upper-layer process. |
| [RFC4443-MPR-2](#rfc4443-mpr-2) | An informational message of unknown type is silently discarded. |
| [RFC4443-MPR-3](#rfc4443-mpr-3) | The upper-layer protocol of a received error is taken from the quoted packet. |
| [RFC4443-MPR-4](#rfc4443-mpr-4) | No error message about an ICMPv6 error message. |
| [RFC4443-MPR-5](#rfc4443-mpr-5) | No error message about an ICMPv6 redirect message. |
| [RFC4443-MPR-6](#rfc4443-mpr-6) | No error message about a packet to a multicast address, except Packet Too Big and Parameter Problem code 2. |
| [RFC4443-MPR-7](#rfc4443-mpr-7) | No error message about a packet sent as a link-layer multicast, with the same exceptions. |
| [RFC4443-MPR-8](#rfc4443-mpr-8) | No error message about a packet sent as a link-layer broadcast, with the same exceptions. |
| [RFC4443-MPR-9](#rfc4443-mpr-9) | No error message about a packet whose source does not identify a single node. |
| [RFC4443-MPR-10](#rfc4443-mpr-10) | A node limits the rate of the error messages it originates. |
| [RFC4443-DU-C](#rfc4443-du-c) | No ICMPv6 message for a packet dropped because of congestion. |
| [RFC4443-DU-P](#rfc4443-du-p) | A router does not forward a packet back onto a point-to-point arrival link. |
| [RFC4443-UL-1](#rfc4443-ul-1) | A received Destination Unreachable is reported to the upper-layer process. |
| [RFC4443-UL-2](#rfc4443-ul-2) | A received Packet Too Big is passed to the upper-layer process. |
| [RFC4443-UL-3](#rfc4443-ul-3) | A received Time Exceeded is passed to the upper-layer process. |

The conventions of an entry — strength, class, and the `Overridden by` field — are the ones
of [`rfc8200/catalog.md`](../rfc8200/catalog.md#how-to-read-an-entry). RFC 4443 uses the
RFC 2119 keywords; a MUST is a `must` and a SHOULD a `should`.

## Error signals for the RFC 8200 checks

### RFC4443-PTB-1

**A router sends Packet Too Big when it cannot forward a packet because the packet is larger
than the MTU of the outgoing link.**

> "A Packet Too Big MUST be sent by a router in response to a packet that it cannot
> forward because the packet is larger than the MTU of the outgoing link.  The information
> in this message is used as part of the Path MTU Discovery process [PMTU]." — §3.2,
> `rfc4443.txt:548-551`

- Strength: must. Class: error-signal.
- Pairs with [RFC8200-FRAG-1](../rfc8200/catalog.md#rfc8200-frag-1): the router does not
  fragment, so the report is the only thing it can do.
- Check idea: send a packet larger than the MTU of the link after the gateway. The source
  receives a message of type 2.

### RFC4443-PTB-2

**The MTU field of a Packet Too Big message carries the MTU of the next-hop link, and the
code is 0.**

> "Code           Set to 0 (zero) by the originator and ignored by the receiver." — §3.2,
> `rfc4443.txt:541-542`
> "MTU            The Maximum Transmission Unit of the next-hop link." — §3.2,
> `rfc4443.txt:544`

- Strength: description (the field definitions of a mandatory message). Class:
  error-signal (the content of the message).
- Check idea: with a known MTU on the link after the gateway, the MTU field of the
  message equals that value. Path MTU discovery (RFC 8201) rests on this field.

### RFC4443-TE-1

**A router that receives a packet with hop limit zero, or decrements it to zero, discards
the packet and sends Time Exceeded code 0 to the source.**

> "If a router receives a packet with a Hop Limit of zero, or if a router decrements a
> packet's Hop Limit to zero, it MUST discard the packet and originate an ICMPv6 Time
> Exceeded message with Code 0 to the source of the packet.  This indicates either a
> routing loop or too small an initial Hop Limit value." — §3.3, `rfc4443.txt:605-609`

- Strength: must. Class: error-signal.
- Pairs with [RFC8200-HL-2](../rfc8200/catalog.md#rfc8200-hl-2). Unlike the IPv4 report,
  the message is not optional.

### RFC4443-DU-4

**A destination should send Destination Unreachable code 4 when the transport protocol has
no listener for the packet and no way of its own to tell the sender.**

> "A destination node SHOULD originate a Destination Unreachable message with Code 4 in
> response to a packet for which the transport protocol (e.g., UDP) has no listener, if
> that transport protocol has no alternative means to inform the sender." — §3.1,
> `rfc4443.txt:481-484`

- Strength: should. Class: error-signal.
- Note: the transport side of this rule belongs to the UDP catalogs; here it is the host's
  error report of the IPv6 layer.

### RFC4443-ERR-1

**An error message includes as much of the invoking packet as fits without exceeding the
minimum IPv6 MTU.**

> "(c) Every ICMPv6 error message (type < 128) MUST include as much of the IPv6 offending
> (invoking) packet (the packet that caused the error) as possible without making the
> error message packet exceed the minimum IPv6 MTU [IPv6]." — §2.4, `rfc4443.txt:295-298`

- Strength: must. Class: error-signal (the content of the message).
- Check idea: a report of a small invoking packet quotes the whole packet; a report of a
  large one is at most 1280 octets long. The content of a report is level 3 work.

### RFC4443-ERR-2

**An error message is addressed to the source of the invoking packet.**

> "Destination Address   Copied from the Source Address field of the invoking packet." —
> §3.1, `rfc4443.txt:416-419`; the same words in §3.2, `rfc4443.txt:532-535`, and §3.3,
> `rfc4443.txt:588-590`

- Strength: description. Class: error-signal.
- Check idea: the report of a packet from host A arrives at host A.

## Source address of a message

### RFC4443-SRC-1

**A message that responds to a message sent to one of the node's unicast addresses carries
that address as its source.**

> "If the node has more than one unicast address, it MUST choose the Source Address of the
> message as follows: (a) If the message is a response to a message sent to one of the
> node's unicast addresses, the Source Address of the reply MUST be that same address." —
> §2.2, `rfc4443.txt:240-245`

- Strength: must. Class: error-signal (a field of the message).
- Check idea: a host's report about a datagram sent to its address carries that address as
  source.

### RFC4443-SRC-2

**A message that responds to a message sent to any other address carries a unicast address
of the node, chosen as the source of any other packet would be.**

> "(b) If the message is a response to a message sent to any other address, such as a
> multicast group address, an anycast address implemented by the node, or a unicast
> address that does not belong to the node the Source Address of the ICMPv6 packet MUST be
> a unicast address belonging to the node.  The address SHOULD be chosen according to the
> rules that would be used to select the source address for any other packet originated by
> the node, given the destination address of the packet." — §2.2, `rfc4443.txt:247-258`

- Strength: must (a unicast address of the node); should (the usual selection). Class:
  error-signal (a field of the message).
- Check idea: a router's report about a packet it forwarded carries one of the router's own
  unicast addresses, the one of the interface toward the source.

## Message processing rules

The rules of §2.4 (a) to (f). The note that closes them: "THE RESTRICTIONS UNDER (e) AND (f)
ABOVE TAKE PRECEDENCE OVER ANY REQUIREMENT ELSEWHERE IN THIS DOCUMENT FOR ORIGINATING ICMP
ERROR MESSAGES." — `rfc4443.txt:379-381`

### RFC4443-MPR-1

**An error message of unknown type is passed to the upper-layer process that sent the
invoking packet, where that process can be identified.**

> "(a) If an ICMPv6 error message of unknown type is received at its destination, it MUST
> be passed to the upper-layer process that originated the packet that caused the error,
> where this can be identified (see Section 2.4, (d))." — §2.4, `rfc4443.txt:287-290`

- Strength: must. Class: internal (a handoff), with a wire half: the node stays silent about
  it, by [RFC4443-MPR-4](#rfc4443-mpr-4).
- Check idea: deliver an error message of an unassigned type below 128. The node sends
  nothing back and keeps running; the handoff itself needs a module test.

### RFC4443-MPR-2

**An informational message of unknown type is silently discarded.**

> "(b) If an ICMPv6 informational message of unknown type is received, it MUST be silently
> discarded." — §2.4, `rfc4443.txt:292-293`

- Strength: must. Class: end-to-end (absence) plus error-signal (absence).
- Check idea: deliver a message of an unassigned type of 128 or above. The node sends
  nothing back, neither a reply nor an error, and keeps running.

### RFC4443-MPR-3

**The upper-layer protocol that handles a received error is taken from the quoted packet.**

> "(d) In cases where the internet-layer protocol is required to pass an ICMPv6 error
> message to the upper-layer process, the upper-layer protocol type is extracted from the
> original packet (contained in the body of the ICMPv6 error message) and used to select
> the appropriate upper-layer process to handle the error." — §2.4, `rfc4443.txt:300-304`

- Strength: description (the mechanism of the MUSTs in (a) and §3). Class: internal.

### RFC4443-MPR-4

**No error message is originated about an ICMPv6 error message.**

> "(e) An ICMPv6 error message MUST NOT be originated as a result of receiving the
> following: (e.1) An ICMPv6 error message." — §2.4, `rfc4443.txt:317-320`

- Strength: must not. Class: error-signal (absence).
- Check idea: let an error message run into an error of its own, for example a hop limit
  that expires at a router. The router sends no Time Exceeded.

### RFC4443-MPR-5

**No error message is originated about an ICMPv6 redirect message.**

> "(e.2) An ICMPv6 redirect message [IPv6-DISC]." — §2.4, `rfc4443.txt:322`

- Strength: must not. Class: error-signal (absence).
- Note: needs a redirect in flight; neighbor discovery is out of scope, so this entry waits
  for a scenario that produces one.

### RFC4443-MPR-6

**No error message is originated about a packet destined to a multicast address, except
Packet Too Big and Parameter Problem code 2.**

> "(e.3) A packet destined to an IPv6 multicast address.  (There are two exceptions to this
> rule: (1) the Packet Too Big Message (Section 3.2) to allow Path MTU discovery to work for
> IPv6 multicast, and (2) the Parameter Problem Message, Code 2 (Section 3.4) reporting an
> unrecognized IPv6 option (see Section 4.2 of [IPv6]) that has the Option Type highest-
> order two bits set to 10)." — §2.4, `rfc4443.txt:324-330`

- Strength: must not, with two exceptions. Class: error-signal (absence).
- Check idea: send a datagram to the all-nodes multicast address and to a port that no
  program opened. No node answers with Destination Unreachable.

### RFC4443-MPR-7

**No error message is originated about a packet sent as a link-layer multicast, with the
same exceptions.**

> "(e.4) A packet sent as a link-layer multicast (the exceptions from e.3 apply to this
> case, too)." — §2.4, `rfc4443.txt:332-333`

- Strength: must not. Class: error-signal (absence).
- Check idea: deliver a unicast datagram to a closed port inside a frame whose link-layer
  destination is a multicast address. The host sends no Destination Unreachable.

### RFC4443-MPR-8

**No error message is originated about a packet sent as a link-layer broadcast, with the
same exceptions.**

> "(e.5) A packet sent as a link-layer broadcast (the exceptions from e.3 apply to this
> case, too)." — §2.4, `rfc4443.txt:343-344`

- Strength: must not. Class: error-signal (absence).
- Check idea: the same with the link-layer broadcast address.

### RFC4443-MPR-9

**No error message is originated about a packet whose source address does not identify a
single node.**

> "(e.6) A packet whose source address does not uniquely identify a single node -- e.g.,
> the IPv6 Unspecified Address, an IPv6 multicast address, or an address known by the ICMP
> message originator to be an IPv6 anycast address." — §2.4, `rfc4443.txt:346-349`

- Strength: must not. Class: error-signal (absence).
- Check idea: deliver a datagram with the unspecified address as source to a closed port.
  The host sends no Destination Unreachable.

### RFC4443-MPR-10

**A node limits the rate of the error messages it originates.**

> "(f) Finally, in order to limit the bandwidth and forwarding costs incurred by originating
> ICMPv6 error messages, an IPv6 node MUST limit the rate of ICMPv6 error messages it
> originates." — §2.4, `rfc4443.txt:351-353`; "The rate-limiting parameters SHOULD be
> configurable." — `rfc4443.txt:372`

- Strength: must; should (configurable). Class: wire, as a rate.
- Note: a rate is a distribution; level 4.

## Destination Unreachable, further rules

### RFC4443-DU-C

**No ICMPv6 message is generated for a packet dropped because of congestion.**

> "(An ICMPv6 message MUST NOT be generated if a packet is dropped due to congestion.)" —
> §3.1, `rfc4443.txt:442-443`

- Strength: must not. Class: error-signal (absence).
- Note: needs a congested queue; level 4.

### RFC4443-DU-P

**A router does not forward, back onto a point-to-point arrival link, a packet destined to
an address within a subnet assigned to that link.**

> "One specific case in which a Destination Unreachable message is sent with a code 3 is in
> response to a packet received by a router from a point-to-point link, destined to an
> address within a subnet assigned to that same link (other than one of the receiving
> router's own addresses).  In such a case, the packet MUST NOT be forwarded back onto the
> arrival link." — §3.1, `rfc4443.txt:474-479`

- Strength: must not. Class: wire (absence) plus error-signal.
- Note: needs a point-to-point link in the mockup; a later pass.

## Upper-layer notification

### RFC4443-UL-1

**A received Destination Unreachable is reported to the upper-layer process.**

> "A node receiving the ICMPv6 Destination Unreachable message MUST notify the upper-layer
> process if the relevant process can be identified (see Section 2.4, (d))." — §3.1,
> `rfc4443.txt:513-515`

- Strength: must. Class: internal (a handoff).

### RFC4443-UL-2

**A received Packet Too Big is passed to the upper-layer process.**

> "An incoming Packet Too Big message MUST be passed to the upper-layer process if the
> relevant process can be identified (see Section 2.4, (d))." — §3.2,
> `rfc4443.txt:569-571`

- Strength: must. Class: internal (a handoff).

### RFC4443-UL-3

**A received Time Exceeded is passed to the upper-layer process.**

> "An incoming Time Exceeded message MUST be passed to the upper-layer process if the
> relevant process can be identified (see Section 2.4, (d))." — §3.3,
> `rfc4443.txt:625-627`

- Strength: must. Class: internal (a handoff).

## Out of scope in this catalog

The checksum of §2.3 (an encoding statement; see
[RFC8200-CKSUM-2](../rfc8200/catalog.md#rfc8200-cksum-2)), the Destination Unreachable codes
other than 4 (routing and policy reasons that the mockup cannot produce), Time Exceeded
code 1 (the reassembly timer, level 4), the Parameter Problem message as a message of its
own (its codes appear as the reports of the RFC 8200 statements), and the informational
messages of §4 (echo request and reply, including the echo responder that every node must
implement), which belong to an ICMPv6 protocol folder.
