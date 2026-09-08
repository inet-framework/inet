# IPv4 — feature map and support matrix

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `IPV4-F-*` · **Stands on:** [standards.md](standards.md), [rfc791/catalog.md](../../standard/rfc791/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. The support of each feature is in
  [`coverage.md`](../../model/ipv4/coverage.md).

The feature list itself comes from the standard texts only. Only the support column comes
from a run. The comparison of the support column against the standards that the model
claims to implement is [`conformance.md`](../../model/ipv4/conformance.md).

## Index

| ID | Feature |
| --- | --- |
| [IPV4-F-TTL](#ipv4-f-ttl) | A datagram carries a time to live, every hop decreases it, and a datagram that reaches zero dies. |
| [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation) | A module splits a datagram that is larger than the MTU of the next link. |
| [IPV4-F-REASSEMBLY](#ipv4-f-reassembly) | The destination collects the fragments and delivers the original datagram upward. |
| [IPV4-F-DONT-FRAGMENT](#ipv4-f-dont-fragment) | A datagram marked don't fragment is discarded instead of split. |
| [IPV4-F-IDENTIFICATION](#ipv4-f-identification) | Each datagram gets a unique identification value, and every fragment keeps it. |
| [IPV4-F-HEADER-CHECKSUM](#ipv4-f-header-checksum) | Every module recomputes and verifies the header checksum, and discards a bad header. |
| [IPV4-F-MIN-SIZE](#ipv4-f-min-size) | Every module forwards 68 octets whole, and every host accepts 576 octets. |
| [IPV4-F-ERROR-REPORT](#ipv4-f-error-report) | A module that discards a datagram may report the failure with an ICMP message. |
| [IPV4-F-DELIVERY](#ipv4-f-delivery) | A datagram crosses the gateway and reaches the addressed program with its data and source address intact. |
| [IPV4-F-HEADER](#ipv4-f-header) | Every datagram carries a version-4 header whose length fields describe it correctly. |

The support of each feature — what the run actually showed — is **not** in this document.
It lives in the coverage ledger, [`coverage.md`](../../model/ipv4/coverage.md). Keeping it out is
deliberate: this map states which capabilities the standards define, so a new run must
never force an edit here.

## Summary table


| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [IPV4-F-TTL](#ipv4-f-ttl) | mandatory | RFC 791 §3.1, §3.2 | RFC791-TTL-1, RFC791-TTL-2 |
| [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation) | mandatory | RFC 791 §2.3, §3.2 | RFC791-FRAG-1..4 |
| [IPV4-F-REASSEMBLY](#ipv4-f-reassembly) | unstated | RFC 791 §2.3 | RFC791-REASM-1 |
| [IPV4-F-DONT-FRAGMENT](#ipv4-f-dont-fragment) | mandatory | RFC 791 §2.3 | RFC791-FRAG-5 |
| [IPV4-F-IDENTIFICATION](#ipv4-f-identification) | mandatory | RFC 791 §2.3 | RFC791-ID-1, RFC791-FRAG-3 |
| [IPV4-F-HEADER-CHECKSUM](#ipv4-f-header-checksum) | mandatory | RFC 791 §1.4, §3.1 | RFC791-CKSUM-1, RFC791-CKSUM-2 |
| [IPV4-F-MIN-SIZE](#ipv4-f-min-size) | mandatory | RFC 791 §3.1, §3.2 | RFC791-FRAG-6, RFC791-REASM-2 |
| [IPV4-F-ERROR-REPORT](#ipv4-f-error-report) | optional | RFC 792 | RFC792-DU-4, RFC792-TE-1 |
| [IPV4-F-DELIVERY](#ipv4-f-delivery) | mandatory | RFC 791 §2.2, §2.4, §3.1 | RFC791-FWD-1, RFC791-DLV-1, RFC791-PROTO-1 |
| [IPV4-F-HEADER](#ipv4-f-header) | mandatory | RFC 791 §3.1 | RFC791-HDR-1, RFC791-HDR-2, RFC791-HDR-3 |

Ten features. What a run showed about each one is in [`coverage.md`](../../model/ipv4/coverage.md).

One level reads `unstated`. RFC 791 predates RFC 2119 and writes reassembly as plain
description of a procedure, without an obligation on the host. RFC 1122 §3.3.2 adds the
keyword, so that level will change on the day RFC 1122 enters the in-scope set. The row
waits in [`standards.md`](standards.md#override-table). Several other features rest on the
`only path` reason rather than on a keyword, for the same historical cause; each one names
it.

## IPV4-F-TTL

**A datagram carries a time to live, every hop decreases it, and a datagram that reaches
zero dies.**

- **Sources** — RFC 791 §3.2 Time to Live, `rfc791.txt:1976-1984`; §3.1 field definition,
  `rfc791.txt:1013-1014`. No later in-scope document changes them.
- **Level** — mandatory (reason: keyword). "This field must be decreased at each point that the internet
  header is processed" and "If this field contains the value zero, then the datagram must
  be destroyed" (`rfc791.txt:1981-1982`, `rfc791.txt:1013-1014`).
- **Description** — the field bounds the lifetime of a datagram in the network and stops a
  datagram that circulates in a loop.
- **Checks** — core: RFC791-TTL-1 (the decrease per hop), RFC791-TTL-2 (the death at
  zero). Supporting: RFC791-TTL-3 (the sender sets the field).

## IPV4-F-FRAGMENTATION

**A module that must forward a datagram larger than the MTU of the next link splits it
into fragments that carry the fragment fields of the original datagram.**

- **Sources** — RFC 791 §2.3 Fragmentation, `rfc791.txt:662-731`; §3.2,
  `rfc791.txt:1656-1671`.
- **Level** — mandatory (reason: keyword). "If an internet datagram is fragmented, its data portion must be
  broken on 8 octet boundaries" (`rfc791.txt:1658-1659`).
- **Description** — the split preserves the identification, marks every fragment but the
  last with the more-fragments flag, and places each piece with an offset in 8-octet
  units.
- **Checks** — core: RFC791-FRAG-1 (zero fragmentation information when whole),
  RFC791-FRAG-2 (the 8-octet boundary), RFC791-FRAG-3 (the identification is copied),
  RFC791-FRAG-4 (the offset and flag pattern of the first and last fragment).

## IPV4-F-REASSEMBLY

**The destination collects the fragments of one datagram and delivers the original
datagram upward.**

- **Sources** — RFC 791 §2.3, `rfc791.txt:723-729`.
- **Level** — unstated (reason: no keyword, and the text describes the procedure without obligating a host to run it). The RFC writes the procedure as description: "The combination is
  done by placing the data portion of each fragment in the relative position indicated by
  the fragment offset". RFC 1122 §3.3.2 `Reassembly` states the host obligation with a
  keyword, but RFC 1122 is out of scope today.
- **Description** — the four fields identification, source, destination, and protocol
  select the fragments that belong together.
- **Checks** — core: RFC791-REASM-1. Supporting: RFC791-REASM-3 (fragments of different
  datagrams do not mix).

## IPV4-F-DONT-FRAGMENT

**A datagram marked don't fragment is never fragmented; a module that would have to
fragment it discards it instead.**

- **Sources** — RFC 791 §2.3, `rfc791.txt:662-666`.
- **Level** — mandatory (reason: only path; the words carry obligation without a keyword). The RFC uses obligation words without a keyword: "Any internet
  datagram so marked is not to be internet fragmented under any circumstances" and "it is
  to be discarded instead".
- **Description** — the flag lets a sender forbid the split, at the price of the datagram
  when the path cannot carry it whole.
- **Checks** — core: RFC791-FRAG-5. The error report that follows the discard belongs to
  [IPV4-F-ERROR-REPORT](#ipv4-f-error-report), not to this feature: the discard is
  mandatory and the report is not.

## IPV4-F-IDENTIFICATION

**The sender gives each datagram an identification value that is unique for its source,
destination, and protocol while the datagram lives, and every fragment keeps it.**

- **Sources** — RFC 791 §2.3, `rfc791.txt:683-687` (uniqueness) and
  `rfc791.txt:692-694` (the copy into each fragment).
- **Level** — mandatory (reason: keyword). "sets the identification field to a value that must be unique for
  that source-destination pair and protocol for the time the datagram will be active".
- **Description** — the field is what makes reassembly possible; without uniqueness the
  destination mixes the fragments of two datagrams.
- **Checks** — core: RFC791-ID-1 (uniqueness across datagrams), RFC791-FRAG-3 (the value
  survives the split). RFC791-FRAG-3 is core for this feature and for
  [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation); one check may serve two features.
- **Note for a later pass** — RFC 6864 §4.1 and §4.3 rewrite this requirement. Bring the
  document into scope together with the RFC791-ID-1 test, so the test targets the
  governing text and not the 1981 text.

## IPV4-F-HEADER-CHECKSUM

**The header carries a checksum, every module that processes the header recomputes and
verifies it, and a header that fails is discarded.**

- **Sources** — RFC 791 §3.1, `rfc791.txt:1031-1033`; §1.4, `rfc791.txt:365-366`.
- **Level** — mandatory (reason: only path; the field is a fixed part of the header format, and recomputation at each point is the only integrity mechanism the header has). Both sentences are description without a keyword: "this is
  recomputed and verified at each point that the internet header is processed" and "the
  internet datagram is discarded at once by the entity which detects the error". RFC 1122
  §3.2.1 gives the IPv4 host rules their keywords.
- **Description** — the checksum protects the header only, and it must change whenever a
  field of the header changes, for example the time to live.
- **Checks** — core: RFC791-CKSUM-1 (the recompute per hop), RFC791-CKSUM-2 (the discard
  of a bad header).

## IPV4-F-MIN-SIZE

**Every module forwards a 68-octet datagram without fragmentation, and every host accepts
a 576-octet datagram.**

- **Sources** — RFC 791 §3.2, `rfc791.txt:1669-1671` (the 68-octet floor); §3.1,
  `rfc791.txt:961-963` (the 576-octet floor).
- **Level** — mandatory (reason: keyword). Both sentences use "must".
- **Description** — the two floors are what let a sender assume a minimum service from an
  unknown path.
- **Checks** — core: RFC791-FRAG-6 (68 octets pass without a split), RFC791-REASM-2 (576
  octets arrive, whole or in fragments).

## IPV4-F-ERROR-REPORT

**A module that discards a datagram may report the failure to the source with an ICMP
message.**

- **Sources** — RFC 792, `rfc792.txt:261-264` and the code list at `rfc792.txt:215`
  (destination unreachable, code 4); `rfc792.txt:344-356` (time exceeded).
- **Level** — optional (reason: keyword, `may`). In both messages the discard is a must and the report is a may:
  "the gateway must discard the datagram and may return a destination unreachable
  message". The mandatory half of each pair belongs to the RFC 791 feature that owns the
  discard.
- **Description** — the report turns a silent drop into a signal that the source can act
  on. The RFC permits silence, so the absence of a report is not a violation.
- **Checks** — core: RFC792-DU-4 (the report after a don't-fragment discard), RFC792-TE-1
  (the report after a TTL-zero discard).
- **Note on the strength** — a low support value on an optional feature is not a defect.
  The model may stay silent and still conform. A support value states what the checks saw,
  and [`conformance.md`](../../model/ipv4/conformance.md) reads it together with the level.

## IPV4-F-DELIVERY

**A datagram sent to a host in another network crosses the gateway and reaches the
addressed program, with its data and its source address intact.**

- **Sources** — RFC 791 §2.2, `rfc791.txt:542-561`; §2.4, `rfc791.txt:735-736`; §3.1
  protocol field, `rfc791.txt:1023-1026`.
- **Level** — mandatory (reason: only path). The document names no other way for data to
  reach a host in another network; the model of operation in §2.2 is the protocol.
- **Description** — the source addresses the datagram, a gateway forwards it by the
  destination address, and the destination hands the data to the protocol that the header
  names. This is the feature the other nine exist to serve.
- **Checks** — core: RFC791-FWD-1 (the gateway forwards, addresses intact), RFC791-DLV-1
  (the addressed program receives the data), RFC791-PROTO-1 (the data goes to the named
  protocol).

## IPV4-F-HEADER

**Every datagram carries a version-4 header whose length fields describe it correctly.**

- **Sources** — RFC 791 §3.1, `rfc791.txt:858-867` and `rfc791.txt:956-959`.
- **Level** — mandatory (reason: only path). The header format is the protocol: a datagram
  in another format is not an IPv4 datagram, and every other feature reads its fields from
  this header.
- **Description** — the version is 4, the header length counts 32-bit words and is at
  least 5, and the total length counts header and data in octets.
- **Checks** — core: RFC791-HDR-1, RFC791-HDR-2, RFC791-HDR-3. Supporting: none in this
  pass; the options, which change the header length, are out of scope.

## Coverage of the catalogs

Step 4 requires that every area of every in-scope catalog appears in at least one feature,
or that this map marks the area as out of scope.

| Catalog area | Feature |
| --- | --- |
| RFC 791, Time to live | IPV4-F-TTL |
| RFC 791, Fragmentation | IPV4-F-FRAGMENTATION, IPV4-F-DONT-FRAGMENT, IPV4-F-MIN-SIZE, IPV4-F-IDENTIFICATION |
| RFC 791, Reassembly | IPV4-F-REASSEMBLY, IPV4-F-MIN-SIZE |
| RFC 791, Header checksum | IPV4-F-HEADER-CHECKSUM |
| RFC 791, Identification | IPV4-F-IDENTIFICATION |
| RFC 791, Header format | IPV4-F-HEADER, IPV4-F-DELIVERY (the protocol field) |
| RFC 791, Delivery and forwarding | IPV4-F-DELIVERY |
| RFC 792, Error signals | IPV4-F-ERROR-REPORT |

All 23 catalog entries of the in-scope set appear in the map, and no entry appears in none.

Out of scope in the map, because the catalogs put them out of scope: options, type of
service and precedence, the security annex, the reassembly timer, and the ICMP messages
other than the two error reports. Each becomes a feature on the day its catalog entries
exist.
