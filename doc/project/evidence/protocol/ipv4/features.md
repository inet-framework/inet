# IPv4 — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `IPV4-F-*` · **Stands on:** [standards.md](standards.md), [rfc791/catalog.md](../../standard/rfc791/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc6864/catalog.md](../../standard/rfc6864/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. The support of each feature is in
  [`coverage.md`](../../model/ipv4/coverage.md).

The feature list itself comes from the standard texts only. The comparison of the support
values against the standards that the model claims to implement is
[`conformance.md`](../../model/ipv4/conformance.md).

## Index

| ID | Feature |
| --- | --- |
| [IPV4-F-TTL](#ipv4-f-ttl) | A datagram carries a time to live, every hop decreases it, a datagram that reaches zero dies, and the destination accepts a datagram that arrives with TTL 1. |
| [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation) | A module splits a datagram that is larger than the MTU of the next link. |
| [IPV4-F-REASSEMBLY](#ipv4-f-reassembly) | The destination collects the fragments and delivers the original datagram upward, and keeps the fragments of different datagrams apart. |
| [IPV4-F-DONT-FRAGMENT](#ipv4-f-dont-fragment) | A datagram marked don't fragment is discarded instead of split, and no device clears the mark. |
| [IPV4-F-IDENTIFICATION](#ipv4-f-identification) | Each fragmentable datagram gets a unique identification, every fragment keeps it, and the identification of an atomic datagram is ignored. |
| [IPV4-F-HEADER-CHECKSUM](#ipv4-f-header-checksum) | Every module recomputes and verifies the header checksum, and silently discards a bad header. |
| [IPV4-F-MIN-SIZE](#ipv4-f-min-size) | Every module forwards 68 octets whole, and every host reassembles 576 octets. |
| [IPV4-F-ERROR-REPORT](#ipv4-f-error-report) | A node that discards a datagram reports the failure with an ICMP message that quotes the datagram. |
| [IPV4-F-DELIVERY](#ipv4-f-delivery) | A datagram crosses the gateway and reaches the addressed program with its data and source address intact. |
| [IPV4-F-HEADER](#ipv4-f-header) | Every datagram carries a version-4 header whose length fields describe it correctly. |
| [IPV4-F-INPUT-VALIDATION](#ipv4-f-input-validation) | A host silently discards input it must not process: a wrong version, a bad checksum, a foreign destination, an invalid source, an unknown ICMP type. |
| [IPV4-F-ERROR-SUPPRESSION](#ipv4-f-error-suppression) | A host sends no ICMP error about an error message, a broadcast, a non-initial fragment, or a source that is not one host. |
| [IPV4-F-ERROR-DELIVERY](#ipv4-f-error-delivery) | A received ICMP error reaches the transport protocol that the quoted header names. |

The support of each feature — what the run actually showed — is **not** in this document.
It lives in the coverage ledger, [`coverage.md`](../../model/ipv4/coverage.md). Keeping it
out is deliberate: this map states which capabilities the standards define, so a new run
must never force an edit here.

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [IPV4-F-TTL](#ipv4-f-ttl) | mandatory | RFC 791 §3.1, §3.2; RFC 1122 §3.2.1.7 | RFC791-TTL-1, RFC791-TTL-2, RFC1122-TTL-2 |
| [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation) | mandatory | RFC 791 §2.3, §3.2; RFC 1122 §3.3.3 | RFC791-FRAG-1..4 |
| [IPV4-F-REASSEMBLY](#ipv4-f-reassembly) | mandatory | RFC 1122 §3.2.1.4, §3.3.2; RFC 791 §2.3 | RFC1122-REASM-1, RFC791-REASM-1, RFC791-REASM-3 |
| [IPV4-F-DONT-FRAGMENT](#ipv4-f-dont-fragment) | mandatory | RFC 6864 §4.3; RFC 791 §2.3 | RFC6864-ID-6, RFC6864-ID-7 |
| [IPV4-F-IDENTIFICATION](#ipv4-f-identification) | mandatory | RFC 6864 §4; RFC 791 §2.3 | RFC6864-ID-5, RFC6864-ID-3, RFC791-FRAG-3 |
| [IPV4-F-HEADER-CHECKSUM](#ipv4-f-header-checksum) | mandatory | RFC 791 §1.4, §3.1; RFC 1122 §3.2.1.2 | RFC791-CKSUM-1, RFC1122-CKSUM-1 |
| [IPV4-F-MIN-SIZE](#ipv4-f-min-size) | mandatory | RFC 791 §3.1, §3.2; RFC 1122 §3.3.2 | RFC791-FRAG-6, RFC1122-REASM-2 |
| [IPV4-F-ERROR-REPORT](#ipv4-f-error-report) | mandatory for a host, optional for a gateway | RFC 792; RFC 1122 §3.2.2, §3.2.2.1, §3.3.8 | RFC792-DU-4, RFC792-TE-1, RFC1122-ERR-1, RFC1122-DU-1, RFC1122-ICMP-2 |
| [IPV4-F-DELIVERY](#ipv4-f-delivery) | mandatory | RFC 791 §2.2, §2.4, §3.1 | RFC791-FWD-1, RFC791-DLV-1, RFC791-PROTO-1 |
| [IPV4-F-HEADER](#ipv4-f-header) | mandatory | RFC 791 §3.1 | RFC791-HDR-1, RFC791-HDR-2, RFC791-HDR-3 |
| [IPV4-F-INPUT-VALIDATION](#ipv4-f-input-validation) | mandatory | RFC 1122 §3.2.1.1, §3.2.1.2, §3.2.1.3, §3.2.2 | RFC1122-VER-1, RFC1122-CKSUM-1, RFC1122-ADDR-2, RFC1122-ADDR-3, RFC1122-ICMP-1 |
| [IPV4-F-ERROR-SUPPRESSION](#ipv4-f-error-suppression) | mandatory | RFC 1122 §3.2.2 | RFC1122-ICMP-5, RFC1122-ICMP-6, RFC1122-ICMP-7, RFC1122-ICMP-8, RFC1122-ICMP-9 |
| [IPV4-F-ERROR-DELIVERY](#ipv4-f-error-delivery) | mandatory | RFC 1122 §3.2.2, §3.2.2.1, §3.2.2.4, §3.2.2.5 | RFC1122-ICMP-3, RFC1122-DU-2, RFC1122-TE-1, RFC1122-PP-2 |

Thirteen features. What a run showed about each one is in
[`coverage.md`](../../model/ipv4/coverage.md).

Two levels changed when RFC 1122 entered the in-scope set, and the rule of step 4 says
why: `mandatory` when any core statement says `must`. Reassembly was `unstated` under
RFC 791, which describes the procedure without an obligation; RFC 1122 §3.3.2 states the
obligation. The error report was `optional` under RFC 792, which says `may` in every
message; RFC 1122 §3.3.8 says `must` for a host, "wherever practical". The gateway of the
mockup stays under RFC 792, so the feature carries both levels, and the ledger keeps the
gateway checks and the host checks apart.

## IPV4-F-TTL

**A datagram carries a time to live, every hop decreases it, a datagram that reaches zero
dies, and the destination accepts a datagram that arrives with TTL 1.**

- **Sources** — RFC 791 §3.2 Time to Live, `rfc791.txt:1976-1984`; §3.1 field definition,
  `rfc791.txt:1013-1014`; RFC 1122 §3.2.1.7, `rfc1122.txt:1977-1985`, which adds the
  host rules: no TTL of zero is sent, no received datagram is discarded for a TTL below 2,
  and the transport layer can set the value.
- **Level** — mandatory (reason: keyword). "This field must be decreased at each point that
  the internet header is processed" and "If this field contains the value zero, then the
  datagram must be destroyed" (`rfc791.txt:1981-1982`, `rfc791.txt:1013-1014`); "A host
  MUST NOT discard a datagram just because it was received with TTL less than 2"
  (`rfc1122.txt:1980-1981`).
- **Description** — the field bounds the lifetime of a datagram in the network and stops a
  datagram that circulates in a loop. The intent is that a gateway discards an expired
  datagram and the destination host does not.
- **Checks** — core: RFC791-TTL-1 (the decrease per hop), RFC791-TTL-2 (the death at
  zero), RFC1122-TTL-2 (the destination accepts TTL 1). Supporting: RFC791-TTL-3 and
  RFC1122-TTL-3 (the sender sets the field), RFC1122-TTL-1 (no TTL of zero leaves a
  host), RFC1122-TTL-4 (the default is configurable).

## IPV4-F-FRAGMENTATION

**A module that must forward a datagram larger than the MTU of the next link splits it
into fragments that carry the fragment fields of the original datagram.**

- **Sources** — RFC 791 §2.3 Fragmentation, `rfc791.txt:662-731`; §3.2,
  `rfc791.txt:1656-1671`; RFC 1122 §3.3.3, `rfc1122.txt:3385-3431`, for the sender-side
  rules (local fragmentation is optional; without it the sender respects MMS_S; off-net
  datagrams should stay within 576 octets; the MTU is configurable).
- **Level** — mandatory (reason: keyword). "If an internet datagram is fragmented, its data portion must be
  broken on 8 octet boundaries" (`rfc791.txt:1658-1659`).
- **Description** — the split preserves the identification, marks every fragment but the
  last with the more-fragments flag, and places each piece with an offset in 8-octet
  units.
- **Checks** — core: RFC791-FRAG-1 (zero fragmentation information when whole),
  RFC791-FRAG-2 (the 8-octet boundary), RFC791-FRAG-3 (the identification is copied),
  RFC791-FRAG-4 (the offset and flag pattern of the first and last fragment). Supporting:
  RFC1122-FRAG-1 (local fragmentation, a may), RFC1122-FRAG-3 (no datagram above MMS_S
  without it), RFC1122-FRAG-4 (at most 576 octets off-net, a should), RFC1122-FRAG-2 and
  RFC1122-FRAG-5 (the interface and the configuration).

## IPV4-F-REASSEMBLY

**The destination collects the fragments of one datagram, delivers the original datagram
upward, and keeps the fragments of different datagrams apart.**

- **Sources** — RFC 1122 §3.2.1.4, `rfc1122.txt:1872-1873`, and §3.3.2,
  `rfc1122.txt:3281` (the obligation; governs); RFC 791 §2.3, `rfc791.txt:674-676` and
  `rfc791.txt:723-729` (the procedure and the four-field key).
- **Level** — mandatory (reason: keyword). "The IP layer MUST implement reassembly of IP
  datagrams" (`rfc1122.txt:3281`). Under RFC 791 alone the level was `unstated`; the
  change came with RFC 1122, by the override table of [`standards.md`](standards.md#override-table).
- **Description** — the four fields identification, source, destination, and protocol
  select the fragments that belong together; the offset places each one. Fragments of two
  datagrams that share three of the four fields, or that arrive interleaved, do not mix.
- **Checks** — core: RFC1122-REASM-1 (the obligation), RFC791-REASM-1 (the procedure
  delivers the original), RFC791-REASM-3 (fragments of different datagrams do not mix).
  Supporting: RFC1122-REASM-4 and RFC1122-REASM-5 (the timeout and its report; level 4),
  RFC1122-REASM-3 (the MMS_R interface).

## IPV4-F-DONT-FRAGMENT

**A datagram marked don't fragment is never fragmented; a module that would have to
fragment it discards it instead; and no device on the path clears the mark.**

- **Sources** — RFC 6864 §4.3, `rfc6864.txt:458-460` (governs; keywords); RFC 791 §2.3,
  `rfc791.txt:662-666` (the base text).
- **Level** — mandatory (reason: keyword). "IPv4 datagrams whose DF=1 MUST NOT be
  fragmented" and "IPv4 datagram transit devices MUST NOT clear the DF bit"
  (`rfc6864.txt:458-460`). Under RFC 791 alone the reason was `only path`.
- **Description** — the flag lets a sender forbid the split, at the price of the datagram
  when the path cannot carry it whole.
- **Checks** — core: RFC6864-ID-6 (no fragmentation; RFC791-FRAG-5 is the overridden
  base statement, and one check establishes both), RFC6864-ID-7 (a gateway forwards the
  bit as it was). The error report that follows the discard belongs to
  [IPV4-F-ERROR-REPORT](#ipv4-f-error-report), not to this feature.

## IPV4-F-IDENTIFICATION

**The sender gives each fragmentable datagram an identification that is unique for its
source, destination, and protocol within one maximum datagram lifetime; every fragment
keeps it; and every device ignores the identification of a datagram that cannot be
fragmented.**

- **Sources** — RFC 6864 §4.1 and §4.3, `rfc6864.txt:354-376` and `rfc6864.txt:438-440`
  (governs); RFC 791 §2.3, `rfc791.txt:683-694` (the base text; RFC791-ID-1 is
  overridden); RFC 1122 §3.2.1.5, `rfc1122.txt:1878-1880` (overridden by RFC 6864 §4.2).
- **Level** — mandatory (reason: keyword). "Sources emitting non-atomic datagrams MUST NOT
  repeat IPv4 ID values within one MDL for a given source address/destination
  address/protocol tuple" (`rfc6864.txt:438-440`); "All devices that examine IPv4 headers
  MUST ignore the IPv4 ID field of atomic datagrams" (`rfc6864.txt:375-376`).
- **Description** — the field is what makes reassembly possible; without uniqueness the
  destination mixes the fragments of two datagrams. For a datagram that cannot be
  fragmented the field has no meaning, and a receiver that reads it anyway breaks
  delivery from sources that reuse it.
- **Checks** — core: RFC6864-ID-5 (uniqueness across non-atomic datagrams), RFC6864-ID-3
  (the field of an atomic datagram is ignored), RFC791-FRAG-3 (the value survives the
  split; core here and in [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation)). Supporting:
  RFC6864-ID-2 (any value on an atomic datagram, a may), RFC6864-ID-4 (no reuse on a
  retransmitted copy), RFC6864-ID-1 (no other use of the field; internal).

## IPV4-F-HEADER-CHECKSUM

**The header carries a checksum, every module that processes the header recomputes and
verifies it, and a header that fails is silently discarded.**

- **Sources** — RFC 791 §3.1, `rfc791.txt:1031-1033`; §1.4, `rfc791.txt:365-366`;
  RFC 1122 §3.2.1.2, `rfc1122.txt:1691-1693` (governs the verification half).
- **Level** — mandatory (reason: keyword). "A host MUST verify the IP header checksum on
  every received datagram and silently discard every datagram that has a bad checksum"
  (`rfc1122.txt:1691-1693`). Under RFC 791 alone the reason was `only path`.
- **Description** — the checksum protects the header only, and it must change whenever a
  field of the header changes, for example the time to live.
- **Checks** — core: RFC791-CKSUM-1 (the recompute per hop), RFC1122-CKSUM-1 (the silent
  discard of a bad header; RFC791-CKSUM-2 is the overridden base statement, and one check
  establishes both).

## IPV4-F-MIN-SIZE

**Every module forwards a 68-octet datagram without fragmentation, and every host
reassembles a 576-octet datagram.**

- **Sources** — RFC 791 §3.2, `rfc791.txt:1669-1671` (the 68-octet floor); RFC 1122
  §3.3.2, `rfc1122.txt:3283-3288` (EMTU_R at least 576; governs RFC 791 §3.1,
  `rfc791.txt:961-963`).
- **Level** — mandatory (reason: keyword). Both sentences use "must".
- **Description** — the two floors are what let a sender assume a minimum service from an
  unknown path.
- **Checks** — core: RFC791-FRAG-6 (68 octets pass without a split), RFC1122-REASM-2 (576
  octets arrive, whole or in fragments; RFC791-REASM-2 is the overridden base statement).

## IPV4-F-ERROR-REPORT

**A node that discards a datagram reports the failure to the source with an ICMP message
that quotes the discarded datagram.**

- **Sources** — RFC 792, `rfc792.txt:261-264` and the code list at `rfc792.txt:215`
  (destination unreachable, code 4); `rfc792.txt:344-356` (time exceeded); RFC 1122
  §3.3.8, `rfc1122.txt:4021-4023` (a host must report, wherever practical); §3.2.2.1,
  `rfc1122.txt:2322-2331` (a host should report a closed port); §3.2.2,
  `rfc1122.txt:2225-2228` and `rfc1122.txt:2236-2237` (the content of a report).
- **Level** — mandatory for a host (reason: keyword, "Wherever practical, hosts MUST
  return ICMP error datagrams on detection of an error", `rfc1122.txt:4021-4023`);
  optional for a gateway (reason: keyword, `may`, in both RFC 792 messages). RFC 1122
  speaks for hosts, and RFC 1812 is out of scope, so the two levels stand side by side.
- **Description** — the report turns a silent drop into a signal that the source can act
  on. The message quotes the internet header and the first 8 data octets of the datagram
  it reports, unchanged, so that the source can match it to the datagram.
- **Checks** — core: RFC792-DU-4 and RFC792-TE-1 (the gateway reports), RFC1122-ERR-1
  and RFC1122-DU-1 (the host reports a closed port), RFC1122-ICMP-2 (the quote is intact).
  Supporting: RFC1122-ICMP-4 (TOS zero on a report, a should), RFC1122-PP-1 (a parameter
  problem report, a should).
- **Note on the strength** — the gateway's silence is not a defect; a host's silence is,
  unless the host can show that a report was not practical.
  [`conformance.md`](../../model/ipv4/conformance.md) reads the support value together
  with the level that applies to the node the check observed.

## IPV4-F-DELIVERY

**A datagram sent to a host in another network crosses the gateway and reaches the
addressed program, with its data and its source address intact.**

- **Sources** — RFC 791 §2.2, `rfc791.txt:542-561`; §2.4, `rfc791.txt:735-736`; §3.1
  protocol field, `rfc791.txt:1023-1026`.
- **Level** — mandatory (reason: only path). The document names no other way for data to
  reach a host in another network; the model of operation in §2.2 is the protocol.
- **Description** — the source addresses the datagram, a gateway forwards it by the
  destination address, and the destination hands the data to the protocol that the header
  names. This is the feature the other twelve exist to serve.
- **Checks** — core: RFC791-FWD-1 (the gateway forwards, addresses intact), RFC791-DLV-1
  (the addressed program receives the data), RFC791-PROTO-1 (the data goes to the named
  protocol). Supporting: RFC1122-ADDR-1 (the source address is the sender's own).

## IPV4-F-HEADER

**Every datagram carries a version-4 header whose length fields describe it correctly.**

- **Sources** — RFC 791 §3.1, `rfc791.txt:858-867` and `rfc791.txt:956-959`.
- **Level** — mandatory (reason: only path). The header format is the protocol: a datagram
  in another format is not an IPv4 datagram, and every other feature reads its fields from
  this header.
- **Description** — the version is 4, the header length counts 32-bit words and is at
  least 5, and the total length counts header and data in octets.
- **Checks** — core: RFC791-HDR-1, RFC791-HDR-2, RFC791-HDR-3. Supporting: none; the
  options, which change the header length, are out of scope. The receiver's side of the
  version field is RFC1122-VER-1, in [IPV4-F-INPUT-VALIDATION](#ipv4-f-input-validation).

## IPV4-F-INPUT-VALIDATION

**A host silently discards input that it must not process: a datagram with a version
other than 4, a bad header checksum, a destination that is not the host, or an invalid
source address, and an ICMP message of unknown type.**

- **Sources** — RFC 1122 §3.2.1.1, `rfc1122.txt:1686-1687`; §3.2.1.2,
  `rfc1122.txt:1691-1693`; §3.2.1.3, `rfc1122.txt:1809-1819` and `rfc1122.txt:1843-1846`;
  §3.2.2, `rfc1122.txt:2222-2223`.
- **Level** — mandatory (reason: keyword). Every source sentence says "MUST be silently
  discarded" or "MUST silently discard".
- **Description** — "silently" is half of every rule: the host delivers nothing upward
  and sends nothing back. A host that answers a malformed datagram with an error report
  amplifies the fault and, for a forged source, redirects it.
- **Checks** — core: RFC1122-VER-1 (version), RFC1122-CKSUM-1 (checksum; core here and in
  [IPV4-F-HEADER-CHECKSUM](#ipv4-f-header-checksum)), RFC1122-ADDR-2 (a foreign
  destination), RFC1122-ADDR-3 (an invalid source), RFC1122-ICMP-1 (an unknown ICMP
  type). Supporting: RFC1122-ADDR-4 (the sender never emits the invalid source forms).

## IPV4-F-ERROR-SUPPRESSION

**A host sends no ICMP error message about an ICMP error message, a datagram to an IP
broadcast or multicast address, a datagram that arrived as a link-layer broadcast, a
non-initial fragment, or a datagram whose source address is not one host.**

- **Sources** — RFC 1122 §3.2.2, `rfc1122.txt:2249-2267`.
- **Level** — mandatory (reason: keyword). "An ICMP error message MUST NOT be sent as the
  result of receiving:" the five cases, and "THESE RESTRICTIONS TAKE PRECEDENCE OVER ANY
  REQUIREMENT ELSEWHERE IN THIS DOCUMENT FOR SENDING ICMP ERROR MESSAGES".
- **Description** — the five rules prevent broadcast storms and error loops. Each one
  needs a scenario of its own, because each fault arrives in a different form.
- **Checks** — core: RFC1122-ICMP-5 (an error about an error), RFC1122-ICMP-6 (a broadcast
  or multicast destination), RFC1122-ICMP-7 (a link-layer broadcast), RFC1122-ICMP-8 (a
  non-initial fragment), RFC1122-ICMP-9 (a source that is not one host).

## IPV4-F-ERROR-DELIVERY

**A received ICMP error message reaches the transport protocol named by the protocol field
of the quoted header.**

- **Sources** — RFC 1122 §3.2.2, `rfc1122.txt:2230-2234`; §3.2.2.1,
  `rfc1122.txt:2333-2334`; §3.2.2.4, `rfc1122.txt:2413-2414`; §3.2.2.5,
  `rfc1122.txt:2442-2444`.
- **Level** — mandatory (reason: keyword). Each source sentence says "MUST be passed" or
  "MUST be reported" to the transport layer.
- **Description** — the report is useless unless it reaches the protocol that sent the
  datagram; the quoted header is what makes the handoff possible.
- **Checks** — core: RFC1122-ICMP-3 (the protocol number selects the transport),
  RFC1122-DU-2, RFC1122-TE-1, RFC1122-PP-2 (each error type is passed up). Supporting:
  RFC1122-DU-3 (codes 0, 1 and 5 are a hint). Every core statement is of class
  `internal`: the handoff happens inside the host, between two layers, and no datagram
  shows it. The map keeps the feature because the standard defines it; the category
  decision belongs to step 9.

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
| RFC 1122, Version | IPV4-F-INPUT-VALIDATION |
| RFC 1122, Header checksum | IPV4-F-HEADER-CHECKSUM, IPV4-F-INPUT-VALIDATION |
| RFC 1122, Addressing | IPV4-F-INPUT-VALIDATION, IPV4-F-DELIVERY |
| RFC 1122, Identification | IPV4-F-IDENTIFICATION (the entry is overridden) |
| RFC 1122, Time to live | IPV4-F-TTL |
| RFC 1122, Reassembly | IPV4-F-REASSEMBLY, IPV4-F-MIN-SIZE |
| RFC 1122, Fragmentation | IPV4-F-FRAGMENTATION |
| RFC 1122, ICMP general rules | IPV4-F-INPUT-VALIDATION, IPV4-F-ERROR-REPORT, IPV4-F-ERROR-SUPPRESSION, IPV4-F-ERROR-DELIVERY |
| RFC 1122, Destination Unreachable, Time Exceeded, Parameter Problem | IPV4-F-ERROR-REPORT, IPV4-F-ERROR-DELIVERY |
| RFC 1122, Error reporting | IPV4-F-ERROR-REPORT |
| RFC 6864, Identification | IPV4-F-IDENTIFICATION, IPV4-F-DONT-FRAGMENT |

All 23 entries of RFC 791 and RFC 792, all 37 entries of RFC 1122, and all 7 entries of
RFC 6864 appear in the map, and no entry appears in none.

Out of scope in the map, because the catalogs put them out of scope: options, type of
service and precedence, the security annex, the ICMP messages other than the error
reports, host routing, broadcast and multicast addressing, and the layer interface of
RFC 1122 §3.4. Each becomes a feature on the day its catalog entries exist.
