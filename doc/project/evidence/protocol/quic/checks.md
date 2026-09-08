# QUIC — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9000/catalog.md)

Step 5 artifact of the standards test workflow. This file holds the English check
procedures for QUIC, one section per check. The procedures come from the specification only.
They name no simulation model and no code.

## Common mockup

Two hosts and a path between them. Host A opens a QUIC connection to host B, sends a block
of application data on one stream, and closes.

```
   host A  ------------  host B
            path P
   (client)              (server)
```

Scenario constants, shared unless a check says otherwise:

| Constant | Value | Why |
| --- | --- | --- |
| Path | one link, no loss, no reordering | level 2 observes the normal path; loss belongs to level 3 and level 4 |
| Data block | 4000 octets on one stream | several packets' worth, so packet numbers and ordering are visible |
| Observation limit | 5 s | generous; nothing in the scenario needs more |

Two properties of QUIC shape every check below, and neither has a counterpart in the
protocols already in this tree:

- **QUIC rides on UDP.** Every QUIC packet is the payload of a UDP datagram. An observation
  of "a packet on the path" is therefore an observation of a UDP datagram, and its size is
  the size the UDP datagram carries.
- **QUIC packets are normally protected.** A real endpoint encrypts everything after the
  header. A check that reads a field of a frame is possible only where the observer has the
  keys, or where packet protection is absent. Each check below states which fields it
  needs; a reader can then tell which checks survive on a protected connection.

## Connection establishment

- **Checks** — RFC9000-PKT-1, RFC9000-PKT-2 (description), RFC9000-SIZE-1 (must).
- **Requirement** — the packets that set a connection up carry a long header with the
  32-bit version field and both connection identifiers. Every UDP datagram that carries an
  Initial packet is at least 1200 octets, expanded with padding if its content is smaller.
  After the handshake, the packets that carry application data use the short header.

### Procedure

1. Build the mockup.
2. Let host A open a connection to host B and send the data block.
3. Observe the UDP datagrams on the path in both directions.

### Expected observations

1. The first UDP datagram from host A to host B carries a QUIC packet with a long header,
   whose version field names QUIC version 1 (RFC9000-PKT-1). *(This confirms the stimulus:
   a connection attempt started.)*
2. That same datagram is at least 1200 octets long (RFC9000-SIZE-1). The Initial packet's
   own content is far smaller, so the size can only come from expansion.
3. Host B answers with a datagram carrying a long header packet: the handshake proceeds in
   both directions.
4. Later, host A sends a datagram carrying a QUIC packet with a short header
   (RFC9000-PKT-2): the connection reached the state in which application data travels.

### Notes

- Observation 2 is the boundary value of QUIC and the reason a QUIC handshake is never
  small. It is also the requirement a simulation can most easily omit without visible harm,
  because a simulated path has no path-MTU problem to discover.
- The RFC requires the expansion of a server's ack-eliciting Initial datagrams as well.
  That half is stated separately in the notes of the results, because a server's first
  datagram is not always ack-eliciting and the check does not force the case.

## Stream data transfer

- **Checks** — RFC9000-STR-2 (must), RFC9000-STR-3 (description), RFC9000-PKT-3 (must).
- **Requirement** — STREAM frames create a stream and carry its data. The receiving
  endpoint delivers the data to its application as an ordered byte stream: the octets the
  application receives are the octets the sender wrote, in the order it wrote them. Within
  one packet number space, an endpoint never reuses a packet number.

### Procedure

1. Build the mockup.
2. Let host A open a connection and send the 4000-octet block on one stream.
3. Observe the datagrams on the path and what host B's application receives.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a STREAM frame (RFC9000-STR-3).
   *(Confirms the stimulus: the application data entered a stream.)*
2. Host A sends a second datagram whose QUIC packet carries a packet number different from
   the first one's, in the same packet number space (RFC9000-PKT-3).
3. Host B's application receives 4000 octets in total (RFC9000-STR-2).

### Notes

- Observation 3 establishes ordered delivery only in the absence of reordering, which the
  mockup guarantees. Ordered delivery of data that arrives out of order is the interesting
  half of RFC9000-STR-2, and it needs a path that reorders: level 3.
- A receiver that counts octets rather than checking their content would satisfy
  observation 3 while delivering the wrong bytes. A check of the content is worth adding
  when a receiving application that verifies the byte order is available.

## Stream identifiers

- **Checks** — RFC9000-STR-1 (description of the encoding, normative).
- **Requirement** — the least significant bit of a stream identifier says who opened the
  stream: 0 for a client-initiated stream, 1 for a server-initiated one. The second bit
  says whether the stream is bidirectional (0) or unidirectional (1). The first
  bidirectional stream a client opens therefore has identifier 0.

### Procedure

1. Build the mockup.
2. Let host A, the client, open one bidirectional stream and send data on it.
3. Observe the stream identifier that the STREAM frames carry.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a STREAM frame. *(Confirms the
   stimulus.)*
2. The stream identifier in that frame is even: its low bit is 0, because the client
   opened the stream (RFC9000-STR-1).
3. The stream identifier's second bit is 0: the stream is bidirectional (RFC9000-STR-1).

### Notes

