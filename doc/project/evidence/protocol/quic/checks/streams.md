# QUIC — English check procedures: streams

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

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

## Ordered delivery under reordering

Checks: **RFC9000-STR-2** (must), at its edge.

### Requirement

RFC 9000 §2.2: "Endpoints MUST be able to deliver stream data to an application as an
ordered byte stream. Delivering an ordered byte stream requires that an endpoint buffer any
data that is received out of order, up to the advertised flow control limit."

The second sentence is what this check is about. Level 2 established the first sentence on a
path that delivers in order, where buffering is never needed.

### Scenario constants

- The common mockup with a relay on the path.
- The client sends one block of data, large enough to need several packets.
- The relay holds the **second** packet that carries stream data for a short time and lets
  every other packet pass at once. The packets therefore arrive in a different order from
  the one in which they were sent, and the hold is short enough that nothing is treated as
  lost.

### Procedure

1. Build the mockup with a relay between the client and the server.
2. Let the client send its block.
3. Watch the order in which the packets reach the server, and what the server's application
   receives.

### Expected observations

1. The server receives a stream frame whose offset is **after** the offset of the frame the
   relay held. This confirms that the arrival order differs from the sending order.
2. The server receives the held frame afterwards.
3. The server's application receives the whole block, and its bytes are the ones the client
   wrote, in the order the client wrote them.

### Notes

- The check needs no crafted content. Reordering is what a network does, and a relay that
  holds one packet is the smallest way to produce it.
- Observation 3 is the requirement. Observations 1 and 2 establish that the requirement was
  put under load: without them a pass would say only that a path in order delivers in order.

