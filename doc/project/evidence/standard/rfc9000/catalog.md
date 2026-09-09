# RFC 9000 (QUIC transport) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC9000-*` · **Stands on:** [standards.md](../../protocol/quic/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for the one document
of the in-scope set: RFC 9000, the QUIC transport, May 2021. The catalog comes from the RFC
text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc9000.txt` — QUIC: A UDP-Based Multiplexed and Secure Transport, May 2021. Downloaded
  2026-09-08 from <https://www.rfc-editor.org/rfc/rfc9000.txt>.

The document is 8485 lines, and this catalog does not exhaust it. It holds the normal-path
mandatory mechanisms that level 2 requires; the areas it leaves out are named at the end,
each with the level that will reach it. The family of QUIC documents and the reason the
handshake's cryptographic content is out of scope are in
[`standards.md`](../../protocol/quic/standards.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`quic/coverage.md`](../../model/quic/coverage.md). Keeping it out is deliberate: this
catalog states what the standard says, so a new test or a new run must never force an edit
here.

Quotes are verbatim. A reference such as `rfc9000.txt:1050` points to a line of the cached
file in this folder.

## Index

| ID | Statement |
| --- | --- |
| [RFC9000-PKT-1](#rfc9000-pkt-1) | A long header carries the version and both connection identifiers. |
| [RFC9000-PKT-2](#rfc9000-pkt-2) | A 1-RTT packet uses the short header, after the keys are negotiated. |
| [RFC9000-PKT-3](#rfc9000-pkt-3) | A packet number is never reused within a packet number space. |
| [RFC9000-SIZE-1](#rfc9000-size-1) | A datagram carrying an Initial packet is at least 1200 bytes. |
| [RFC9000-STR-1](#rfc9000-str-1) | The two low bits of a stream identifier give its initiator and its directionality. |
| [RFC9000-STR-2](#rfc9000-str-2) | An endpoint delivers stream data to the application as an ordered byte stream. |
| [RFC9000-STR-3](#rfc9000-str-3) | STREAM frames create a stream and carry its data. |
| [RFC9000-FC-1](#rfc9000-fc-1) | A sender sends no data beyond the stream limit or the connection limit. |
| [RFC9000-ACK-1](#rfc9000-ack-1) | An ack-eliciting packet is acknowledged within the declared maximum delay. |
| [RFC9000-CLOSE-1](#rfc9000-close-1) | A CONNECTION_CLOSE frame terminates the connection immediately. |
| [RFC9000-VER-1](#rfc9000-ver-1) | A server that does not accept the client's version answers with a Version Negotiation packet. |
| [RFC9000-VER-2](#rfc9000-ver-2) | No endpoint answers a Version Negotiation packet with another one. |
| [RFC9000-AMP-1](#rfc9000-amp-1) | Before the client's address is validated, a server sends at most three times the bytes it received. |
| [RFC9000-ERR-1](#rfc9000-err-1) | A frame of unknown type is a connection error of type FRAME_ENCODING_ERROR. |

The rows above the line come from the level 2 pass, which read the document for the normal
path of a connection. The rows below come from the level 3 pass, which read it for the
answers a QUIC endpoint owes to a packet it did not expect.

## How to read an entry

The conventions of an entry — strength, class, and the `Overridden by` field — are the ones
of [`rfc791/catalog.md`](../rfc791/catalog.md#how-to-read-an-entry). RFC 9000 uses RFC 2119
keywords throughout, so the strength of a statement is the keyword it carries.

## Packets

### RFC9000-PKT-1

**A long header carries the version and both connection identifiers.**

> ```
> Long Header Packet {
>   Header Form (1) = 1,
>   Fixed Bit (1) = 1,
>   Long Packet Type (2),
>   Type-Specific Bits (4),
>   Version (32),
>   Destination Connection ID Length (8),
>   Destination Connection ID (0..160),
>   Source Connection ID Length (8),
>   Source Connection ID (0..160),
>   Type-Specific Payload (..),
> }
> ```
> — §17.2, `rfc9000.txt:4949-4960`
>
> "Version: The QUIC Version is a 32-bit field that follows the first byte. This field
> indicates the version of QUIC that is in use and determines how the rest of the protocol
> fields are interpreted." — §17.2, `rfc9000.txt:4987-4989`

- Strength: description of the format, which is normative by the document's own structure.
  Class: wire.
- Check idea: the first packets of a connection carry a long header with the version of
  QUIC version 1 and a destination connection identifier that the peer chose.

### RFC9000-PKT-2

**A 1-RTT packet uses the short header, after the version and the keys are negotiated.**

> "This version of QUIC defines a single packet type that uses the short packet header."
> — §17.3, `rfc9000.txt:5470-5471`
>
> "A 1-RTT packet uses a short packet header. It is used after the version and 1-RTT keys
> are negotiated." — §17.3.1, `rfc9000.txt:5475-5476`

- Strength: description. Class: wire.
- Check idea: after the handshake, the packets that carry application data use the short
  header, and no long header follows.

### RFC9000-PKT-3

**A packet number is never reused within a packet number space.**

> "A QUIC endpoint MUST NOT reuse a packet number within the same packet number space in
> one connection." — §12.3, `rfc9000.txt:3827-3828`

- Strength: must. Class: wire.
- Check idea: the packet numbers an endpoint sends in one space are all different, and a
  connection that sends several packets shows them increasing.

### RFC9000-SIZE-1

**A datagram carrying an Initial packet is at least 1200 bytes.**

> "A client MUST expand the payload of all UDP datagrams carrying Initial packets to at
> least the smallest allowed maximum datagram size of 1200 bytes by adding PADDING frames
> to the Initial packet or by coalescing the Initial packet" — §14.1,
> `rfc9000.txt:4642-4646`
>
> "Similarly, a server MUST expand the payload of all UDP datagrams carrying ack-eliciting
> Initial packets to at least the smallest allowed maximum datagram size of 1200 bytes."
> — §14.1, `rfc9000.txt:4648-4650`

- Strength: must, for both endpoints. Class: wire.
- Check idea: the UDP datagram that carries the client's first Initial packet is at least
  1200 octets, however small the packet's own content is. This is the boundary value of
  QUIC, and the reason a QUIC handshake is never small.

## Streams

### RFC9000-STR-1

**The two low bits of a stream identifier give its initiator and its directionality.**

> "The least significant bit (0x01) of the stream ID identifies the initiator of the
> stream. Client-initiated streams have even-numbered stream IDs (with the bit set to 0),
> and server-initiated streams have odd-numbered stream IDs (with the bit set to 1)."
> — §2.1, `rfc9000.txt:531-534`
>
> "The second least significant bit (0x02) of the stream ID distinguishes between
> bidirectional streams (with the bit set to 0) and unidirectional streams (with the bit
> set to 1)." — §2.1, `rfc9000.txt:536-538`

- Strength: description of the encoding, normative. Class: wire.
- Check idea: the first stream a client opens for a two-way exchange has identifier 0, and
  every stream a client initiates has an even identifier.

### RFC9000-STR-2

**An endpoint delivers stream data to the application as an ordered byte stream.**

> "Endpoints MUST be able to deliver stream data to an application as an ordered byte
> stream. Delivering an ordered byte stream requires that an endpoint buffer any data that
> is received out of order, up to the advertised flow control limit." — §2.2,
> `rfc9000.txt:569-572`

- Strength: must. Class: end-to-end.
- Check idea: the receiving application receives exactly the octets the sender wrote, in
  the order it wrote them.

### RFC9000-STR-3

**STREAM frames create a stream and carry its data.**

> "STREAM frames implicitly create a stream and carry stream data. The Type field in the
> STREAM frame takes the form 0b00001XXX (or the set of values from 0x08 to 0x0f)."
> — §19.8, `rfc9000.txt:6215-6217`

- Strength: description. Class: wire.
- Check idea: the packets that carry application data carry STREAM frames, and the stream
  identifier in the frame is the one the application opened.

## Flow control

### RFC9000-FC-1

**A sender sends no data beyond the stream limit or the connection limit.**

> "QUIC employs a limit-based flow control scheme where a receiver advertises the limit of
> total bytes it is prepared to receive on a given stream or for the entire connection."
> — §4.1, `rfc9000.txt:1037-1039`
>
> "Senders MUST NOT send data in excess of either limit." — §4.1, `rfc9000.txt:1050`

- Strength: must. Class: wire.
- Check idea: with a small initial stream limit, the sender stops at the limit and resumes
  only after the receiver raises it with a MAX_STREAM_DATA or MAX_DATA frame.

## Acknowledgment

### RFC9000-ACK-1

**An ack-eliciting packet is acknowledged within the declared maximum delay.**

> "Every packet SHOULD be acknowledged at least once, and ack-eliciting packets MUST be
> acknowledged at least once within the maximum delay an endpoint communicated using the
> max_ack_delay transport parameter" — §13.2.1, `rfc9000.txt:4094-4096`

- Strength: should for every packet; must for an ack-eliciting one, against a declared
  bound. Class: wire.
- Check idea: a packet that carries a STREAM frame is ack-eliciting; an ACK frame that
  covers its packet number follows. The bound itself is a timing statement and belongs to
  a later level.

## Connection termination

### RFC9000-CLOSE-1

**A CONNECTION_CLOSE frame terminates the connection immediately.**

> "An endpoint sends a CONNECTION_CLOSE frame (Section 19.19) to terminate the connection
> immediately. A CONNECTION_CLOSE frame causes all streams to immediately become closed;
> open streams can be assumed to be implicitly reset." — §10.2, `rfc9000.txt:3170-3173`

- Strength: description of the mechanism. Class: wire.
- Check idea: the endpoint that closes sends a CONNECTION_CLOSE frame, and no application
  data follows it in either direction.


## Version negotiation

### RFC9000-VER-1

**A server that does not accept the client's version answers with a Version Negotiation
packet.**

> "If the version selected by the client is not acceptable to the server, the server
> responds with a Version Negotiation packet; see Section 17.2.1. This includes a list of
> versions that the server will accept." — §6.1, `rfc9000.txt:1644-1647`

- Strength: description, stated as what a server does. Class: wire.
- Check idea: put a version number that names no QUIC version in the client's first packet.
  The server answers with a packet whose version field is zero, which is what marks a
  Version Negotiation packet, and the connection does not open.

### RFC9000-VER-2

**No endpoint answers a Version Negotiation packet with another one.**

> "An endpoint MUST NOT send a Version Negotiation packet in response to receiving a
> Version Negotiation packet." — §6.1, `rfc9000.txt:1647-1648`

- Strength: must not. Class: wire (absence).
- Check idea: the rule guards against two endpoints that answer each other without end. A
  check needs a Version Negotiation packet delivered to an endpoint, and then silence.

## Address validation

### RFC9000-AMP-1

**Before the client's address is validated, a server sends at most three times the bytes it
received.**

> "Prior to validating the client address, servers MUST NOT send more than three times as
> many bytes as the number of bytes they have received. This limits the magnitude of any
> amplification attack that can be mounted using spoofed source addresses." — §8.1,
> `rfc9000.txt:2221-2224`

- Strength: must not. Class: wire, counted over a window.
- Check idea: count what the server sends and what it receives, from the first packet of a
  connection until the point where the address counts as validated, which §8.1 puts at the
  successful processing of a Handshake packet from the peer. The sent count never passes
  three times the received count.
- Note: this is the rule that keeps a QUIC server from becoming an amplifier for an
  attacker who writes someone else's address into a packet. It is also the reason the
  1200-octet floor of [RFC9000-SIZE-1](#rfc9000-size-1) exists: a client that sends little
  would leave a server too little budget to answer.

## Frame validation

### RFC9000-ERR-1

**A frame of unknown type is a connection error of type FRAME_ENCODING_ERROR.**

> "An endpoint MUST treat the receipt of a frame of unknown type as a connection error of
> type FRAME_ENCODING_ERROR." — §12.4, `rfc9000.txt:3980-3981`

- Strength: must. Class: error-signal plus end-to-end.
- Check idea: put a type number that Table 3 of §12.4 does not list into a frame of a
  packet in flight. The receiver closes the connection and says why, with a CONNECTION_CLOSE
  frame that names the error. It does not ignore the frame, and it does not stop.

## Out of scope in this catalog

RFC 9000 is a large document, and this catalog holds its normal path only. What it leaves
out, with the level that reaches it:

- **Level 3, and available in RFC 9000 itself.** The level 3 pass took four of these:
  version negotiation (RFC9000-VER-1 and VER-2), the anti-amplification limit
  (RFC9000-AMP-1), the frame encoding error (RFC9000-ERR-1), and the edge of ordered
  delivery, which needed no new entry because RFC9000-STR-2 already carries the sentence
  about buffering data received out of order. What is still out: the stateless reset;
  address validation with Retry and tokens; path validation and connection migration;
  stream reset with RESET_STREAM and STOP_SENDING; and the handling of a packet that cannot
  be decrypted or parsed.
- **Level 4, needing RFC 9002:** loss detection, congestion control, the probe timeout,
  the idle timeout, and the timing bound of RFC9000-ACK-1.
- **Level 5:** the cryptographic handshake and packet protection (RFC 9001), transport
  parameters in their full detail, the remaining frame types (NEW_CONNECTION_ID,
  RETIRE_CONNECTION_ID, NEW_TOKEN, PATH_CHALLENGE, PATH_RESPONSE, HANDSHAKE_DONE, PING,
  the BLOCKED family), 0-RTT, key updates, and the coalescing of several packets in one
  datagram.
