# UDP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `UDP-F-*` · **Stands on:** [standards.md](standards.md), [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md)

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
| [UDP-F-CHECKSUM](#udp-f-checksum) | A host protects the datagram with a checksum over a pseudo header, the header and the data, and checks the value it receives; zero means none. |
| [UDP-F-PORT-UNREACHABLE](#udp-f-port-unreachable) | A datagram to a port that no program opened may be answered with an ICMP port unreachable report. |
| [UDP-F-INPUT-VALIDATION](#udp-f-input-validation) | A datagram whose checksum is wrong, or whose source address is not a single host, is discarded without an answer. |
| [UDP-F-ERROR-DELIVERY](#udp-f-error-delivery) | An ICMP error message that reports a sent datagram reaches the program that sent it. |
| [UDP-F-APP-INTERFACE](#udp-f-app-interface) | A program chooses the source address, the TTL, the TOS and the IP options of the datagrams it sends, and reads what a received datagram carried. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [UDP-F-DELIVERY](#udp-f-delivery) | mandatory | RFC 768 Fields, User Interface, Protocol Number, RFC 1122 §4.1.3.6 | RFC768-HDR-2, RFC768-PROTO-1, RFC768-UI-1, RFC1122-UADDR-2 |
| [UDP-F-HEADER](#udp-f-header) | mandatory | RFC 768 Format, Fields | RFC768-HDR-3 |
| [UDP-F-CHECKSUM](#udp-f-checksum) | mandatory | RFC 768 Fields, RFC 1122 §4.1.3.4 | RFC1122-UCK-1, RFC1122-UCK-3, RFC768-CKSUM-2 |
| [UDP-F-PORT-UNREACHABLE](#udp-f-port-unreachable) | optional | RFC 792, RFC 1122 §4.1.3.1 | RFC1122-UPORT-1, RFC792-DU-3 |
| [UDP-F-INPUT-VALIDATION](#udp-f-input-validation) | mandatory | RFC 1122 §4.1.3.4, §4.1.3.6 | RFC1122-UCK-4, RFC1122-UADDR-1 |
| [UDP-F-ERROR-DELIVERY](#udp-f-error-delivery) | mandatory | RFC 1122 §4.1.3.3 | RFC1122-UERR-1 |
| [UDP-F-APP-INTERFACE](#udp-f-app-interface) | mandatory | RFC 1122 §4.1.3.2, §4.1.3.5, §4.1.4 | RFC1122-UAPI-1, RFC1122-UMH-1, RFC1122-UMH-2, RFC1122-UOPT-1, RFC1122-UOPT-2 |

Seven features, five mandatory. UDP is a thin protocol by design — "a minimum of protocol
mechanism", in the words of its introduction — and the map is correspondingly short. Four
of the five mandatory features come from RFC 1122, which entered the in-scope set at
level 3: RFC 768 describes a datagram, and RFC 1122 says what a host must do with one. What
a run showed about each feature is in [`coverage.md`](../../model/udp/coverage.md).

## UDP-F-DELIVERY

**A datagram sent to a port at an address reaches the program that opened that port, with
its data and its source.**

- **Sources** — RFC 768 Fields, `rfc768.txt:66-67`; User Interface, `rfc768.txt:100-108`;
  Protocol Number, `rfc768.txt:144`; IP Interface, `rfc768.txt:127-128`; RFC 1122 §4.1.3.6,
  `rfc1122.txt:4650-4651`.
- **Level** — mandatory (reason: only path). Delivery to a port is the one service UDP
  provides; the document gives no other. No core statement carries a keyword: HDR-2 and
  PROTO-1 are description and UI-1 is a `should`.
- **Description** — the destination port selects the program within the address, the
  datagram travels as IP protocol 17, and the program receives the data with the source
  port and the source address.
- **Checks** — core: RFC768-HDR-2 (the port selects the receiver), RFC768-PROTO-1
  (protocol 17), RFC768-UI-1 (the program receives data and source), RFC1122-UADDR-2 (the
  source address a host sends is one of its own, so that the source the program reads means
  something). Supporting: RFC768-IP-1 (the module reads the addresses and the protocol from
  IP; its visible effects are UI-1 and the checksum), RFC768-HDR-1 (a meaningful source
  port).

## UDP-F-HEADER

**Every datagram carries the 8-octet header, and the length field counts header and data.**

- **Sources** — RFC 768 Format, `rfc768.txt:31-43`; Fields, `rfc768.txt:48-71`.
- **Level** — mandatory (reason: only path). The header format is the protocol.
- **Description** — four 16-bit fields: source port, destination port, length, checksum.
  The length counts the header and the data, so an empty datagram has length 8.
- **Checks** — core: RFC768-HDR-3 (the length rule, including the minimum). Supporting:
  RFC768-HDR-1 (the source port is zero when unused).

## UDP-F-CHECKSUM

**A host protects the datagram with a checksum over a pseudo header, the header and the
data, and checks the value it receives; an all-zero checksum means none was generated.**

- **Sources** — RFC 768 Fields, `rfc768.txt:73-95`; RFC 1122 §4.1.3.4,
  `rfc1122.txt:4584-4593`, which governs.
- **Level** — mandatory (reason: keyword). RFC 768 leaves the choice to the transmitter,
  `rfc768.txt:93-95`, and that is why the level 2 pass called the feature optional. RFC 1122
  entered the in-scope set at level 3 and says "A host MUST implement the facility to
  generate and validate UDP checksums", `rfc1122.txt:4584-4585`, and "MUST default to
  checksumming on", `rfc1122.txt:4586-4587`. Both citations stand, and the later one
  governs; see [`standards.md`](standards.md#override-table).
- **Description** — the checksum covers a pseudo header of addresses, protocol and length,
  which protects against misrouted datagrams. A computed zero is sent as all ones so that
  zero can mean "none". A host generates the value without being asked, and it checks the
  value it receives.
- **Checks** — core: RFC1122-UCK-1 (a host generates and checks), RFC1122-UCK-3 (on by
  default), RFC768-CKSUM-2 (nonzero when generated, zero when not, both accepted).
  Supporting: RFC768-CKSUM-1 (what the value covers), RFC1122-UCK-6 (a computed zero goes
  out as all ones), RFC1122-UCK-2 and RFC1122-UCK-5 (the two permissions for a program).
  The discard that follows a wrong value is a check of
  [UDP-F-INPUT-VALIDATION](#udp-f-input-validation).

## UDP-F-PORT-UNREACHABLE

**A datagram to a port that no program opened may be answered with an ICMP destination
unreachable report, code 3.**

- **Sources** — RFC 792, the destination unreachable message; see
  [RFC792-DU-3](../../standard/rfc792/catalog.md#rfc792-du-3). RFC 1122 §4.1.3.1,
  `rfc1122.txt:4527-4529`, which governs.
- **Level** — optional (reason: keyword, `should`). RFC 792 says the host **may** send the
  message. RFC 1122 §4.1.3.1 says UDP **should** send it. The rule of the guide groups
  `may` and `should` under `optional`, and the catalogs keep the two apart, so the rise in
  strength is not lost.
- **Description** — the destination host should tell the source that nothing listens on the
  port. Silence still conforms, but a host that follows RFC 1122 needs a reason for it.
- **Checks** — core: RFC1122-UPORT-1, RFC792-DU-3. The complementary half — that the
  datagram reaches no program — is RFC768-HDR-2 and belongs to
  [UDP-F-DELIVERY](#udp-f-delivery).

## UDP-F-INPUT-VALIDATION

**A datagram whose checksum is wrong, or whose source address is not a single host, is
discarded without an answer.**

- **Sources** — RFC 1122 §4.1.3.4, `rfc1122.txt:4589-4590`; §4.1.3.6,
  `rfc1122.txt:4646-4648`.
- **Level** — mandatory (reason: keyword, two `must`).
- **Description** — a receiver drops a datagram it cannot trust, and it stays silent about
  the drop. Two conditions demand this: a checksum that is non-zero and does not match what
  it covers, and a source address that no single host can own, such as a broadcast or a
  multicast address. The silence matters as much as the drop; an answer would tell a
  forger that the address exists.
- **Checks** — core: RFC1122-UCK-4 (the wrong checksum), RFC1122-UADDR-1 (the invalid
  source). Supporting: RFC768-CKSUM-1, which says what the checksum covers, and therefore
  what a change in flight must break: the data, the header, or the pseudo header.

## UDP-F-ERROR-DELIVERY

**An ICMP error message that reports a sent datagram reaches the program that sent it.**

- **Sources** — RFC 1122 §4.1.3.3, `rfc1122.txt:4565-4566`.
- **Level** — mandatory (reason: keyword, `must`).
- **Description** — UDP holds no state, so it cannot act on an ICMP error itself. It hands
  every error it gets from the IP layer to the program, which is the only part that knows
  what the datagram was for. The report arrives at a time of its own, long after the send.
- **Checks** — core: RFC1122-UERR-1.

## UDP-F-APP-INTERFACE

**A program chooses the source address, the TTL, the TOS and the IP options of the
datagrams it sends, and reads what a received datagram carried.**

- **Sources** — RFC 1122 §4.1.3.2, `rfc1122.txt:4533-4538`; §4.1.3.5,
  `rfc1122.txt:4626-4634`; §4.1.4, `rfc1122.txt:4655-4678`.
- **Level** — mandatory (reason: keyword, six `must`).
- **Description** — UDP gives the program the full service of the layer below it. In the
  send direction the program names the source address, the TTL, the TOS and the IP options,
  and UDP passes each value down without a change. In the receive direction the program
  reads the specific destination address and the IP options that arrived.
- **Checks** — core: RFC1122-UAPI-1 (TTL, TOS and options from the program), RFC1122-UMH-1
  (the specific destination address goes up), RFC1122-UMH-2 (the program names the source
  address), RFC1122-UOPT-1 and RFC1122-UOPT-2 (the options in both directions). Supporting:
  RFC1122-UMH-3, RFC1122-UAPI-2 and RFC1122-UAPI-3.

  Most of these statements describe the interface between UDP and a program, which two
  nodes on a link cannot show each other. Two of them have a consequence on the wire and
  are checked there: the TTL and the TOS that the program names appear in the datagram
  (RFC1122-UAPI-1), and so does the source address it names (RFC1122-UMH-2).

## Coverage of the catalogs

| Catalog area | Feature |
| --- | --- |
| RFC 768, Header | UDP-F-HEADER, UDP-F-DELIVERY |
| RFC 768, Checksum | UDP-F-CHECKSUM |
| RFC 768, Interface | UDP-F-DELIVERY |
| RFC 768, Protocol number | UDP-F-DELIVERY |
| RFC 792, Error signals (port unreachable) | UDP-F-PORT-UNREACHABLE |
| RFC 1122, UDP checksums | UDP-F-CHECKSUM, UDP-F-INPUT-VALIDATION |
| RFC 1122, UDP ports | UDP-F-PORT-UNREACHABLE |
| RFC 1122, UDP and ICMP errors | UDP-F-ERROR-DELIVERY |
| RFC 1122, UDP and IP options | UDP-F-APP-INTERFACE |
| RFC 1122, UDP multihoming | UDP-F-APP-INTERFACE |
| RFC 1122, UDP addresses | UDP-F-INPUT-VALIDATION, UDP-F-DELIVERY |
| RFC 1122, UDP and the application interface | UDP-F-APP-INTERFACE |

All 8 entries of the RFC 768 catalog, the one RFC 792 entry that serves UDP, and all 18
entries of §4.1 in the RFC 1122 catalog appear in the map. One RFC 1122 entry sits in two
places: RFC1122-UADDR-2, the rule that a host sends only its own address, is a check of
[UDP-F-DELIVERY](#udp-f-delivery), because the source address is part of what the receiving
program gets.
