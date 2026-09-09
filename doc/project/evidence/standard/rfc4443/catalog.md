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

The scope of this catalog is narrow by intent: the two messages that report the failures of
the RFC 8200 base mechanisms, the report a host sends for a closed port, and the two rules
that shape every error message. The other messages and the processing rules of §2.4 are out
of scope in this pass.

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

## Out of scope in this catalog

The message processing rules of §2.4 other than (c): the rules for unknown types, the
delivery of a received error to the upper layer, the five cases in which no error is
originated, and the rate limit. They are the negative and the dynamic rules of level 3 and
level 4. Also out: the source address selection of §2.2, the checksum of §2.3 (an encoding
statement; see [RFC8200-CKSUM-2](../rfc8200/catalog.md#rfc8200-cksum-2)), the Destination
Unreachable codes other than 4, Time Exceeded code 1 (the reassembly timer, level 4),
Parameter Problem, and the informational messages of §4 (echo request and reply), which
belong to an ICMPv6 protocol folder.
