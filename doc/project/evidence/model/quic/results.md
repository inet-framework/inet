# QUIC checks — run results and model analysis (pass 1, level 2)

> **Kind:** report · **Status:** snapshot 2026-09-08 · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9000/catalog.md), [checks.md](../../protocol/quic/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the QUIC
workflow that may reference code.

- Date: 2026-09-08. Tree: `inet-rfc-tests-quic`, branch `topic/rfc-tests-quic` on top of
  `topic/rfc-tests-ipv4`, source identical to `master`.
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w quic
  ```

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9000ConnectionEstablishment.test | RFC9000-PKT-1, PKT-2, SIZE-1 | PASS |
| Rfc9000StreamDataTransfer.test | RFC9000-STR-2, STR-3, PKT-3 | PASS |
| Rfc9000StreamIdentifiers.test | RFC9000-STR-1 | PASS, **and the pass establishes little; see below** |
| Rfc9000FlowControl.test | RFC9000-FC-1 | PASS |
| Rfc9000Acknowledgment.test | RFC9000-ACK-1 | PASS |
| Rfc9000ConnectionClose.test | RFC9000-CLOSE-1 | PASS |

Summary: 6 PASS in 0.4 s. No `%# expected-result: FAIL` marker was necessary, and no test
failed. The rest of this document is about what those passes do and do not establish,
because for QUIC that gap is wider than for the three protocols already in this tree.

## The tooling finding: the framework cannot see QUIC

This is the first protocol in this tree whose packets the test framework cannot dissect,
and it changed how every check had to be written.

1. **No protocol dissector is registered for QUIC.** A repository-wide search for
   `Register_Protocol_Dissector` finds none for `Protocol::quic`. `PacketDissector`
   therefore falls back to the default dissector, which reports the whole undissected
   remainder as one chunk. The class map that the framework's `evalPacketField` builds gets
   a single entry, `SequenceChunk`, and never the QUIC classes. Every attempt at
   `InitialPacketHeader.version`, `LongPacketHeader.version`,
   `ShortPacketHeader.packetNumber` and `StreamFrameHeader.streamId` failed with
   `protocol '...' not present in packet`. This is architectural, not a corner case:
   without a dissector, no `quic.<field>` filter can ever work.
2. **The Quic module emits none of the signals the framework recognises.** It emits
   `packetSent` and `packetReceived` of type `cPacket`, and a large family of statistics;
   the framework turns only `packetSentToLower`, `packetReceivedFromLower`,
   `packetSentToUpper` and their relatives into events. So `on("client.quic")` has nothing
   to hook.
3. **What works.** QUIC rides on UDP, and the UDP module does emit the recognised signals.
   Every check therefore observes UDP datagrams at `client.udp` and `server.udp`, and
   reads QUIC fields by walking the packet's own chunk list, which carries the concrete
   QUIC chunk types in wire order. The suite's shared helper
   [`QuicChunks.h`](../../../../../tests/protocol/quic/QuicChunks.h) does that walk.
4. **A trap in that helper, found in review.** Its first form returned only the first chunk
   of a kind, and a QUIC packet carries several frames. A check that must hold for *every*
   frame of a kind therefore silently examined one. The helper now offers both forms, and
   the difference is documented in it.
5. **Two statistics that cannot be observed.** The application's `bytesReceived` signal
   fires per chunk, never as a total. The module's `totalRcvAppData` statistic is a running
   total, but subscribing to it aborts the run: `Unsupported signal data type uintval_t`,
   because the tester implements only the signed overload.

The first task of a QUIC level 3 is therefore not a test at all: it is a protocol
dissector, or a decision that QUIC checks will always be written against chunk types.

## What the passes establish, and what they do not

### The stream identifier check is nearly vacuous

`Rfc9000StreamIdentifiers.test` asserts that the client's stream identifier is even and
bidirectional. It passes: the identifier is 0. But it passes because the reference
application asks for 0, not because the model maintains the encoding. Reading the model
settles the question:

- `QuicTrafficgen::getStreamId` returns the generator's index unchanged
  ([QuicTrafficgen.cc:234-237](../../../../../src/inet/applications/quicapp/QuicTrafficgen.cc#L234-L237)).
- `Connection::findOrCreateStream` creates a stream for whatever identifier it is handed,
  with no validation
  ([Connection.cc:169-178](../../../../../src/inet/transportlayer/quic/Connection.cc#L169-L178)).
- The model says so itself, in the "Missing bits" section of
  [Quic.ned:163-166](../../../../../src/inet/transportlayer/quic/Quic.ned#L163-L166):
  "Stream types: QUIC distinguish streams depending on who opened the stream (client or
  server) and whether the stream is unidirectional or bidirectional. This module does not
  distinguish that. All streams can be used bidirectional."

So an application that asks for stream 1 gets stream 1 on the wire, and 1 is an odd
identifier, which RFC 9000 §2.1 reserves for the server. The encoding is not maintained.

**A decisive check was attempted and withdrawn.** The check
[stream identifier assignment](../../protocol/quic/checks.md#stream-identifier-assignment)
states it: with two application streams, every identifier the client sends on must be a
multiple of four. A test was written for it and it passed — but the application's second
stream never reached the observation point, so the test passed without exercising the case
it exists for. Rather than keep a green test whose pass could not be explained, the test
was removed and the check left without one. The ledger records it as a coverage gap, not as
a satisfied requirement. Finding out why the second stream does not appear is the first
task of the next pass.

### The 1200-octet floor is half implemented

`Rfc9000ConnectionEstablishment.test` observes the client's first datagram at 1200 octets
and passes. The client-side padding is explicit:

```cpp
packet->addFrame(createPaddingFrame(1200 - packet->getSize()));
```

([PacketBuilder.cc:476](../../../../../src/inet/transportlayer/quic/PacketBuilder.cc#L476),
in `buildClientInitialPacket`).

`buildServerInitialPacket`
([PacketBuilder.cc:481-497](../../../../../src/inet/transportlayer/quic/PacketBuilder.cc#L481-L497))
has no padding call at all. RFC 9000 §14.1 requires the server to expand every datagram
carrying an ack-eliciting Initial packet to 1200 octets as well. That half is not
implemented, and it is **not** among the deviations the model declares: the "Missing bits"
list names amplification-attack mitigation, which is a related but different requirement.
No check covers the server half, so this is a code reading and not a verdict; it is the
strongest candidate for a `defect` at the next pass.

### Ordered delivery is established only where nothing reorders

`Rfc9000StreamDataTransfer.test` establishes that the whole 4000-octet block arrives. The
interesting half of RFC9000-STR-2 — that an endpoint buffers data that arrives out of order
and still delivers an ordered stream — needs a path that reorders, which is level 3.

### Flow control is established for one stream

The check binds the stream limit and shows the sender stopping at it. QUIC's second limit,
the connection limit across several streams, is untouched. The model has one parameter for
all stream types where the RFC has three, and says so
([Quic.ned:167-171](../../../../../src/inet/transportlayer/quic/Quic.ned#L167-L171)).

## Deviations between the English observations and the test steps

1. **Every check observes UDP datagrams, not QUIC packets**, and reads fields through typed
   predicates on chunk types. The reason is the tooling finding above.
2. **Stream data transfer, observation 3.** "The application receives 4000 octets" is not
   observable: the application's signal fires per chunk and the module's total aborts the
   run. The step observes instead the arrival at the server of the STREAM frame whose
   offset plus length reaches 4000, which is the transport's own accounting of the same
   fact.
3. **Connection close, observation 2.** The absence of STREAM frames is checked on the
   client's side only. The server application never originates a STREAM frame at all, so
   the reverse direction would be trivially true for the whole run and would establish
   nothing.
4. **Flow control, observation 2.** The framework's steps are sequential and single-pass,
   and a scalar signal cannot be accumulated in a predicate. The running total is kept in
   state shared between the steps' predicates instead, so that a violation makes the later
   step unmatchable and the check fails by deadline. The mechanism was verified with two
   negative controls: forcing the violation flag made the check fail as intended, and a
   configuration change that creates no real violation left it passing.

## Model analysis — where INET implements the checked behavior

- **Packet headers (PKT-1, PKT-2):** the header classes are in
  [PacketHeader.msg](../../../../../src/inet/transportlayer/quic/packet/PacketHeader.msg);
  `LongPacketHeader` carries `version = 1`, both connection identifiers and their lengths,
  and `ShortPacketHeader` carries the destination identifier and the packet number. The
  builder constructs Initial, Handshake, 0-RTT and 1-RTT headers
  ([PacketBuilder.cc:48-92](../../../../../src/inet/transportlayer/quic/PacketBuilder.cc#L48-L92)).
  `RetryPacketHeader` and `VersionNegotiationPacketHeader` exist in the schema and are
  never built, which matches the model's declared deviations.
- **Packet numbers (PKT-3):** one counter per packet number space, assigned by
  post-increment
  ([PacketBuilder.cc:53, 67, 78, 92](../../../../../src/inet/transportlayer/quic/PacketBuilder.cc#L53));
  monotonic and never reused by construction.
- **Initial datagram size (SIZE-1):**
  [PacketBuilder.cc:476](../../../../../src/inet/transportlayer/quic/PacketBuilder.cc#L476),
  client side only.
- **Streams (STR-2, STR-3):** `StreamFrameHeader`
  ([FrameHeader.msg:38-45](../../../../../src/inet/transportlayer/quic/packet/FrameHeader.msg#L38-L45)),
  which carries the FIN bit as a separate field and says so in its own comment, because the
  RFC packs it into the frame type. Delivery is through
  `Connection::newStreamData` and the stream receive queue.
- **Flow control (FC-1):** enforced at both levels in
  [Stream.cc:243-259](../../../../../src/inet/transportlayer/quic/stream/Stream.cc#L243-L259),
  which returns the smaller of the connection credit and the stream credit and generates a
  blocked frame when either is exhausted; the result caps the STREAM frame region at
  [Stream.cc:70-77](../../../../../src/inet/transportlayer/quic/stream/Stream.cc#L70-L77).
- **Acknowledgment (ACK-1):** `AckFrameHeader`
  ([FrameHeader.msg](../../../../../src/inet/transportlayer/quic/packet/FrameHeader.msg)),
  whose ECN fields are commented out, matching the declared absence of ECN.
- **Connection close (CLOSE-1):** `ConnectionCloseFrameHeader`, sent synchronously from
  `Connection::close`.

## Model observations the checks did not claim

1. **No encryption, by design and by declaration.** The model states it
   ([Quic.ned:142-144](../../../../../src/inet/transportlayer/quic/Quic.ned#L142-L144)).
   This is what makes the checks possible at all: on a protected connection, every check
   above that reads a frame field would need the keys.
2. **The connection identifier is always 0**
   ([Quic.ned:175-178](../../../../../src/inet/transportlayer/quic/Quic.ned#L175-L178)), so
   the identifier half of RFC9000-PKT-1 is present in form and degenerate in content.
3. **Eight frame types of RFC 9000 are absent** from the model's frame enumeration:
   RESET_STREAM, STOP_SENDING, MAX_STREAMS, STREAMS_BLOCKED, NEW_CONNECTION_ID,
   RETIRE_CONNECTION_ID, PATH_CHALLENGE and PATH_RESPONSE. Each corresponds to a declared
   deviation.
4. **A showcase describes QUIC's real-world features without the model's caveats.**
   `showcases/quic/linksharing/doc/index.rst` describes TLS 1.3 encryption and connection
   migration as QUIC properties, and the page does not say that this model implements
   neither. A reader of the showcase alone could reasonably conclude otherwise. The module
   documentation is exemplary; the showcase does not inherit it.

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger,
[`coverage.md`](coverage.md#feature-support).

## Sharpening candidates for the next pass

- **A protocol dissector for QUIC**, or an agreed convention that QUIC checks read chunk
  types. This is the gate on everything else.
- **Find out why a second application stream does not reach the wire**, and then run the
  withdrawn stream-identifier assignment check. It is the one check that would settle
  RFC9000-STR-1 rather than coincide with it.
- **The server-side 1200-octet expansion**, which the code does not do and the model does
  not declare.
- **Level 3:** ordered delivery under reordering; connection errors; the anti-amplification
  limit; version negotiation. RFC 9000 already holds the text for all of them.
- **Level 4:** RFC 9002, and with it loss detection, congestion control, the idle timeout
  and the acknowledgment delay bound.
