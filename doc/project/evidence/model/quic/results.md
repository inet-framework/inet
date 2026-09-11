# QUIC checks — run results and model analysis (pass 2, level 3)

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9000/catalog.md), [checks.md](../../protocol/quic/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the QUIC
workflow that may reference code.

- Date: 2026-09-10 15:18 +0200
- INET: branch `master`, commit `0868c36c88`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w quic
  ```
- Earlier passes ran on other trees. The pass log of [`coverage.md`](coverage.md#pass-log)
  names them. Every verdict below is the verdict of the run above, and it repeats the
  verdict of the earlier pass.

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9000ConnectionEstablishment.test | RFC9000-PKT-1, PKT-2, SIZE-1 | PASS |
| Rfc9000StreamDataTransfer.test | RFC9000-STR-2, STR-3, PKT-3 | PASS |
| Rfc9000StreamIdentifiers.test | RFC9000-STR-1 | PASS, **and the pass establishes little; see below** |
| Rfc9000FlowControl.test | RFC9000-FC-1 | PASS |
| Rfc9000Acknowledgment.test | RFC9000-ACK-1 | PASS |
| Rfc9000ConnectionClose.test | RFC9000-CLOSE-1 | PASS |

Summary of pass 1: 6 PASS in 0.4 s. No `%# expected-result: FAIL` marker was necessary, and
no test failed. Much of this document is about what those passes do and do not establish,
because for QUIC that gap is wider than for the three protocols already in this tree.

Pass 2, level 3, with the two level 2 blockers closed first:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9000ReorderedDelivery.test | RFC9000-STR-2, at its edge | PASS |
| Rfc9000AntiAmplification.test | RFC9000-AMP-1 | PASS |
| Rfc9000ServerInitialSize.test | RFC9000-SIZE-1, the server half | **FAIL (unexpected)**, gap 1, a defect |
| Rfc9000VersionNegotiation.test | RFC9000-VER-1 | FAIL (expected), gap 2, unimplemented |
| Rfc9000UnknownFrameType.test | RFC9000-ERR-1 | **FAIL (unexpected)**, gap 3, a defect |

Summary after pass 2: 11 tests, 8 PASS, **1 FAIL (expected), 2 FAIL (unexpected)**, so the suite
reports FAIL. The tallies of the other suites are not repeated here: a document that quotes another
suite's numbers goes stale on that suite's next run.

## Which failures are declared, and which are not

Reviewed against
[the third principle of the guide](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test):
a failure is declared expected only where the model does **not** claim the behavior, and a claim is
code.

| Test | Class | The claim, in the model |
| --- | --- | --- |
| `Rfc9000ServerInitialSize.test` | **defect** | `PacketBuilder::buildClientInitialPacket` pads to exactly the size the requirement names, `createPaddingFrame(1200 - packet->getSize())` (PacketBuilder.cc:476). The padding mechanism is implemented and applied to one of the two sides RFC 9000 names. |
| `Rfc9000UnknownFrameType.test` | **defect** | `ConnectionState.cc` dispatches a frame on its type and has a `default:` branch for one it does not know; that branch throws. |
| `Rfc9000VersionNegotiation.test` | unimplemented | Nothing reads the version of a received packet and nothing builds a Version Negotiation packet. `VersionNegotiationPacketHeader` is declared in `packet/PacketHeader.msg:45`, and a search of the hand-written sources finds no use of it: a declared message type with no builder and no sender is a modelled packet format, not an implemented behavior. |

Two of the passes are worth as much as the failures. The reordering check puts the buffering
half of RFC9000-STR-2 under load for the first time, and the model keeps the data that
arrives early. The anti-amplification check establishes a bound over a window, and the model
stays far inside it.

## Two probes, before any test was written

The level 2 ledger left two blockers, and each one was settled by a probe rather than by a
guess. Both probes were thrown away afterwards; what they found is here.

### Probe 1 — the model has one stream identifier

The level 2 notes ended with "Finding out why the second stream does not appear is the first
task of the next pass." The answer is that the second stream does appear, and it is stream 0.

With two generators, whose identifiers are 0 and 1, every stream frame the client sends
carries `streamId = 0`, and the second generator's bytes continue the first one's offsets:
0, 1454, 2907, then 4000, 5453, 6906. With a single generator asked for identifier 1, the
client still sends on stream 0.

So the model has one stream identifier and no path that assigns another. A check on the wire
that asks anything of the identifier cannot fail, whatever it asks. RFC9000-STR-1 therefore
becomes `no check` in the ledger, with this reason, rather than a check that passes because
the only value it can see happens to be lawful. That is the honest form of the level 2
blocker, and it closes it: the feature keeps its `partial` value for a stated cause and not
for a missing test.

### Probe 2 — the server's Initial datagram is 27 octets

The level 2 notes suspected an undeclared gap in the 1200-octet floor. The probe found it.
The server's four handshake datagrams are 27, 38, 24 and 22 octets, and the first of them
carries an Initial packet with a CRYPTO frame, which is exactly the case §14.1 names. This
became `Rfc9000ServerInitialSize.test` and model gap 1.

## Model gaps

### Gap 1 — the server does not expand its Initial datagram

**Statement.** [RFC9000-SIZE-1](../../standard/rfc9000/catalog.md#rfc9000-size-1), must, the
server half: "Similarly, a server MUST expand the payload of all UDP datagrams carrying
ack-eliciting Initial packets to at least the smallest allowed maximum datagram size of 1200
bytes", §14.1, `rfc9000.txt:4648-4650`.

**What the model does.** It sends 27 octets. The packet carries a CRYPTO frame, so it is
ack-eliciting and the requirement applies to it.

**Why it is a gap.** The floor is not decoration. It is what gives a server room to answer
under the anti-amplification bound of §8.1, and it is what makes a path prove it can carry a
1200-octet datagram before a connection depends on it.

**Half implemented, and the missing half undeclared.** The client half passes: the client's
Initial datagram is padded to exactly 1200, and `Rfc9000AntiAmplification.test` depends on
it. `Quic.ned` declares twelve deviations and this is not among them, which is why the level
2 pass called it undeclared and left it for a check.

### Gap 2 — a version nobody speaks buys a full connection

**Statement.** [RFC9000-VER-1](../../standard/rfc9000/catalog.md#rfc9000-ver-1): "If the
version selected by the client is not acceptable to the server, the server responds with a
Version Negotiation packet", §6.1, `rfc9000.txt:1644-1646`.

**What the model does.** Nothing about the version at all. The relay wrote 0x0A0A0A0A into
the version field of the client's Initial packets. The run shows 22 packets carrying that
version, a HandshakeDone frame, and 75 stream frames: the handshake completed and the data
flowed.

**Why it is a gap.** The version is the first thing a QUIC endpoint reads, before anything it
must decrypt, and answering it is the one decision a server can take with no state. A server
that ignores the field cannot tell a peer of a future version that it speaks something else,
and the mechanism that lets QUIC change versions at all is absent.

### Gap 3 — an unknown frame type stops the run

**Statement.** [RFC9000-ERR-1](../../standard/rfc9000/catalog.md#rfc9000-err-1), must: "An
endpoint MUST treat the receipt of a frame of unknown type as a connection error of type
FRAME_ENCODING_ERROR", §12.4, `rfc9000.txt:3980-3981`.

**What the model does.** It stops the simulation:

```
Error: Unknown Frame Header Type -- in module (inet::quic::Quic) QuicTapNet.server.quic
(id=67), at t=0.10024426s, event #150
```

`ConnectionState.cc` dispatches an incoming frame on `getFrameType()`, and the default branch
of that switch throws.

**Why it is a gap.** A connection error is an answer to the peer and the end of one
connection. A stop is neither: the endpoint does not survive, and the peer learns nothing.
Frame types come from a registry that grows, so an endpoint meets unknown ones in the
ordinary course of the internet.

**A pattern across this tree.** This is the third protocol whose model stops on a crafted but
lawful input: IPv4 on an unknown ICMP type, TCP on a Source Quench, and QUIC here. The causes
are separate switches in three modules, and the shape is the same one.

## The tooling finding: the framework cannot see QUIC

This is the first protocol in this tree whose packets the test framework cannot dissect,
and it changed how every check had to be written.

**What pass 2 adds to it.** The absence of a dissector does not stop a relay. A relay selects
a datagram by a UDP field, which the filter can read, or by its size, and the mutator then
looks at the chunks and decides for itself. Three checks of pass 2 change a QUIC field this
way. Two costs came with it, and both are worth knowing before the next protocol without a
dissector:

- **Size is a poor selector near the 1200-octet floor.** The client's Initial datagram is
  padded to exactly 1200, so a threshold of 1000 selects the handshake rather than the data.
  The reordering check uses 1300.
- **A mutator that sees every datagram must decline politely.** `mutateFirstQuicChunk`
  returns false when the packet carries no chunk of the type it was asked for, so a relay
  with no content filter can be pointed at a whole direction and still change one kind of
  packet.

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
[stream identifier assignment](../../protocol/quic/checks/streams.md#stream-identifier-assignment)
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

Four items of the pass 1 list are done. The second is answered rather than done: the second
stream does reach the wire, and it is stream 0.

- **Done:** the two probes above; ordered delivery under reordering; the anti-amplification
  limit; version negotiation; connection errors through the unknown frame type; the
  server-side 1200-octet expansion, which is now gap 1.
- **A protocol dissector for QUIC**, still. It is not a gate any more — the level 3 checks
  read and now also **change** chunk types, and that works — but every QUIC check in this
  suite pays for its absence in hand-written code, and a dissector would turn all of them
  into ordinary field filters.
- **Correct the frame type switch** so that a type it does not know ends the connection
  instead of the run. The same shape waits in the ICMP module for IPv4 and TCP.
- **A check for RFC9000-VER-2**, that no endpoint answers a Version Negotiation packet with
  another. The relay can make such a packet now that it can change a version field; this
  pass did not.
- **Level 3, what remains:** the stateless reset, address validation with Retry and tokens,
  path validation and migration, stream reset, and the handling of a packet that cannot be
  decrypted.
- **Level 4:** RFC 9002, and with it loss detection, congestion control, the idle timeout
  and the acknowledgment delay bound.