- This check reads a field of a frame, so on a protected connection it needs the keys.
- The encoding is not decoration. A peer uses these bits to decide whether it may send on
  the stream, and a wrong identifier makes a stream unusable in one direction.

## Stream identifier assignment

- **Checks** — RFC9000-STR-1 (description of the encoding, normative), from the decisive
  angle.
- **Requirement** — the encoding of a stream identifier is a property the endpoint
  maintains, not a number an application chooses. Every stream a client initiates carries
  an identifier whose low bit is 0, and every bidirectional stream carries an identifier
  whose second bit is 0. A client-initiated bidirectional stream therefore has an
  identifier that is a multiple of four: 0, 4, 8, and so on. An endpoint that opens a
  second such stream uses 4, never 1, because 1 belongs to the peer.

### Scenario constants for this check

- The application on host A opens **two** streams and sends data on both.

### Procedure

1. Build the mockup.
2. Let the application on host A open two streams and send data on each.
3. Observe every STREAM frame that host A sends, and read its stream identifier.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a STREAM frame. *(Confirms the
   stimulus.)*
2. Every stream identifier that host A sends on is a multiple of four: its low bit is 0,
   because host A is the client that initiated the stream, and its second bit is 0, because
   the stream is bidirectional (RFC9000-STR-1).

### Notes

- This check exists because the [stream identifiers](#stream-identifiers) check above
  cannot distinguish an endpoint that maintains the encoding from one that passes an
  application's chosen number through unchanged. With one stream both look the same; with
  two they do not.
- The check is level 2. It needs no fault and no crafted packet: an application opening a
  second stream is an ordinary action.
- The identifier is the endpoint's to assign. An interface that lets an application choose
  the raw number is itself a departure from the document's model, in which the application
  opens a stream and the transport names it.
- This check has no test yet. A first attempt could not be made to run: the sending
  application asked for a second stream, and no frame for that stream reached the
  observation point, so the check would have passed without ever exercising the case it
  exists to exercise. A check that passes for a reason the author cannot state is worth
  nothing, so the attempt was withdrawn rather than kept. The results record what is known
  about the model's behavior from reading it.

## Flow control

- **Checks** — RFC9000-FC-1 (must).
- **Requirement** — a receiver advertises a limit on the data it will accept, per stream
  and for the connection as a whole, and a sender sends no data beyond either limit. A
  sender that has reached a limit waits for the receiver to raise it.

### Scenario constants for this check

| Constant | Value | Why |
| --- | --- | --- |
| Initial stream limit at host B | 2000 octets | smaller than the data block, so the limit binds |
| Data block | 4000 octets | twice the limit, so the sender must stop and wait |

### Procedure

1. Build the mockup with host B's initial stream limit at 2000 octets.
2. Let host A open a connection and send the 4000-octet block on one stream.
3. Observe the total stream data that host A sends before host B raises the limit.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a STREAM frame. *(Confirms the
   stimulus.)*
2. The total stream data host A sends, before any frame from host B raises the limit,
   does not exceed 2000 octets (RFC9000-FC-1).
3. Host B sends a frame that raises the limit, and host A then sends the remainder, so that
   host B's application receives all 4000 octets.

### Notes

- Observation 2 is the whole of the requirement: the limit binds a sender, it does not
  merely inform it.
- QUIC has two limits at once, per stream and per connection. This check exercises the
  stream limit. A connection limit binding across several streams is a second check, and it
  needs a scenario with more than one stream.

## Acknowledgment

- **Checks** — RFC9000-ACK-1 (must, for an ack-eliciting packet).
- **Requirement** — a packet that carries a frame other than ACK, PADDING or
  CONNECTION_CLOSE elicits an acknowledgment. The peer sends an ACK frame that covers that
  packet's number, within the maximum delay it declared.

### Procedure

1. Build the mockup.
2. Let host A open a connection and send the data block.
3. Observe the ACK frames that return from host B.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a STREAM frame, which is
   ack-eliciting. Record its packet number. *(Confirms the stimulus.)*
2. Host B sends a datagram whose QUIC packet carries an ACK frame whose largest
   acknowledged number is at least the recorded one (RFC9000-ACK-1).

### Notes

- The timing half of the requirement, the declared maximum delay, is a statement about a
  distribution. It belongs to level 4 with RFC 9002.
- An ACK frame acknowledges packet numbers, not octets. That is the difference from TCP's
  cumulative acknowledgment, and it is why observation 2 names a number and not a stream
  offset.

## Connection close

- **Checks** — RFC9000-CLOSE-1 (description of the mechanism).
- **Requirement** — an endpoint terminates the connection immediately by sending a
  CONNECTION_CLOSE frame. The frame closes every stream at once; no application data
  follows it.

### Procedure

1. Build the mockup.
2. Let host A open a connection, send the data block, and close the connection.
3. Observe the datagrams on the path after the close.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a CONNECTION_CLOSE frame. *(Confirms
   the stimulus: the close really started.)*
2. From then on, no datagram carrying a STREAM frame travels in either direction: the
   streams are closed.

### Notes

- RFC 9000 gives an endpoint a second way to end a connection, the idle timeout. It is a
  timer and belongs to level 4.
- The closing and draining states that follow the frame, and the retransmission of the
  frame in answer to a late packet, are level 3.
