# UDP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `UDP-F-*` · **Stands on:** [standards.md](standards.md), [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/udp/coverage.md), and step 8 compares it with the claims of
  the model in [`conformance.md`](../../model/udp/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what
the run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/udp/coverage.md). Keeping it out is deliberate: this map states
which capabilities the standards define, so a new run must never force an edit here.

## Index

| ID | Feature |
| --- | --- |
| [UDP-F-DELIVERY](#udp-f-delivery) | A datagram sent to a port at an address reaches the program that opened that port, with its data and its source. |
| [UDP-F-HEADER](#udp-f-header) | Every datagram carries the 8-octet header, and the length field counts header and data. |
| [UDP-F-CHECKSUM](#udp-f-checksum) | A sender may protect the datagram with a checksum over a pseudo header, the header and the data; zero means none. |
| [UDP-F-PORT-UNREACHABLE](#udp-f-port-unreachable) | A datagram to a port that no program opened may be answered with an ICMP port unreachable report. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [UDP-F-DELIVERY](#udp-f-delivery) | mandatory | RFC 768 Fields, User Interface, Protocol Number | RFC768-HDR-2, RFC768-PROTO-1, RFC768-UI-1 |
| [UDP-F-HEADER](#udp-f-header) | mandatory | RFC 768 Format, Fields | RFC768-HDR-3 |
| [UDP-F-CHECKSUM](#udp-f-checksum) | optional | RFC 768 Fields | RFC768-CKSUM-2 |
| [UDP-F-PORT-UNREACHABLE](#udp-f-port-unreachable) | optional | RFC 792 | RFC792-DU-3 |

Four features, two mandatory. UDP is a thin protocol by design — "a minimum of protocol
mechanism", in the words of its introduction — and the map is correspondingly short. What a
run showed about each feature is in [`coverage.md`](../../model/udp/coverage.md).

## UDP-F-DELIVERY

**A datagram sent to a port at an address reaches the program that opened that port, with
its data and its source.**

- **Sources** — RFC 768 Fields, `rfc768.txt:66-67`; User Interface, `rfc768.txt:100-108`;
  Protocol Number, `rfc768.txt:144`; IP Interface, `rfc768.txt:127-128`.
- **Level** — mandatory (reason: only path). Delivery to a port is the one service UDP
  provides; the document gives no other. No core statement carries a keyword: HDR-2 and
  PROTO-1 are description and UI-1 is a `should`.
- **Description** — the destination port selects the program within the address, the
  datagram travels as IP protocol 17, and the program receives the data with the source
  port and the source address.
- **Checks** — core: RFC768-HDR-2 (the port selects the receiver), RFC768-PROTO-1
  (protocol 17), RFC768-UI-1 (the program receives data and source). Supporting:
  RFC768-IP-1 (the module reads the addresses and the protocol from IP; its visible effects
  are UI-1 and the checksum), RFC768-HDR-1 (a meaningful source port).

## UDP-F-HEADER

**Every datagram carries the 8-octet header, and the length field counts header and data.**

- **Sources** — RFC 768 Format, `rfc768.txt:31-43`; Fields, `rfc768.txt:48-71`.
- **Level** — mandatory (reason: only path). The header format is the protocol.
- **Description** — four 16-bit fields: source port, destination port, length, checksum.
  The length counts the header and the data, so an empty datagram has length 8.
- **Checks** — core: RFC768-HDR-3 (the length rule, including the minimum). Supporting:
  RFC768-HDR-1 (the source port is zero when unused).

## UDP-F-CHECKSUM

**A sender may protect the datagram with a checksum over a pseudo header, the header and
the data; an all-zero checksum means none was generated.**

- **Sources** — RFC 768 Fields, `rfc768.txt:73-95`.
- **Level** — optional (reason: the text permits a transmitter to generate no checksum,
  `rfc768.txt:93-95`). The derivation rule of the guide names `may` and `should`; RFC 768
  predates the keywords and writes the permission as a sentence. RFC 1122 §4.1.3.4 turns
  the feature into a mandatory one, and is out of scope at level 2; see
  [`standards.md`](standards.md#override-table).
- **Description** — the checksum covers a pseudo header of addresses, protocol and length,
  which protects against misrouted datagrams. A computed zero is sent as all ones so that
  zero can mean "none".
- **Checks** — core: RFC768-CKSUM-2 (nonzero when generated, zero when not, both accepted).
  Supporting: RFC768-CKSUM-1 (the arithmetic; a serializer unit test).

## UDP-F-PORT-UNREACHABLE

**A datagram to a port that no program opened may be answered with an ICMP destination
unreachable report, code 3.**

- **Sources** — RFC 792, the destination unreachable message; see
  [RFC792-DU-3](../../standard/rfc792/catalog.md#rfc792-du-3).
- **Level** — optional (reason: keyword, `may`).
- **Description** — the destination host may tell the source that nothing listens on the
  port. Silence also conforms.
- **Checks** — core: RFC792-DU-3. The complementary half — that the datagram reaches no
  program — is RFC768-HDR-2 and belongs to [UDP-F-DELIVERY](#udp-f-delivery).

## Coverage of the catalogs

| Catalog area | Feature |
| --- | --- |
| RFC 768, Header | UDP-F-HEADER, UDP-F-DELIVERY |
| RFC 768, Checksum | UDP-F-CHECKSUM |
| RFC 768, Interface | UDP-F-DELIVERY |
| RFC 768, Protocol number | UDP-F-DELIVERY |
| RFC 792, Error signals (port unreachable) | UDP-F-PORT-UNREACHABLE |

All 8 entries of the RFC 768 catalog and the one RFC 792 entry that serves UDP appear in
the map.
