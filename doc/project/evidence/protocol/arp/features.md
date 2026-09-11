# ARP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `ARP-F-*` · **Stands on:** [standards.md](standards.md), [rfc826/catalog.md](../../standard/rfc826/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc5494/catalog.md](../../standard/rfc5494/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/arp/coverage.md), and step 8 compares it with the claims of
  the model in [`conformance.md`](../../model/arp/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/arp/coverage.md). Keeping it out is deliberate: this map states
which capabilities the standards define, so a new run must never force an edit here.

## Index

| ID | Feature |
| --- | --- |
| [ARP-F-RESOLUTION](#arp-f-resolution) | A host that has no hardware address for a neighbour broadcasts a request that names the address it wants. |
| [ARP-F-REPLY](#arp-f-reply) | The host that owns the target protocol address answers with its own hardware address, directly to the requester. |
| [ARP-F-CACHE](#arp-f-cache) | A host keeps a translation table, fills it from the sender fields of the packets it accepts, and flushes an entry that is out of date. |
| [ARP-F-INPUT-VALIDATION](#arp-f-input-validation) | A packet that the receiver cannot use is discarded, and the receiver answers nothing. |
| [ARP-F-PACKET-FORMAT](#arp-f-packet-format) | Every packet carries the nine fields in one order, with the values the number spaces define. |
| [ARP-F-QUEUE](#arp-f-queue) | The datagram that waits for a resolution is saved and sent after the reply. |
| [ARP-F-FLOOD-PREVENTION](#arp-f-flood-prevention) | The rate of requests for one unresolved address is limited. |
| [ARP-F-NO-ERROR-REPORT](#arp-f-no-error-report) | A missing table entry alone produces no Destination Unreachable report. |
| [ARP-F-GENERALIZATION](#arp-f-generalization) | The packet format serves hardware other than the Ethernet and protocols other than IPv4. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [ARP-F-RESOLUTION](#arp-f-resolution) | mandatory | RFC 826 Packet Generation, RFC 1122 §2.3.3 | RFC826-REQ-1, RFC826-REQ-4, RFC826-REQ-6, RFC1122-AUSE-1 |
| [ARP-F-REPLY](#arp-f-reply) | mandatory | RFC 826 Packet Reception | RFC826-RECV-8, RFC826-RECV-9 |
| [ARP-F-CACHE](#arp-f-cache) | mandatory | RFC 826 Packet Reception, Related issue, RFC 1122 §2.3.2.1 | RFC826-REQ-2, RFC826-RECV-4, RFC826-RECV-6, RFC826-RECV-7, RFC826-TABLE-1, RFC1122-ACACHE-1 |
| [ARP-F-INPUT-VALIDATION](#arp-f-input-validation) | mandatory | RFC 826 Packet Reception | RFC826-RECV-1, RFC826-RECV-2, RFC826-RECV-3, RFC826-RECV-5, RFC826-RECV-10 |
| [ARP-F-PACKET-FORMAT](#arp-f-packet-format) | mandatory | RFC 826 Packet format, Definitions, RFC 5494 §2, §3 | RFC826-FMT-1, RFC826-FMT-2, RFC826-FMT-4, RFC5494-NUM-1, RFC5494-NUM-4 |
| [ARP-F-QUEUE](#arp-f-queue) | optional | RFC 1122 §2.3.2.2 | RFC1122-AQUEUE-1 |
| [ARP-F-FLOOD-PREVENTION](#arp-f-flood-prevention) | mandatory | RFC 1122 §2.3.2.1 | RFC1122-AFLOOD-1 |
| [ARP-F-NO-ERROR-REPORT](#arp-f-no-error-report) | mandatory | RFC 1122 §2.4 | RFC1122-ANOERR-1 |
| [ARP-F-GENERALIZATION](#arp-f-generalization) | optional | RFC 826 Generalization, RFC 5494 §2, §3 | RFC826-GEN-1 |

Nine features, seven mandatory. Four of the nine come wholly or partly from RFC 1122:
RFC 826 describes a mechanism and leaves three questions open — how an entry ages, how often
a host may ask, and what happens to the datagram that waits — and RFC 1122 answers all three
with a keyword. The one MUST NOT of the set is also RFC 1122's. What a run showed about each
feature is in [`coverage.md`](../../model/arp/coverage.md).

## ARP-F-RESOLUTION

**A host that has no hardware address for a neighbour broadcasts a request that names the
address it wants.**

- **Sources** — RFC 826 Packet Generation, `rfc826.txt:161-190`; Definitions,
  `rfc826.txt:115-126`; RFC 1122 §2.3.3, `rfc1122.txt:1429-1431`, which names the protocol
  and the two link types.
- **Level** — mandatory (reason: keyword). RFC 826 says the sending layer "must consult the
  Address Resolution module", `rfc826.txt:166-169`, and RFC 1122 says the translation "MUST
  be managed by the Address Resolution Protocol (ARP)", `rfc1122.txt:1430-1431`.
- **Description** — routing gives the protocol address of the next hop and the hardware it
  sits on. The module looks the pair up in its table. On a miss it builds a packet that
  carries its own two addresses in the sender fields and the address it wants in the target
  protocol address, sets the opcode to request, and broadcasts the packet on that hardware.
- **Checks** — core: RFC826-REQ-1 (the sender asks the module), RFC826-REQ-4 (the field
  values of the request), RFC826-REQ-6 (broadcast, on the hardware the routing chose),
  RFC1122-AUSE-1 (ARP is the protocol that does this on an Ethernet link). Supporting:
  RFC826-FMT-3 (the frame carries it as ARP), RFC826-FMT-5 (opcode 1), RFC826-REQ-5 (the
  target hardware address of a request means nothing).

## ARP-F-REPLY

**The host that owns the target protocol address answers with its own hardware address,
directly to the requester.**

- **Sources** — RFC 826 Packet Reception, `rfc826.txt:214-225`; An Example,
  `rfc826.txt:393-401`.
- **Level** — mandatory (reason: only path). The answer is the one way the requester can
  learn the address; RFC 826 offers no other. No core statement carries a keyword, because
  the whole reception algorithm is written as description.
- **Description** — the receiver that is the target swaps the hardware and the protocol
  fields, puts its own two addresses in the sender fields, sets the opcode to reply, and
  sends the packet to the hardware address that stood in the sender field of the request.
  The reply is not broadcast, and it leaves on the hardware that received the request.
- **Checks** — core: RFC826-RECV-8 (the swap and the opcode), RFC826-RECV-9 (direct, same
  hardware). Supporting: RFC826-FMT-6 (a reply is as long as a request), RFC826-FMT-5
  (opcode 2).

## ARP-F-CACHE

**A host keeps a translation table, fills it from the sender fields of the packets it
accepts, and flushes an entry that is out of date.**

- **Sources** — RFC 826 Packet Generation, `rfc826.txt:169-172`; Packet Reception,
  `rfc826.txt:209-234`; Related issue, `rfc826.txt:412-466`; RFC 1122 §2.3.2.1,
  `rfc1122.txt:1286-1289`, which governs the aging.
- **Level** — mandatory (reason: keyword, and only path). The table is the only thing the
  exchange produces: without it every datagram would need its own exchange. RFC 1122 adds
  the keyword — "MUST provide a mechanism to flush out-of-date cache entries",
  `rfc1122.txt:1287-1288`.
- **Description** — the table maps a pair of a protocol type and a protocol address onto a
  hardware address. A packet that the receiver accepts fills it in two ways: a sender
  protocol address the table already holds gets its hardware address updated, and a sender
  the table does not hold is added when the receiver is the target. Both happen before the
  opcode is read, so a reply teaches the receiver as much as a request does. A newer
  hardware address always wins. RFC 826 leaves the aging of an entry outside its own scope,
  and RFC 1122 requires a flush mechanism with a configurable timeout.
- **Checks** — core: RFC826-REQ-2 (a hit needs no request), RFC826-RECV-4 (an update for a
  known sender), RFC826-RECV-6 (an addition when the receiver is the target),
  RFC826-RECV-7 (before the opcode), RFC826-TABLE-1 (the newer address supersedes),
  RFC1122-ACACHE-1 (the flush). Supporting: RFC1122-ACACHE-2 (the timeout is
  configurable), RFC826-TABLE-2 (a host sends information about itself only, which is what
  makes the table trustworthy). RFC826-TABLE-3 is the statement that RFC 1122 overrides; it
  is not core to anything, and it stays in the catalog with its identifier.

## ARP-F-INPUT-VALIDATION

**A packet that the receiver cannot use is discarded, and the receiver answers nothing.**

- **Sources** — RFC 826 Packet Reception, `rfc826.txt:197-225`; An Example,
  `rfc826.txt:404-406`.
- **Level** — mandatory (reason: only path). The reception algorithm gives a packet two
  outcomes and no third: a reply, or an end of processing with the packet discarded. The
  document states the second one in one sentence, `rfc826.txt:200-201`.
- **Description** — four questions can end the processing. The receiver may not have the
  hardware space of the packet. It may not speak the protocol. It may not be the target
  protocol address, which is the common case, because every station on the link receives a
  broadcast request. And the opcode may not be a request, which is the case of a reply and
  of every other opcode value. In each case the packet is discarded and no packet goes out.
  The discard is silent: ARP has no error message of any kind.
- **Checks** — core: RFC826-RECV-1 (the rule for every negative branch), RFC826-RECV-2
  (the hardware space), RFC826-RECV-3 (the protocol), RFC826-RECV-5 (the target protocol
  address), RFC826-RECV-10 (a reply provokes nothing). Supporting: RFC5494-NUM-2 and
  RFC5494-NUM-3, which give a crafted packet a defined hardware space value and a defined
  opcode value that no host handles; RFC826-RECV-11 (the optional length check).

## ARP-F-PACKET-FORMAT

**Every packet carries the nine fields in one order, with the values the number spaces
define.**

- **Sources** — RFC 826 Packet format, `rfc826.txt:131-155`; Definitions,
  `rfc826.txt:115-126`; Why is it done this way, `rfc826.txt:260-323`; RFC 5494 §2 and §3,
  `rfc5494.txt:119-183`.
- **Level** — mandatory (reason: only path). The format is the protocol.
- **Description** — two 16-bit spaces, two 8-bit lengths, a 16-bit opcode, and four address
  fields whose sizes the two length fields give. Nothing pads the addresses, and the three
  16-bit fields go most significant byte first. On an Ethernet the hardware space is 1 and
  the hardware length is 6. RFC 5494 closes both ends of the hardware space and the opcode
  range as reserved, and it pins the protocol space to the Ethertype numbers.
- **Checks** — core: RFC826-FMT-1 (the field order), RFC826-FMT-2 (1 and 6 on an
  Ethernet), RFC826-FMT-4 (no padding, most significant byte first), RFC5494-NUM-1 (0 and
  65535 are reserved in both spaces), RFC5494-NUM-4 (the protocol space is the Ethertype
  space). Supporting: RFC826-FMT-6 (a reply is as long as a request), RFC826-FMT-7 (one
  resolution per packet), RFC826-RECV-11 (the lengths may be checked against the addresses).

## ARP-F-QUEUE

**The datagram that waits for a resolution is saved and sent after the reply.**

- **Sources** — RFC 1122 §2.3.2.2, `rfc1122.txt:1375-1378`, which governs; RFC 826 Packet
  Generation, `rfc826.txt:172-176`.
- **Level** — optional (reason: keyword, `should`). RFC 826 throws the datagram away, and it
  says so with the word "probably", `rfc826.txt:173`. RFC 1122 says the link layer SHOULD
  save at least the latest one instead. The rule of the guide groups `should` under
  `optional`, and the catalogs keep `should` apart from `may`, so the rise in strength is
  not lost.
- **Description** — without the queue the first datagram of every exchange is lost, and a
  higher layer has to send it again. The requirement asks for one packet per unresolved
  address, the latest one, and for that packet to go out when the reply arrives.
- **Checks** — core: RFC1122-AQUEUE-1. Supporting: RFC826-REQ-3, the statement it
  overrides. The two predict different outcomes for the same datagram, which is what makes
  one observation enough.

## ARP-F-FLOOD-PREVENTION

**The rate of requests for one unresolved address is limited.**

- **Sources** — RFC 1122 §2.3.2.1, `rfc1122.txt:1291-1305`.
- **Level** — mandatory (reason: keyword). "A mechanism to prevent ARP flooding ... MUST be
  included", `rfc1122.txt:1291-1293`.
- **Description** — an address that nobody owns produces no reply, so the question can be
  asked again forever. A host that asks as fast as its datagrams arrive floods the link with
  broadcasts, which every station must receive. The requirement demands a mechanism and
  recommends a maximum of one request per second per destination. The rate is a
  recommendation; the mechanism is not.
- **Checks** — core: RFC1122-AFLOOD-1. The check counts requests over a known time, so it
  needs the recommended rate as its bound; the notes of the check say what a host that picks
  another bound would look like.

## ARP-F-NO-ERROR-REPORT

**A missing table entry alone produces no Destination Unreachable report.**

- **Sources** — RFC 1122 §2.4, `rfc1122.txt:1493-1494`.
- **Level** — mandatory (reason: keyword, `must not`).
- **Description** — a resolution that gets no answer looks like an unreachable destination,
  and it is not one: the address may be a host that is starting, or a host on a link that is
  slow to answer. The link layer therefore keeps the failure to itself. The datagram is
  lost, and nothing tells the sending program why.
- **Checks** — core: RFC1122-ANOERR-1. The check is an absence, and the absence is the
  whole statement.

## ARP-F-GENERALIZATION

**The packet format serves hardware other than the Ethernet and protocols other than IPv4.**

- **Sources** — RFC 826 Generalization, `rfc826.txt:236-242`; Notes, `rfc826.txt:55-59`;
  RFC 5494 §2 and §3, `rfc5494.txt:119-183`.
- **Level** — optional (reason: keyword, `should`). The one core statement says the protocol
  space field "should be associated with the protocol whose address resolution is being
  sought", `rfc826.txt:240-242`. Nothing demands that a host support a second hardware type.
- **Description** — the hardware space and the two length fields exist so that the same
  packet can carry a hardware address of another width. The document was written for the
  10 Mbit Ethernet and generalized afterwards, and it says so in its own notes. RFC 5494
  allocates the numbers that a second hardware type or a second protocol would need.
- **Checks** — core: RFC826-GEN-1. Supporting: RFC826-FMT-2 (the Ethernet values, which are
  the one case an Ethernet link shows), RFC5494-NUM-2 (the experimental hardware space
  values), RFC5494-NUM-4 (the protocol space).

  The standards map files the other hardware types at level 5, so this feature has one
  check and it is about the field and not about a second link type.

## Coverage of the catalogs

| Catalog area | Feature |
| --- | --- |
| RFC 826, Packet format | ARP-F-PACKET-FORMAT, ARP-F-RESOLUTION |
| RFC 826, Request generation | ARP-F-RESOLUTION, ARP-F-CACHE, ARP-F-QUEUE |
| RFC 826, Packet reception | ARP-F-REPLY, ARP-F-CACHE, ARP-F-INPUT-VALIDATION |
| RFC 826, The translation table | ARP-F-CACHE |
| RFC 826, Generalization | ARP-F-GENERALIZATION |
| RFC 1122, ARP cache | ARP-F-CACHE, ARP-F-FLOOD-PREVENTION |
| RFC 1122, ARP packet queue | ARP-F-QUEUE |
| RFC 1122, ARP use | ARP-F-RESOLUTION |
| RFC 1122, Link and internet layer interface | ARP-F-NO-ERROR-REPORT |
| RFC 5494, The number spaces | ARP-F-PACKET-FORMAT, ARP-F-INPUT-VALIDATION, ARP-F-GENERALIZATION |

All 28 entries of the RFC 826 catalog and all six ARP entries of the RFC 1122 catalog appear
in the map. Of the five RFC 5494 entries, four appear; the fifth, RFC5494-PROC-1, binds IANA
and not a host, and no feature of a protocol can hold it. Its area is covered, so the loop
closes: every area of every in-scope catalog is in at least one feature.

Three entries sit in two places. RFC826-FMT-2, the Ethernet values of the hardware fields,
is a check of [ARP-F-PACKET-FORMAT](#arp-f-packet-format) and of
[ARP-F-GENERALIZATION](#arp-f-generalization), because it is the one instance of the general
rule that an Ethernet link can show. RFC826-RECV-11, the optional length check, supports the
format and the validation. And RFC826-FMT-5, the two opcode values, serves the request and
the reply.
