# TCP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `TCP-F-*` · **Stands on:** [standards.md](standards.md), [catalog.md](../../standard/rfc9293/catalog.md)

Step 4 artifact of the standards test workflow. The catalog is flat, fine-grained, and per
document. This document is the high-level view above it: the capabilities that the in-scope
set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level of each,
and the cross reference into the catalog.

The in-scope set holds one document, RFC 9293, so every feature draws on that one source.
The map still spans documents by design: RFC 5681 and RFC 6298 will add features here
without a new file when they enter the in-scope set.

The feature list comes from the standard text only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/tcp/coverage.md). Keeping it out is deliberate: this map states
which capabilities the standard defines, so a new run must never force an edit here. The
comparison against the standards that the model claims to implement is
[`conformance.md`](../../model/tcp/conformance.md).

## Index

| ID | Feature |
| --- | --- |
| [TCP-F-ESTABLISH](#tcp-f-establish) | A connection opens with the three-way handshake. |
| [TCP-F-SEQUENCE](#tcp-f-sequence) | Sequence numbers count data octets, and SYN and FIN each occupy one. |
| [TCP-F-ACKNOWLEDGE](#tcp-f-acknowledge) | The receiver acknowledges the next sequence number it expects. |
| [TCP-F-TERMINATE](#tcp-f-terminate) | A connection closes with a FIN in each direction, each acknowledged. |
| [TCP-F-DATA-TRANSFER](#tcp-f-data-transfer) | A byte stream is carried in segments no larger than the effective MSS and delivered in order. |
| [TCP-F-FLOW-CONTROL](#tcp-f-flow-control) | The receiver advertises a window, and the sender sends no new data beyond it. |
| [TCP-F-CHECKSUM](#tcp-f-checksum) | Every segment carries a checksum; the sender generates it and the receiver checks it. |
| [TCP-F-HEADER](#tcp-f-header) | The data offset gives the header length in 32-bit words, at least five. |
| [TCP-F-RESET](#tcp-f-reset) | A segment for a connection that does not exist is answered with a reset. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [TCP-F-ESTABLISH](#tcp-f-establish) | mandatory | RFC 9293 §3.5 | RFC9293-EST-1, RFC9293-EST-2 |
| [TCP-F-SEQUENCE](#tcp-f-sequence) | mandatory | RFC 9293 §3.4, §4 | RFC9293-SEQ-1, RFC9293-FIN-1 |
| [TCP-F-ACKNOWLEDGE](#tcp-f-acknowledge) | mandatory | RFC 9293 §3.1 | RFC9293-ACK-1 |
| [TCP-F-TERMINATE](#tcp-f-terminate) | mandatory | RFC 9293 §3.6 | RFC9293-FIN-2 |
| [TCP-F-DATA-TRANSFER](#tcp-f-data-transfer) | mandatory | RFC 9293 §2.1, §3.7.1, §3.8 | RFC9293-DATA-1, RFC9293-SEG-1 |
| [TCP-F-FLOW-CONTROL](#tcp-f-flow-control) | mandatory | RFC 9293 §3.1, §3.8.6 | RFC9293-WND-1, RFC9293-WND-2 |
| [TCP-F-CHECKSUM](#tcp-f-checksum) | mandatory | RFC 9293 §3.1 | RFC9293-CKSUM-1, RFC9293-CKSUM-2 |
| [TCP-F-HEADER](#tcp-f-header) | mandatory | RFC 9293 §3.1 | RFC9293-HDR-1 |
| [TCP-F-RESET](#tcp-f-reset) | mandatory | RFC 9293 §3.5.2, §3.10.7.1 | RFC9293-RST-1 |

Nine features, all mandatory. What a run showed about them is in
[`coverage.md`](../../model/tcp/coverage.md). Together they describe one connection from
open to close, the stream it carries, the window that paces it, the checksum that guards
it, and the reset that answers a connection that is not there. Congestion control, the
retransmission timer, and the options are other documents' features and later levels.

## TCP-F-ESTABLISH

**A connection opens with the three-way handshake.**

- **Sources** — RFC 9293 §3.5, `rfc9293.txt:1223-1225` and Figure 6,
  `rfc9293.txt:1260-1268`. RFC 9293 governs; RFC 793 is obsolete.
- **Level** — mandatory (reason: only path). The handshake is the only procedure the
  document gives for a normal open, and the state machine admits no other path to
  ESTABLISHED.
- **Description** — the initiator sends a SYN, the responder answers with a SYN that also
  acknowledges, and the initiator acknowledges in turn. Each side learns the other's
  initial sequence number. The SYN may carry the maximum segment size option.
- **Checks** — core: RFC9293-EST-1 (the three segments in order), RFC9293-EST-2 (the
  SYN-ACK acknowledges the initial sequence number plus one). Supporting: RFC9293-ISS-1
  (each side picks its own initial sequence number), RFC9293-OPT-1 (the MSS option in the
  SYN, a `should`).

## TCP-F-SEQUENCE

**Sequence numbers count data octets, and the SYN and the FIN each occupy one.**

- **Sources** — RFC 9293 §3.4, `rfc9293.txt:988-992`; §4 Glossary, `rfc9293.txt:3971-3974`.
- **Level** — mandatory (reason: only path). Every acknowledgment arithmetic in the
  document rests on it.
- **Description** — the SYN counts before the first data octet and the FIN counts after the
  last, so a peer acknowledges each of them with a plus-one. A pure ACK counts for nothing.
- **Checks** — core: RFC9293-SEQ-1 (the SYN occupies one), RFC9293-FIN-1 (the FIN occupies
  one). Supporting: RFC9293-SEQ-2 (a pure ACK occupies none), RFC9293-ISS-2 (the initial
  sequence number generator).
- **Note** — this feature is the reason the other features work. It is checked only through
  them, never on its own, which is the normal shape for an arithmetic rule.

## TCP-F-ACKNOWLEDGE

**The receiver acknowledges the next sequence number it expects.**

- **Sources** — RFC 9293 §3.1, `rfc9293.txt:337-339`; §3.8.6.3, `rfc9293.txt:2326-2329`.
- **Level** — mandatory (reason: only path, and the word "always"). The field definition is
  unconditional once a connection is established: "Once a connection is established, this
  is always sent."
- **Description** — the acknowledgment is cumulative. It names the first octet the receiver
  has not yet accepted, so one acknowledgment covers everything before it. A receiver may
  delay it, but by less than half a second.
- **Checks** — core: RFC9293-ACK-1 (after N octets, the acknowledgment is S plus N, and
  after a whole stream it is the end of the stream). Supporting: RFC9293-ACK-2 (every
  established segment carries the ACK bit), RFC9293-ACKD-1 (the delay bound, a timing
  statement for level 4).

## TCP-F-TERMINATE

**A connection closes with a FIN in each direction, and each FIN is acknowledged.**

- **Sources** — RFC 9293 §3.6 Figure 12, `rfc9293.txt:1590-1606`.
- **Level** — mandatory (reason: only path). The figure is the normal close, and the state
  machine reaches CLOSED through it.
- **Description** — the close is independent per direction. One side may finish sending
  while the other still sends, which is why two FIN segments appear rather than one
  exchange.
- **Checks** — core: RFC9293-FIN-2 (the FIN, its acknowledgment, the peer FIN, and its
  acknowledgment).

## TCP-F-DATA-TRANSFER

**A byte stream is carried in segments no larger than the effective maximum segment size
and delivered in order.**

- **Sources** — RFC 9293 §2.1, `rfc9293.txt:243-244`; §3.7.1, `rfc9293.txt:1745-1747`;
  §3.8, `rfc9293.txt:1891-1892`.
- **Level** — mandatory (reason: keyword; RFC9293-SEG-1 is a MUST, and the ordered stream
  is the service the document exists to define).
- **Description** — the sender cuts the stream into segments of at most the effective send
  MSS, 536 octets by default over IPv4, and the receiver acknowledges the stream
  cumulatively, so that an acknowledgment of its end means every octet arrived in order.
- **Checks** — core: RFC9293-DATA-1 (the whole stream is acknowledged), RFC9293-SEG-1 (a
  segment is no larger than the effective send MSS). Supporting: RFC9293-ACK-2,
  RFC9293-PSH-1 (the last segment of a send carries PSH).
- **Bound** — in the absence of loss. Recovery from loss is the retransmission machinery,
  which needs a dropped segment to show: level 3 for the mechanism, level 4 for its timer.

## TCP-F-FLOW-CONTROL

**The receiver advertises a window, and the sender sends no new data beyond it.**

- **Sources** — RFC 9293 §3.1, `rfc9293.txt:391-395`; §3.8.6, `rfc9293.txt:2114-2122` and
  `rfc9293.txt:2155-2157`.
- **Level** — mandatory (reason: only path). The window is the only mechanism the base
  document gives a receiver to pace a sender; congestion control belongs to RFC 5681.
- **Description** — every segment carries the number of octets its sender is willing to
  accept beyond the acknowledged one. The peer may send new data only up to that edge.
  When the window is smaller than a segment, the window and not the segment size bounds
  what the sender transmits.
- **Checks** — core: RFC9293-WND-1 (the window field and its meaning), RFC9293-WND-2 (the
  sender stays within the advertised window). Supporting: RFC9293-ZWP-1 (zero-window
  probing, a timer matter for level 4).

## TCP-F-CHECKSUM

**Every segment carries a checksum; the sender generates it and the receiver checks it.**

- **Sources** — RFC 9293 §3.1, `rfc9293.txt:458-459`.
- **Level** — mandatory (reason: keyword, MUST-2 and MUST-3). Unlike UDP, TCP never lets
  the checksum be absent.
- **Description** — the checksum covers a pseudo header, the TCP header and the data. The
  sender must generate it; the receiver must check it and discard on failure.
- **Checks** — core: RFC9293-CKSUM-1 (the sender generates it: nonzero on the wire),
  RFC9293-CKSUM-2 (the receiver checks it: a wrong checksum is discarded). The second
  needs a corrupted segment in flight and is level 3; at level 2 the feature stands on the
  first alone, with a `partial` support value until the second runs.

## TCP-F-HEADER

**The data offset gives the header length in 32-bit words, at least five.**

- **Sources** — RFC 9293 §3.1, `rfc9293.txt:341-345`.
- **Level** — mandatory (reason: only path). The header format is the protocol.
- **Description** — the offset points to the data; a header without options is five words,
  and a header with options is still a whole number of words.
- **Checks** — core: RFC9293-HDR-1. Supporting: RFC9293-OPT-1 (an MSS option makes the SYN
  six words).

## TCP-F-RESET

**A segment for a connection that does not exist is answered with a reset.**

- **Sources** — RFC 9293 §3.5.2, `rfc9293.txt:1468-1471`; §3.10.7.1, `rfc9293.txt:3287-3289`.
- **Level** — mandatory (reason: only path). The document gives exactly one response to a
  segment for a closed port, and states it as a rule of the state machine.
- **Description** — a SYN to a port with no listener is answered with a segment that carries
  RST and ACK, sequence number zero, and an acknowledgment of the SYN's sequence number plus
  one. The connection attempt ends.
- **Checks** — core: RFC9293-RST-1.
- **Bound** — the closed-port case only. Reset on a live connection, and the acceptance
  rules for a received reset, are level 3.

## Coverage of the catalog

| Catalog area | Feature |
| --- | --- |
| Connection establishment | TCP-F-ESTABLISH |
| Sequence space | TCP-F-SEQUENCE |
| Acknowledgment | TCP-F-ACKNOWLEDGE, TCP-F-DATA-TRANSFER |
| Connection termination | TCP-F-TERMINATE, TCP-F-SEQUENCE |
| Data transfer and segment size | TCP-F-DATA-TRANSFER |
| Window | TCP-F-FLOW-CONTROL |
| Checksum | TCP-F-CHECKSUM |
| Header | TCP-F-HEADER, TCP-F-ESTABLISH (the MSS option) |
| Reset | TCP-F-RESET |

Every area of the catalog appears in the map.

Out of scope in the map, because the catalog puts them out of scope: retransmission and its
timer, congestion control, the options other than MSS, keep-alives, the simultaneous cases,
and the TIME-WAIT duration. Urgent data is a judgment call recorded here: the document
requires the mechanism (MUST-30 to MUST-32), and discourages its use; no ordinary transfer
exercises it, so it is not a normal-path mechanism for level 2 and waits for level 5. Congestion control and the retransmission
timer need RFC 5681 and RFC 6298 in the in-scope set first; see
[`standards.md`](standards.md#override-table).
