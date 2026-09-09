# QUIC — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `QUIC-F-*` · **Stands on:** [standards.md](standards.md), [catalog.md](../../standard/rfc9000/catalog.md)

Step 4 artifact of the standards test workflow. The catalog is flat, fine-grained, and per
document. This document is the high-level view above it: the capabilities that the in-scope
set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level of each,
and the cross reference into the catalog.

The in-scope set holds one document, RFC 9000, so every feature draws on that one source.
The map still spans documents by design: RFC 9002 will add loss detection and congestion
control here without a new file when it enters the in-scope set.

The feature list comes from the standard text only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/quic/coverage.md). Keeping it out is deliberate: this map
states which capabilities the standard defines, so a new run must never force an edit here.
The comparison against the standards that the model claims to implement is
[`conformance.md`](../../model/quic/conformance.md).

## Index

| ID | Feature |
| --- | --- |
| [QUIC-F-PACKET](#quic-f-packet) | Packets carry a long or a short header, and a packet number that is never reused. |
| [QUIC-F-INITIAL-SIZE](#quic-f-initial-size) | A datagram carrying an Initial packet is padded to at least 1200 bytes. |
| [QUIC-F-STREAMS](#quic-f-streams) | Streams are identified by a structured number and deliver an ordered byte stream. |
| [QUIC-F-FLOW-CONTROL](#quic-f-flow-control) | A receiver advertises limits per stream and per connection, and a sender respects them. |
| [QUIC-F-ACKNOWLEDGE](#quic-f-acknowledge) | An ack-eliciting packet is acknowledged. |
| [QUIC-F-CLOSE](#quic-f-close) | A CONNECTION_CLOSE frame ends the connection at once. |
| [QUIC-F-VERSION-NEGOTIATION](#quic-f-version-negotiation) | A server that does not accept the client's version says so, and no endpoint answers such a packet with another. |
| [QUIC-F-ADDRESS-VALIDATION](#quic-f-address-validation) | Until the client's address is validated, a server sends at most three times what it received. |
| [QUIC-F-FRAME-VALIDATION](#quic-f-frame-validation) | A frame an endpoint cannot understand ends the connection with a stated error, and nothing worse. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [QUIC-F-PACKET](#quic-f-packet) | mandatory | RFC 9000 §17.2, §17.3, §12.3 | RFC9000-PKT-1, PKT-2, PKT-3 |
| [QUIC-F-INITIAL-SIZE](#quic-f-initial-size) | mandatory | RFC 9000 §14.1 | RFC9000-SIZE-1 |
| [QUIC-F-STREAMS](#quic-f-streams) | mandatory | RFC 9000 §2.1, §2.2, §19.8 | RFC9000-STR-1, STR-2, STR-3 |
| [QUIC-F-FLOW-CONTROL](#quic-f-flow-control) | mandatory | RFC 9000 §4.1 | RFC9000-FC-1 |
| [QUIC-F-ACKNOWLEDGE](#quic-f-acknowledge) | mandatory | RFC 9000 §13.2.1 | RFC9000-ACK-1 |
| [QUIC-F-CLOSE](#quic-f-close) | mandatory | RFC 9000 §10.2 | RFC9000-CLOSE-1 |
| [QUIC-F-VERSION-NEGOTIATION](#quic-f-version-negotiation) | mandatory | RFC 9000 §6.1 | RFC9000-VER-1, RFC9000-VER-2 |
| [QUIC-F-ADDRESS-VALIDATION](#quic-f-address-validation) | mandatory | RFC 9000 §8.1 | RFC9000-AMP-1 |
| [QUIC-F-FRAME-VALIDATION](#quic-f-frame-validation) | mandatory | RFC 9000 §12.4 | RFC9000-ERR-1 |

Nine features, all mandatory. The first six describe one QUIC connection from its first
Initial packet to its close: the packets it uses, the size floor that makes the path usable,
the streams it multiplexes, the limits that pace it, the acknowledgments that carry its
reliability, and the frame that ends it. The three the level 3 pass added describe what an
endpoint owes to a packet it did not expect: a version it does not speak, an address it has
not yet trusted, and a frame it cannot read. Loss recovery and congestion control are
RFC 9002's features and level 4; the cryptographic handshake is RFC 9001's and level 5.

## QUIC-F-PACKET

**Packets carry a long or a short header, and a packet number that is never reused.**

- **Sources** — RFC 9000 §17.2, `rfc9000.txt:4949-4960` and `rfc9000.txt:4987-4989`;
  §17.3, `rfc9000.txt:5470-5476`; §12.3, `rfc9000.txt:3827-3828`.
- **Level** — mandatory (reason: only path for the two header forms, which are the wire
  image of the protocol; keyword for the packet number, a MUST NOT).
- **Description** — the packets that set a connection up carry a long header with the
  version and both connection identifiers; the packets that carry application data
  afterwards carry a short header; and within one packet number space an endpoint never
  sends one number twice.
- **Checks** — core: RFC9000-PKT-1 (the long header and its version), RFC9000-PKT-2 (the
  short header after the handshake), RFC9000-PKT-3 (packet numbers not reused).

## QUIC-F-INITIAL-SIZE

**A datagram carrying an Initial packet is padded to at least 1200 bytes.**

- **Sources** — RFC 9000 §14.1, `rfc9000.txt:4642-4650`.
- **Level** — mandatory (reason: keyword, MUST, on both the client and the server).
- **Description** — the client expands every datagram that carries an Initial packet, and
  the server expands every datagram that carries an ack-eliciting Initial packet, to at
  least 1200 octets. The floor proves the path can carry a reasonable maximum transmission
  unit, and it limits the amplification an unverified address can cause.
- **Checks** — core: RFC9000-SIZE-1.
- **Note** — this is the boundary value of QUIC, and the reason a QUIC handshake is never
  small. It is also the one requirement of this map that a model can miss without any
  visible harm in a simulation, because a simulated path has no minimum transmission unit
  problem.

## QUIC-F-STREAMS

**Streams are identified by a structured number and deliver an ordered byte stream.**

- **Sources** — RFC 9000 §2.1, `rfc9000.txt:531-538`; §2.2, `rfc9000.txt:569-572`; §19.8,
  `rfc9000.txt:6215-6217`.
- **Level** — mandatory (reason: keyword; RFC9000-STR-2 is a MUST, and the identifier
  encoding is the only scheme the document gives).
- **Description** — the low bit of a stream identifier says who opened the stream and the
  next bit says whether it is bidirectional; STREAM frames create a stream and carry its
  data; and the receiving endpoint hands the application the octets in the order the
  sender wrote them.
- **Checks** — core: RFC9000-STR-1 (the identifier encoding), RFC9000-STR-2 (ordered
  delivery), RFC9000-STR-3 (STREAM frames carry the data).

## QUIC-F-FLOW-CONTROL

**A receiver advertises limits per stream and per connection, and a sender respects them.**

- **Sources** — RFC 9000 §4.1, `rfc9000.txt:1037-1055`.
- **Level** — mandatory (reason: keyword, "Senders MUST NOT send data in excess of either
  limit").
- **Description** — QUIC paces a sender at two levels at once: a limit for each stream and
  a limit for the connection. The receiver sets the initial limits in its transport
  parameters and raises them later with MAX_STREAM_DATA and MAX_DATA frames.
- **Checks** — core: RFC9000-FC-1.
- **Note** — the two levels are what distinguishes QUIC's flow control from TCP's single
  window. A check that exercises only one level establishes only that level.

## QUIC-F-ACKNOWLEDGE

**An ack-eliciting packet is acknowledged.**

- **Sources** — RFC 9000 §13.2.1, `rfc9000.txt:4094-4096`.
- **Level** — mandatory (reason: keyword, MUST, for an ack-eliciting packet).
- **Description** — a packet that carries a frame other than ACK, PADDING or
  CONNECTION_CLOSE elicits an acknowledgment; the peer sends an ACK frame that covers its
  packet number, within the delay it declared.
- **Checks** — core: RFC9000-ACK-1.
- **Bound** — the timing half, the declared maximum delay, is a statement about a
  distribution and belongs to level 4 with RFC 9002.

## QUIC-F-CLOSE

**A CONNECTION_CLOSE frame ends the connection at once.**

- **Sources** — RFC 9000 §10.2, `rfc9000.txt:3170-3173`.
- **Level** — mandatory (reason: only path for an immediate close; the document's other
  ending, the idle timeout, is a timer and level 4).
- **Description** — the frame closes every stream at once and moves the sender into the
  closing state. No application data follows it.
- **Checks** — core: RFC9000-CLOSE-1.

## QUIC-F-VERSION-NEGOTIATION

**A server that does not accept the client's version says so, and no endpoint answers such a
packet with another.**

- **Sources** — RFC 9000 §6.1, `rfc9000.txt:1644-1648`.
- **Level** — mandatory (reason: only path). A server that speaks no common version has one
  answer the document gives it, and the prohibition beside it has a keyword.
- **Description** — a QUIC packet names its version in the clear, so a server can read it
  before it does anything else. If the version is one the server does not accept, it answers
  with a packet whose version field is zero and which lists the versions it will accept. It
  keeps no state for the attempt. The prohibition stops two endpoints from answering each
  other without end.
- **Checks** — core: RFC9000-VER-1 (the answer), RFC9000-VER-2 (no answer to an answer).

## QUIC-F-ADDRESS-VALIDATION

**Until the client's address is validated, a server sends at most three times what it
received.**

- **Sources** — RFC 9000 §8.1, `rfc9000.txt:2200-2224`.
- **Level** — mandatory (reason: keyword, MUST NOT).
- **Description** — anyone can write someone else's address into a packet. A server that
  answered such a packet freely would send its answer to the victim, and a small forged
  packet would buy a large flood. So a server keeps a budget: three times what it has
  received, until it knows the address is real. Receiving a Handshake packet from the peer
  is what settles that.
- **Checks** — core: RFC9000-AMP-1. The floor of
  [QUIC-F-INITIAL-SIZE](#quic-f-initial-size) is the other side of the same design: it makes
  sure the budget is large enough for a handshake to finish.

## QUIC-F-FRAME-VALIDATION

**A frame an endpoint cannot understand ends the connection with a stated error, and nothing
worse.**

- **Sources** — RFC 9000 §12.4, `rfc9000.txt:3980-3981`.
- **Level** — mandatory (reason: keyword, MUST).
- **Description** — frame types are a registry, and a QUIC version defines the ones it uses.
  A type outside that set means the packet cannot be trusted, so the connection ends. It ends
  in the way the document names: a connection error of type FRAME_ENCODING_ERROR, which the
  peer is told about. Ignoring the frame would leave two endpoints with different ideas of
  the stream; stopping would be worse still.
- **Checks** — core: RFC9000-ERR-1. The frame that carries the news is the one of
  [QUIC-F-CLOSE](#quic-f-close).


## Coverage of the catalog

| Catalog area | Feature |
| --- | --- |
| Packets | QUIC-F-PACKET |
| Initial datagram size | QUIC-F-INITIAL-SIZE |
| Streams | QUIC-F-STREAMS |
| Flow control | QUIC-F-FLOW-CONTROL |
| Acknowledgment | QUIC-F-ACKNOWLEDGE |
| Connection termination | QUIC-F-CLOSE |
| Version negotiation | QUIC-F-VERSION-NEGOTIATION |
| Address validation | QUIC-F-ADDRESS-VALIDATION |
| Frame validation | QUIC-F-FRAME-VALIDATION |

All 14 entries of the catalog appear in the map.

Out of scope in the map, because the catalog still puts them out of scope: the stateless
reset, address validation with Retry and tokens, path validation and migration, stream reset,
the handling of a packet that cannot be decrypted, loss detection and congestion control, the
cryptographic handshake, and the frame types beyond STREAM, ACK, CRYPTO, PADDING and
CONNECTION_CLOSE. The catalog's closing section names the level that reaches each one.
