# RFC 8504 (IPv6 node requirements) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8504-*` · **Stands on:** [standards.md](../../protocol/ipv6/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 8504, the sections of §5 that the IPv6 standards map pins
([`standards.md`](../../protocol/ipv6/standards.md#in-scope-set)). The catalog comes from
the RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc8504.txt` — IPv6 Node Requirements, January 2019, Best Current Practice 220.
  Downloaded 2026-09-09 from <https://www.rfc-editor.org/rfc/rfc8504.txt>.

RFC 8504 is the RFC 1122 of IPv6, with one difference of form: most of its sentences say
which other document a node MUST, SHOULD or MAY implement, and only some state a behavior of
their own. The document-level requirements are in the catalog too, because they are
mandatory statements of an in-scope document; their class is `implementation`, and a check
of such a statement is the pass over the document it names. The behavior-level statements
of §5.1 and §5.2 restate and sharpen RFC 8200 §4.5, and the RFC 8200 entries that they
govern carry an `Overridden by` field.

Quotes are verbatim. A reference such as `rfc8504.txt:327` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipv6/coverage.md`](../../model/ipv6/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC8504-NR-1](#rfc8504-nr-1) | A node supports RFC 8200 and follows its transmission rules. |
| [RFC8504-NR-2](#rfc8504-nr-2) | A node can send, receive, and process fragment headers. |
| [RFC8504-NR-3](#rfc8504-nr-3) | A node does not create overlapping fragments, and silently discards a datagram with one. |
| [RFC8504-NR-4](#rfc8504-nr-4) | A node does not generate atomic fragments; a receiver processes one as a whole packet. |
| [RFC8504-NR-5](#rfc8504-nr-5) | A node should avoid predictable fragment identifications. |
| [RFC8504-NR-6](#rfc8504-nr-6) | Unrecognized extension headers, options, and upper-layer protocols are processed as RFC 8200 says. |
| [RFC8504-NR-7](#rfc8504-nr-7) | A node processes the RFC 8200 extension headers; routing type 0 is unrecognized. |
| [RFC8504-NR-8](#rfc8504-nr-8) | The first fragment carries the whole header chain; otherwise it should be discarded with Parameter Problem code 3. |
| [RFC8504-NR-9](#rfc8504-nr-9) | Path MTU discovery should be supported. |
| [RFC8504-NR-10](#rfc8504-nr-10) | The fragmentation and reassembly rules of RFC 8200 and RFC 5722 are followed. |
| [RFC8504-NR-11](#rfc8504-nr-11) | Packet Too Big messages are not filtered. |
| [RFC8504-NR-12](#rfc8504-nr-12) | ICMPv6 is supported. |

## How to read an entry

- **Strength** — the keyword the RFC uses: `must`, `must not`, `should`, `should not`,
  `may`, or `description`. RFC 8504 uses the RFC 2119 keywords (§2).
- **Class** — how a test can observe the statement: `wire`, `end-to-end`, `error-signal`,
  `internal`, `encoding` as in [`rfc8200/catalog.md`](../rfc8200/catalog.md#how-to-read-an-entry),
  and one more: `implementation` — the statement names a document or a mechanism that a
  node must implement, and its check is the pass over that document.
- **Governs** — the RFC 8200 entry that this entry restates with a keyword or sharpens.

## Internet Protocol version 6

### RFC8504-NR-1

**A node supports RFC 8200 and follows its packet transmission rules.**

> "The Internet Protocol version 6 is specified in [RFC8200].  This specification MUST be
> supported." — §5.1, `rfc8504.txt:317-318`
> "The node MUST follow the packet transmission rules in RFC 8200." — §5.1,
> `rfc8504.txt:320`

- Strength: must. Class: implementation.
- Check idea: the whole RFC 8200 pass. A node that fails a mandatory RFC 8200 statement
  fails this one.

### RFC8504-NR-2

**A node can always send, receive, and process fragment headers.**

> "Nodes MUST always be able to send, receive, and process Fragment headers." — §5.1,
> `rfc8504.txt:324-325`

- Strength: must. Class: implementation, with a wire and end-to-end effect.
- Governs: it turns [RFC8200-FRAG-3](../rfc8200/catalog.md#rfc8200-frag-3) and its
  neighbors from a `may` mechanism into one every node has, and keeps
  [RFC8200-MTU-4](../rfc8200/catalog.md#rfc8200-mtu-4) a `must`.
- Check idea: a node fragments a packet larger than its link MTU, and reassembles fragments
  it receives.

### RFC8504-NR-3

**A node does not create overlapping fragments, and a receiver that finds an overlapping
fragment silently discards the whole datagram.**

> "IPv6 nodes MUST not create overlapping fragments.  Also, when reassembling an IPv6
> datagram, if one or more of its constituent fragments is determined to be an overlapping
> fragment, the entire datagram (and any constituent fragments) MUST be silently discarded.
> See [RFC5722] for more information." — §5.1, `rfc8504.txt:327-331`

- Strength: must not (create); must (discard). Class: wire (the sender) plus end-to-end
  (absence at the receiver).
- Governs: [RFC8200-FRAG-6](../rfc8200/catalog.md#rfc8200-frag-6) and
  [RFC8200-REASM-5](../rfc8200/catalog.md#rfc8200-reasm-5), with the word "silently".
- Check idea: deliver two fragments whose pieces overlap by a few octets. Nothing reaches
  the upper layer, and no report is sent.

### RFC8504-NR-4

**A node does not generate atomic fragments; a receiver processes an atomic fragment as a
fully reassembled packet and any matching fragments independently.**

> "As recommended in [RFC8021], nodes MUST NOT generate atomic fragments, i.e., where the
> fragment is a whole datagram.  As per [RFC6946], if a receiving node reassembling a
> datagram encounters an atomic fragment, it should be processed as a fully reassembled
> packet, and any other fragments that match this packet should be processed
> independently." — §5.1, `rfc8504.txt:343-348`

- Strength: must not (generate); should (process as whole). Class: wire (the sender) plus
  end-to-end (the receiver).
- Governs: [RFC8200-REASM-6](../rfc8200/catalog.md#rfc8200-reasm-6) for the receiver; the
  sender rule is new.
- Check idea: a fragmenting source never emits a fragment with offset 0 and M = 0. A
  receiver that gets one hands its content upward as a whole packet.

### RFC8504-NR-5

**A node should avoid predictable fragment identification values.**

> "To mitigate a variety of potential attacks, nodes SHOULD avoid using predictable Fragment
> Identification values in Fragment headers, as discussed in [RFC7739]." — §5.1,
> `rfc8504.txt:350-352`

- Strength: should. Class: wire.
- Check idea: the identifications of successive fragmented packets do not form a simple
  sequence. A counter declines the should.

## Extension headers

### RFC8504-NR-6

**Unrecognized extension headers, options, and upper-layer protocols are processed as
RFC 8200 §4 says.**

> "Any unrecognized extension headers or options MUST be processed as described in RFC 8200.
> Note that where Section 4 of RFC 8200 refers to the action to be taken when a Next Header
> value in the current header is not recognized by a node, that action applies whether the
> value is an unrecognized extension header or an unrecognized upper-layer protocol (ULP)."
> — §5.2, `rfc8504.txt:373-378`

- Strength: must. Class: end-to-end (absence) plus error-signal.
- Governs: [RFC8200-EXT-3](../rfc8200/catalog.md#rfc8200-ext-3): the "should" of RFC 8200
  becomes the action a node must take, for an unrecognized upper-layer protocol as well.
- Check idea: deliver a packet whose next header names no protocol the node knows. The
  node delivers nothing and sends Parameter Problem code 1 to the source.

### RFC8504-NR-7

**A node processes the extension headers of RFC 8200; routing header type 0 is treated as
an unrecognized routing type.**

> "An IPv6 node MUST be able to process these extension headers.  An exception is Routing
> Header type 0 (RH0), which was deprecated by [RFC5095] due to security concerns and which
> MUST be treated as an unrecognized routing type." — §5.2, `rfc8504.txt:380-383`

- Strength: must. Class: implementation.
- Note: the fragment header is the one extension header in scope at this level; the
  others are level 5 by the level table of the guide.

### RFC8504-NR-8

**The first fragment carries the entire header chain through the upper-layer header; a
receiver should discard a first fragment that does not, with Parameter Problem code 3.**

> "As per RFC 8200, when a node fragments an IPv6 datagram, it MUST include the entire IPv6
> Header Chain in the first fragment. [...] On reassembly, if the first fragment does not
> include all headers through an upper-layer header, then that fragment SHOULD be
> discarded and an ICMP Parameter Problem, Code 3, message SHOULD be sent to the source of
> the fragment, with the Pointer field set to zero." — §5.2, `rfc8504.txt:399-409`

- Strength: must (the sender); should (the receiver). Class: wire plus error-signal.
- Governs: [RFC8200-FRAG-4](../rfc8200/catalog.md#rfc8200-frag-4) item (3) and
  [RFC8200-REASM-7](../rfc8200/catalog.md#rfc8200-reasm-7).
- Check idea: the first fragment of a source carries the UDP header; a crafted first
  fragment without it is discarded with a report.

## Path MTU discovery and packet size

### RFC8504-NR-9

**Path MTU discovery should be supported.**

> ""Path MTU Discovery for IP version 6" [RFC8201] SHOULD be supported." — §5.7.1,
> `rfc8504.txt:602`

- Strength: should. Class: implementation.
- Note: RFC 8201 is level 4 work; see [RFC8200-MTU-3](../rfc8200/catalog.md#rfc8200-mtu-3).

### RFC8504-NR-10

**The fragmentation and reassembly rules of RFC 8200 and RFC 5722 are followed.**

> "The rules in [RFC8200] and [RFC5722] MUST be followed for packet fragmentation and
> reassembly." — §5.7.1, `rfc8504.txt:612-613`

- Strength: must. Class: implementation.
- Check idea: the fragmentation and reassembly checks of the RFC 8200 catalog, including
  the overlap rule of [RFC8504-NR-3](#rfc8504-nr-3).

### RFC8504-NR-11

**Packet Too Big messages are not filtered.**

> "Path MTU Discovery relies on ICMPv6 Packet Too Big (PTB) to determine the MTU of the path
> (and thus these MUST NOT be filtered, as per the recommendation in [RFC4890])." — §5.7.1,
> `rfc8504.txt:629-631`

- Strength: must not. Class: wire.
- Check idea: a Packet Too Big message sent by a router reaches the source.

## ICMPv6

### RFC8504-NR-12

**ICMPv6 is supported.**

> "ICMPv6 [RFC4443] MUST be supported.  "Extended ICMP to Support Multi-Part Messages"
> [RFC4884] MAY be supported." — §5.8, `rfc8504.txt:647-648`

- Strength: must (RFC 4443); may (RFC 4884). Class: implementation.
- Check idea: the RFC 4443 pass.

## Out of scope in this catalog

§4 (the sub-IP layer), §5.3 (the limits a host may place on options; every statement is a
`may` with a `should` inside it), §5.4 to §5.6 and §5.9 to §5.11 (neighbor discovery, SEND,
router advertisement flags, router preferences, first-hop selection, multicast listener
discovery: protocols of their own), §5.12 (ECN), the flow label sentences of §5.1, and
§6 to §17 (addressing, DNS, configuration, transition, applications, mobility, security,
router-specific functions, constrained devices, management). The reasons are in
[`standards.md`](../../protocol/ipv6/standards.md#in-scope-set).
